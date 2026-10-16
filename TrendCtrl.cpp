// TrendCtrl.cpp : 处理前后趋势对比图
//

#include "stdafx.h"
#include "TrendCtrl.h"

BEGIN_MESSAGE_MAP(CTrendCtrl, CStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CTrendCtrl::CTrendCtrl()
{
}

void CTrendCtrl::SetData(const Data& data)
{
	m_data = data;
	if (GetSafeHwnd())
	{
		Invalidate(FALSE);
	}
}

BOOL CTrendCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

static int ValueToY(double v, double yMin, double yMax, const CRect& rc)
{
	return rc.bottom - (int)((v - yMin) / (yMax - yMin) * rc.Height());
}

void CTrendCtrl::DrawSeries(CDC* pDC, const CRect& rc, const std::vector<double>& vMin,
	const std::vector<double>& vMax, double yMin, double yMax, COLORREF color)
{
	size_t n = m_data.has.size();
	if (n == 0)
	{
		return;
	}

	CPen pen(PS_SOLID, 1, color);
	CPen* pOldPen = pDC->SelectObject(&pen);

	// 每个桶画一条最小值到最大值的竖线，并与前一个桶相连，数据有缺口时断开
	std::vector<POINT> pts;
	for (size_t i = 0; i <= n; i++)
	{
		if (i == n || !m_data.has[i])
		{
			if (pts.size() >= 2)
			{
				pDC->Polyline(&pts[0], (int)pts.size());
			}
			else if (pts.size() == 1)
			{
				pDC->SetPixel(pts[0], color);
			}
			pts.clear();
			continue;
		}
		int x = rc.left + (int)((double)i / (n > 1 ? n - 1 : 1) * rc.Width());
		POINT p1 = { x, ValueToY(vMin[i], yMin, yMax, rc) };
		POINT p2 = { x, ValueToY(vMax[i], yMin, yMax, rc) };
		pts.push_back(p1);
		pts.push_back(p2);
	}

	pDC->SelectObject(pOldPen);
}

void CTrendCtrl::OnPaint()
{
	CPaintDC dc(this);

	CRect rcClient;
	GetClientRect(&rcClient);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);

	CFont* pOldFont = NULL;
	if (GetParent() && GetParent()->GetFont())
	{
		pOldFont = memDC.SelectObject(GetParent()->GetFont());
	}

	memDC.FillSolidRect(&rcClient, RGB(255, 255, 255));
	memDC.SetBkMode(TRANSPARENT);

	CRect rcPlot = rcClient;
	rcPlot.DeflateRect(60, 20, 12, 22);

	memDC.Draw3dRect(&rcPlot, RGB(180, 180, 180), RGB(180, 180, 180));

	// 图例
	memDC.SetTextColor(RGB(120, 120, 120));
	memDC.TextOut(rcPlot.left, 3, CString(_T("灰色：原始值")));
	memDC.SetTextColor(RGB(0, 90, 220));
	memDC.TextOut(rcPlot.left + 100, 3, CString(_T("蓝色：处理后")));
	memDC.SetTextColor(RGB(220, 0, 0));
	memDC.TextOut(rcPlot.left + 200, 3, CString(_T("红色虚线：量程上下限")));

	bool bAny = false;
	double yMin = m_data.low;
	double yMax = m_data.high;
	for (size_t i = 0; i < m_data.has.size(); i++)
	{
		if (!m_data.has[i]) continue;
		bAny = true;
		if (m_data.origMin[i] < yMin) yMin = m_data.origMin[i];
		if (m_data.origMax[i] > yMax) yMax = m_data.origMax[i];
	}

	memDC.SetTextColor(RGB(0, 0, 0));
	if (!bAny || yMax <= yMin)
	{
		memDC.DrawText(CString(_T("暂无数据")), &rcPlot, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		double pad = (yMax - yMin) * 0.05;
		yMin -= pad;
		yMax += pad;

		// 量程上下限
		CPen penLimit(PS_DOT, 1, RGB(220, 0, 0));
		CPen* pOldPen = memDC.SelectObject(&penLimit);
		int yHigh = ValueToY(m_data.high, yMin, yMax, rcPlot);
		int yLow = ValueToY(m_data.low, yMin, yMax, rcPlot);
		memDC.MoveTo(rcPlot.left, yHigh);
		memDC.LineTo(rcPlot.right, yHigh);
		memDC.MoveTo(rcPlot.left, yLow);
		memDC.LineTo(rcPlot.right, yLow);
		memDC.SelectObject(pOldPen);

		DrawSeries(&memDC, rcPlot, m_data.origMin, m_data.origMax, yMin, yMax, RGB(170, 170, 170));
		DrawSeries(&memDC, rcPlot, m_data.newMin, m_data.newMax, yMin, yMax, RGB(0, 90, 220));

		// 纵轴刻度
		CString str;
		double ticks[4] = { yMax, m_data.high, m_data.low, yMin };
		for (int k = 0; k < 4; k++)
		{
			str.Format(_T("%.2f"), ticks[k]);
			int y = ValueToY(ticks[k], yMin, yMax, rcPlot);
			CRect rcText(0, y - 8, rcPlot.left - 4, y + 8);
			memDC.SetTextColor(k == 1 || k == 2 ? RGB(220, 0, 0) : RGB(0, 0, 0));
			memDC.DrawText(str, &rcText, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
	}

	// 横轴时间
	memDC.SetTextColor(RGB(0, 0, 0));
	CRect rcAxis(rcPlot.left, rcPlot.bottom + 3, rcPlot.right, rcClient.bottom);
	memDC.DrawText(m_data.strLeft, &rcAxis, DT_LEFT | DT_TOP | DT_SINGLELINE);
	memDC.DrawText(m_data.strRight, &rcAxis, DT_RIGHT | DT_TOP | DT_SINGLELINE);

	dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);

	if (pOldFont)
	{
		memDC.SelectObject(pOldFont);
	}
	memDC.SelectObject(pOldBmp);
}
