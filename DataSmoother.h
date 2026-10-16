// DataSmoother.h : 超量程数据平滑修正
//
// 处理思路（针对温度、压力等缓慢变化的量）：
//   1. 以量程上下限 [L, H] 为界，向内留出一个“过渡带” band（量程的百分比），
//      得到上拐点 Ku = H - band、下拐点 Kl = L + band。
//   2. 连续落在拐点之外的点组成一个“越界段”。只有段内存在真正超出量程的点时才修正，
//      仍在量程内的数据保持原值不动。
//   3. 对段内数据先做 5 点中值滤波（剔除孤立尖峰），再以拐点为基准做线性压缩：
//         新值 = Ku + (中值 - Ku) * k，  k = 0.85 * band / (段内峰值 - Ku)
//      曲线的起伏形状按比例保留，段首尾与相邻正常数据连续衔接，峰值落在量程内，
//      不会出现被削平的一条直线。
//   4. 数据按时间顺序分批送入，越界段跨批次时会等待该段结束后再统一修正。
#pragma once

#include <cstddef>
#include <vector>

struct SmoothPoint
{
	long long t;	// 时间戳（纳秒）
	double v;		// 原始值
};

struct SmoothResult
{
	long long t;	// 时间戳（纳秒）
	double orig;	// 原始值
	double val;		// 处理后的值
	bool modified;	// 是否被修改
};

struct SmoothStats
{
	long long total;		// 已处理的总点数
	long long outOfRange;	// 超出量程的点数
	long long modified;		// 被修改的点数
	long long segments;		// 被修正的越界段数
};

class CDataSmoother
{
public:
	CDataSmoother();

	// bandPercent: 过渡带占量程的百分比，取值范围 [1, 30]
	void Init(double low, double high, double bandPercent);

	// 按时间升序追加数据，已能确定结果的点追加到 out
	void Push(const std::vector<SmoothPoint>& pts, std::vector<SmoothResult>& out);

	// 数据全部送入后调用，输出剩余所有点
	void Flush(std::vector<SmoothResult>& out);

	const SmoothStats& Stats() const { return m_stats; }

private:
	void Process(bool bFinal, std::vector<SmoothResult>& out);
	void FixSegment(size_t a, size_t b, int dir, std::vector<SmoothResult>& out);
	void Emit(size_t i, double val, std::vector<SmoothResult>& out);
	double Median(size_t i) const;
	int Classify(double v) const;

	double m_low;
	double m_high;
	double m_band;
	double m_kneeLow;
	double m_kneeHigh;

	std::vector<SmoothPoint> m_buf;	// 前 m_hist 个点是已输出的历史点，仅用于中值滤波
	size_t m_hist;
	SmoothStats m_stats;
};
