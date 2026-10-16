// DataSmoother.cpp : 超量程数据平滑修正
//

#include "stdafx.h"
#include "DataSmoother.h"

#include <algorithm>
#include <cmath>

// 中值滤波半窗口（窗口 = 2 * 2 + 1 = 5 点）
static const size_t kHalfWin = 2;
// 压缩后峰值占过渡带的比例，留出余量使峰值不贴着量程线
static const double kPeakRatio = 0.85;
// 一个越界段持续过长时，缓存超过该点数则强制输出，防止内存无限增长
static const size_t kMaxPending = 3000000;

CDataSmoother::CDataSmoother()
{
	Init(0.0, 100.0, 5.0);
}

void CDataSmoother::Init(double low, double high, double bandPercent)
{
	if (bandPercent < 1.0) bandPercent = 1.0;
	if (bandPercent > 30.0) bandPercent = 30.0;

	m_low = low;
	m_high = high;
	m_band = (high - low) * bandPercent / 100.0;
	m_kneeLow = low + m_band;
	m_kneeHigh = high - m_band;

	m_buf.clear();
	m_hist = 0;
	m_stats.total = 0;
	m_stats.outOfRange = 0;
	m_stats.modified = 0;
	m_stats.segments = 0;
}

int CDataSmoother::Classify(double v) const
{
	if (v > m_kneeHigh) return 1;
	if (v < m_kneeLow) return -1;
	return 0;
}

void CDataSmoother::Push(const std::vector<SmoothPoint>& pts, std::vector<SmoothResult>& out)
{
	m_buf.insert(m_buf.end(), pts.begin(), pts.end());
	Process(false, out);
}

void CDataSmoother::Flush(std::vector<SmoothResult>& out)
{
	Process(true, out);
	m_buf.clear();
	m_hist = 0;
}

void CDataSmoother::Process(bool bFinal, std::vector<SmoothResult>& out)
{
	size_t n = m_buf.size();
	if (n <= m_hist)
	{
		return;
	}

	// 只输出到最后一个正常点为止，之后的越界段可能还没结束，留到下一批
	size_t cut = 0;
	if (bFinal)
	{
		cut = n;
	}
	else
	{
		for (size_t i = n; i-- > m_hist;)
		{
			if (Classify(m_buf[i].v) == 0)
			{
				cut = i + 1;
				break;
			}
		}
		if (cut == 0)
		{
			if (n - m_hist <= kMaxPending)
			{
				return;
			}
			cut = n;
		}
	}

	size_t i = m_hist;
	while (i < cut)
	{
		int dir = Classify(m_buf[i].v);
		if (dir == 0)
		{
			Emit(i, m_buf[i].v, out);
			i++;
			continue;
		}
		size_t j = i;
		while (j < cut && Classify(m_buf[j].v) == dir)
		{
			j++;
		}
		FixSegment(i, j, dir, out);
		i = j;
	}

	// 保留少量已输出的点，供下一批数据做中值滤波
	size_t keep = (std::min)(kHalfWin, cut);
	m_buf.erase(m_buf.begin(), m_buf.begin() + (cut - keep));
	m_hist = keep;
}

double CDataSmoother::Median(size_t i) const
{
	size_t a = i >= kHalfWin ? i - kHalfWin : 0;
	size_t b = (std::min)(m_buf.size(), i + kHalfWin + 1);
	double w[2 * kHalfWin + 1];
	size_t cnt = 0;
	for (size_t k = a; k < b; k++)
	{
		w[cnt++] = m_buf[k].v;
	}
	std::nth_element(w, w + cnt / 2, w + cnt);
	return w[cnt / 2];
}

void CDataSmoother::Emit(size_t i, double val, std::vector<SmoothResult>& out)
{
	SmoothResult r;
	r.t = m_buf[i].t;
	r.orig = m_buf[i].v;
	r.val = val;
	r.modified = std::fabs(val - r.orig) > 1e-9;
	out.push_back(r);

	m_stats.total++;
	if (r.orig > m_high || r.orig < m_low) m_stats.outOfRange++;
	if (r.modified) m_stats.modified++;
}

// 修正越界段 [a, b)，dir = 1 表示超上限方向，-1 表示超下限方向
void CDataSmoother::FixSegment(size_t a, size_t b, int dir, std::vector<SmoothResult>& out)
{
	bool bBeyond = false;
	for (size_t i = a; i < b; i++)
	{
		if (m_buf[i].v > m_high || m_buf[i].v < m_low)
		{
			bBeyond = true;
			break;
		}
	}
	if (!bBeyond)
	{
		// 只是进入了过渡带，没有超出量程，保持原值
		for (size_t i = a; i < b; i++)
		{
			Emit(i, m_buf[i].v, out);
		}
		return;
	}

	m_stats.segments++;

	const double knee = dir > 0 ? m_kneeHigh : m_kneeLow;
	std::vector<double> med(b - a);
	double peak = 0.0;	// 段内相对拐点的最大偏离量
	for (size_t i = a; i < b; i++)
	{
		med[i - a] = Median(i);
		double e = (med[i - a] - knee) * dir;
		if (e > peak) peak = e;
	}

	const double room = kPeakRatio * m_band;
	const double k = peak > room ? room / peak : 1.0;

	for (size_t i = a; i < b; i++)
	{
		double o = m_buf[i].v;
		double m = med[i - a];
		double val;
		if (k >= 1.0)
		{
			// 滤掉尖峰后已在量程内：只替换超量程的点，其余保持原值
			val = (o > m_high || o < m_low) ? m : o;
		}
		else
		{
			double e = (m - knee) * dir;
			val = e > 0 ? knee + dir * e * k : m;
		}
		if (val > m_high) val = m_high;
		if (val < m_low) val = m_low;
		Emit(i, val, out);
	}
}
