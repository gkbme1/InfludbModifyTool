// TestInfluxDB_2Dlg.h : 头文件
//

#pragma once

#include <afxmt.h>
#include <string>
#include "InfluxdbClient.h"
#include <vector>
#include "DataSmoother.h"
#include "TrendCtrl.h"

#define WM_APP_LOG		(WM_APP + 100)	// 工作线程输出日志，lParam 为 new 出来的 CString*
#define WM_APP_DONE		(WM_APP + 101)	// 工作线程结束，wParam 为结果码

// 处理参数（开始处理时从界面读取，处理过程中只读）
struct ProcessParams
{
	std::string strUrl;			// 例如 http://localhost:8086
	std::string strDB;
	std::string strMeasurement;
	std::string strTag;
	std::string strBackupMeasurement;
	double fLow;
	double fHigh;
	double fBandPercent;
	long long llStartNs;
	long long llEndNs;
	bool bPreview;
	bool bBackup;
	CString strStartText;
	CString strEndText;
};

// CTestInfluxDB_2Dlg 对话框
class CTestInfluxDB_2Dlg : public CDialogEx
{
// 构造
public:
	CTestInfluxDB_2Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TESTINFLUXDB_2_DIALOG };

	enum
	{
		RESULT_OK = 0,
		RESULT_STOPPED = 1,
		RESULT_ERROR = 2
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedBackup();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnAppLog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAppDone(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	// 待写入数据库的缓冲
	struct WriteBuffer
	{
		std::string strLines;	// 修正后的值（写回原表）
		std::string strBackup;	// 原始值（写入备份表）
		int nLines;
		long long llModified;
		long long llWritten;
		WriteBuffer() : nLines(0), llModified(0), llWritten(0) {}
	};

	static UINT ProcessThreadProc(LPVOID pParam);
	int RunProcess();
	bool HandleResults(InfluxdbClient& client, const std::string& strTags,
		const std::vector<SmoothResult>& results, WriteBuffer& buf);
	bool FlushWrites(InfluxdbClient& client, WriteBuffer& buf);
	bool WriteLines(InfluxdbClient& client, const std::string& strLines, LPCTSTR szWhat);

	bool CollectParams(ProcessParams& params);
	void SetBusy(bool bBusy);
	void Log(LPCTSTR szText);
	void RefreshStatus();
	void WaitForThread();

	CDateTimeCtrl m_dtStart;
	CDateTimeCtrl m_dtEnd;
	CProgressCtrl m_progress;
	CListBox m_listLog;
	CTrendCtrl m_trend;
	CButton m_btnStart;

	CWinThread* m_pThread;
	volatile LONG m_bStop;
	ProcessParams m_params;

	// 以下成员由工作线程写、界面线程读，访问时需加锁
	CCriticalSection m_lock;
	CTrendCtrl::Data m_chart;
	bool m_bChartDirty;
	long long m_llRead;
	long long m_llOutOfRange;
	long long m_llModified;
	long long m_llSegments;
	long long m_llWritten;
	int m_iProgress;	// 0 ~ 1000
};
