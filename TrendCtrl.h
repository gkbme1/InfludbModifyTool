// TrendCtrl.h : 处理前后趋势对比图
//
#pragma once

#include <vector>

class CTrendCtrl : public CStatic
{
public:
	// 按时间等分成若干桶，每个桶记录原始值和处理后值的最小/最大值
	struct Data
	{
		std::vector<double> origMin;
		std::vector<double> origMax;
		std::vector<double> newMin;
		std::vector<double> newMax;
		std::vector<char> has;
		double low;
		double high;
		CString strLeft;	// 横轴起点文字
		CString strRight;	// 横轴终点文字

		Data() : low(0), high(0) {}
		void Reset(size_t nBuckets)
		{
			origMin.assign(nBuckets, 0);
			origMax.assign(nBuckets, 0);
			newMin.assign(nBuckets, 0);
			newMax.assign(nBuckets, 0);
			has.assign(nBuckets, 0);
		}
	};

	CTrendCtrl();
	void SetData(const Data& data);

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

private:
	void DrawSeries(CDC* pDC, const CRect& rc, const std::vector<double>& vMin,
		const std::vector<double>& vMax, double yMin, double yMax, COLORREF color);

	Data m_data;
};
