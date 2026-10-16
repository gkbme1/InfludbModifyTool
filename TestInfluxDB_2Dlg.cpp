// TestInfluxDB_2Dlg.cpp : 实现文件
//
// 功能：输入一个标签名、量程上下限和时间范围（最长 6 个月），从本地 InfluxDB 读取该标签的值，
// 对超出量程的数据做平滑修正（见 DataSmoother.h），再按原时间戳写回原标签。
//
// 数据结构沿用原程序：表（measurement）默认 TagValue，标签键 TagName，字段 value / Quality。
// 写回时只写 value 字段，InfluxDB 会按“表 + 标签 + 时间戳”覆盖原点的 value，Quality 等其它字段保持不变。

#include "stdafx.h"
#include "TestInfluxDB_2.h"
#include "TestInfluxDB_2Dlg.h"
#include "afxdialogex.h"

#include <map>
#include <vector>
#include "curl/curl.h"
#include "json/json.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_REFRESH		1

static const int kChartBuckets = 800;					// 趋势图横向分桶数
static const long long kNsPerSec = 1000000000LL;
static const long long kChunkNs = 6LL * 3600 * kNsPerSec;	// 每次查询 6 小时的数据
static const int kLinesPerWrite = 5000;					// 每次写入的行数
static const int kQueryTimeout = 300;					// 查询超时（秒）
static const int kWriteTimeout = 60;					// 写入超时（秒）

/////////////////////////////////////////////////////////////////////////////
// 辅助函数

static std::string ToUtf8(const CString& str)
{
	CStringA strA(CW2A(str, CP_UTF8));
	return std::string(strA.GetString(), strA.GetLength());
}

static CString FromUtf8(const std::string& str)
{
	return CString(CA2W(str.c_str(), CP_UTF8));
}

// InfluxQL 字符串常量：'...'
static std::string QuoteString(const std::string& str)
{
	std::string out = "'";
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == '\'' || str[i] == '\\') out += '\\';
		out += str[i];
	}
	out += "'";
	return out;
}

// InfluxQL 标识符："..."
static std::string QuoteIdent(const std::string& str)
{
	std::string out = "\"";
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == '"' || str[i] == '\\') out += '\\';
		out += str[i];
	}
	out += "\"";
	return out;
}

// 行协议转义：表名转义逗号和空格，标签键/值还要转义等号
static std::string EscapeLine(const std::string& str, bool bTag)
{
	std::string out;
	for (size_t i = 0; i < str.size(); i++)
	{
		char c = str[i];
		if (c == ',' || c == ' ' || (bTag && c == '='))
		{
			out += '\\';
		}
		out += c;
	}
	return out;
}

static bool ParseDouble(const CString& str, double& value)
{
	CString s = str;
	s.Trim();
	if (s.IsEmpty())
	{
		return false;
	}
	wchar_t* pEnd = NULL;
	value = wcstod(s, &pEnd);
	return pEnd != NULL && *pEnd == 0;
}

static CTime AddMonths(const CTime& t, int nMonths)
{
	int y = t.GetYear();
	int m = t.GetMonth() - 1 + nMonths;
	y += m / 12;
	m = m % 12 + 1;
	static const int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int maxDay = days[m - 1];
	if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0))
	{
		maxDay = 29;
	}
	int d = t.GetDay() > maxDay ? maxDay : t.GetDay();
	return CTime(y, m, d, t.GetHour(), t.GetMinute(), t.GetSecond());
}

// 把界面上的时间转换为纳秒时间戳。
// bUtc = true：把输入的年月日时分秒直接当成 UTC 时间（原程序的做法，查询串直接拼 'Z'）；
// bUtc = false：按本机时区换算成 UTC。
static long long ToEpochNs(const CTime& t, bool bUtc)
{
	__time64_t sec;
	if (bUtc)
	{
		struct tm tmv = { 0 };
		tmv.tm_year = t.GetYear() - 1900;
		tmv.tm_mon = t.GetMonth() - 1;
		tmv.tm_mday = t.GetDay();
		tmv.tm_hour = t.GetHour();
		tmv.tm_min = t.GetMinute();
		tmv.tm_sec = t.GetSecond();
		sec = _mkgmtime64(&tmv);
	}
	else
	{
		sec = t.GetTime();
	}
	return (long long)sec * kNsPerSec;
}

/////////////////////////////////////////////////////////////////////////////
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestInfluxDB_2Dlg 对话框

CTestInfluxDB_2Dlg::CTestInfluxDB_2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestInfluxDB_2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pThread = NULL;
	m_bStop = FALSE;
	m_bChartDirty = false;
	m_llRead = 0;
	m_llOutOfRange = 0;
	m_llModified = 0;
	m_llSegments = 0;
	m_llWritten = 0;
	m_iProgress = 0;
}

void CTestInfluxDB_2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DT_START, m_dtStart);
	DDX_Control(pDX, IDC_DT_END, m_dtEnd);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_STATIC_TREND, m_trend);
	DDX_Control(pDX, IDC_BTN_START, m_btnStart);
}

BEGIN_MESSAGE_MAP(CTestInfluxDB_2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_START, &CTestInfluxDB_2Dlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_CHECK_BACKUP, &CTestInfluxDB_2Dlg::OnBnClickedBackup)
	ON_MESSAGE(WM_APP_LOG, &CTestInfluxDB_2Dlg::OnAppLog)
	ON_MESSAGE(WM_APP_DONE, &CTestInfluxDB_2Dlg::OnAppDone)
END_MESSAGE_MAP()


// CTestInfluxDB_2Dlg 消息处理程序

BOOL CTestInfluxDB_2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	curl_global_init(CURL_GLOBAL_ALL);

	SetDlgItemText(IDC_EDIT_URL, _T("http://localhost:8086"));
	SetDlgItemText(IDC_EDIT_DB, _T("mydb1"));
	SetDlgItemText(IDC_EDIT_MEASUREMENT, _T("TagValue"));
	SetDlgItemText(IDC_EDIT_BAND, _T("5"));
	SetDlgItemText(IDC_EDIT_BACKUP, _T("TagValue_Backup"));
	CheckDlgButton(IDC_CHECK_UTC, BST_CHECKED);
	CheckDlgButton(IDC_CHECK_BACKUP, BST_UNCHECKED);
	OnBnClickedBackup();

	m_dtStart.SetFormat(_T("yyyy-MM-dd HH:mm:ss"));
	m_dtEnd.SetFormat(_T("yyyy-MM-dd HH:mm:ss"));
	CTime now = CTime::GetCurrentTime();
	CTime today(now.GetYear(), now.GetMonth(), now.GetDay(), 0, 0, 0);
	CTime tStart = today - CTimeSpan(7, 0, 0, 0);
	m_dtStart.SetTime(&tStart);
	m_dtEnd.SetTime(&today);

	m_progress.SetRange(0, 1000);
	m_progress.SetPos(0);
	m_listLog.SetHorizontalExtent(2000);

	SetTimer(TIMER_REFRESH, 300, NULL);
	RefreshStatus();

	Log(_T("请输入标签名、量程上下限和时间范围，然后点击“开始处理”。"));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTestInfluxDB_2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestInfluxDB_2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTestInfluxDB_2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTestInfluxDB_2Dlg::OnOK()
{
	// 屏蔽回车键关闭对话框
}

void CTestInfluxDB_2Dlg::OnCancel()
{
	if (m_pThread != NULL)
	{
		if (AfxMessageBox(_T("正在处理中，确定要停止并退出吗？\n已经写入数据库的数据不会回退。"), MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}
		InterlockedExchange(&m_bStop, TRUE);
		CWaitCursor wait;
		WaitForThread();
	}
	KillTimer(TIMER_REFRESH);
	curl_global_cleanup();
	CDialogEx::OnCancel();
}

void CTestInfluxDB_2Dlg::WaitForThread()
{
	if (m_pThread != NULL)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
		delete m_pThread;
		m_pThread = NULL;
	}
}

void CTestInfluxDB_2Dlg::OnBnClickedBackup()
{
	GetDlgItem(IDC_EDIT_BACKUP)->EnableWindow(IsDlgButtonChecked(IDC_CHECK_BACKUP) == BST_CHECKED);
}

void CTestInfluxDB_2Dlg::Log(LPCTSTR szText)
{
	// 界面线程和工作线程都可以调用，统一通过消息投递到界面线程显示
	CString* pStr = new CString(CTime::GetCurrentTime().Format(_T("[%H:%M:%S] ")) + szText);
	if (!PostMessage(WM_APP_LOG, 0, (LPARAM)pStr))
	{
		delete pStr;
	}
}

LRESULT CTestInfluxDB_2Dlg::OnAppLog(WPARAM /*wParam*/, LPARAM lParam)
{
	CString* pStr = (CString*)lParam;
	if (pStr)
	{
		if (m_listLog.GetCount() > 5000)
		{
			m_listLog.DeleteString(0);
		}
		int idx = m_listLog.AddString(*pStr);
		m_listLog.SetTopIndex(idx);
		delete pStr;
	}
	return 0;
}

bool CTestInfluxDB_2Dlg::CollectParams(ProcessParams& params)
{
	CString str;

	GetDlgItemText(IDC_EDIT_URL, str);
	str.Trim();
	str.TrimRight(_T('/'));
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("请输入 InfluxDB 地址，例如 http://localhost:8086"));
		return false;
	}
	if (str.Find(_T("://")) < 0)
	{
		str = _T("http://") + str;
	}
	params.strUrl = ToUtf8(str);

	GetDlgItemText(IDC_EDIT_DB, str);
	str.Trim();
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("请输入数据库名。"));
		return false;
	}
	params.strDB = ToUtf8(str);

	GetDlgItemText(IDC_EDIT_MEASUREMENT, str);
	str.Trim();
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("请输入表名（measurement）。"));
		return false;
	}
	params.strMeasurement = ToUtf8(str);

	GetDlgItemText(IDC_EDIT_TAG, str);
	str.Trim();
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("请输入标签名。"));
		GetDlgItem(IDC_EDIT_TAG)->SetFocus();
		return false;
	}
	params.strTag = ToUtf8(str);

	GetDlgItemText(IDC_EDIT_LOW, str);
	if (!ParseDouble(str, params.fLow))
	{
		AfxMessageBox(_T("量程下限不是有效的数字。"));
		GetDlgItem(IDC_EDIT_LOW)->SetFocus();
		return false;
	}
	GetDlgItemText(IDC_EDIT_HIGH, str);
	if (!ParseDouble(str, params.fHigh))
	{
		AfxMessageBox(_T("量程上限不是有效的数字。"));
		GetDlgItem(IDC_EDIT_HIGH)->SetFocus();
		return false;
	}
	if (params.fLow >= params.fHigh)
	{
		AfxMessageBox(_T("量程下限必须小于量程上限。"));
		return false;
	}

	GetDlgItemText(IDC_EDIT_BAND, str);
	if (!ParseDouble(str, params.fBandPercent) || params.fBandPercent < 1 || params.fBandPercent > 30)
	{
		AfxMessageBox(_T("过渡带请输入 1 ~ 30 之间的数字（量程的百分比）。"));
		GetDlgItem(IDC_EDIT_BAND)->SetFocus();
		return false;
	}

	CTime tStart, tEnd;
	m_dtStart.GetTime(tStart);
	m_dtEnd.GetTime(tEnd);
	if (tStart >= tEnd)
	{
		AfxMessageBox(_T("开始时间必须早于结束时间。"));
		return false;
	}
	if (tEnd > AddMonths(tStart, 6))
	{
		AfxMessageBox(_T("时间范围不能超过 6 个月。"));
		return false;
	}
	bool bUtc = IsDlgButtonChecked(IDC_CHECK_UTC) == BST_CHECKED;
	params.llStartNs = ToEpochNs(tStart, bUtc);
	params.llEndNs = ToEpochNs(tEnd, bUtc);
	params.strStartText = tStart.Format(_T("%Y-%m-%d %H:%M:%S"));
	params.strEndText = tEnd.Format(_T("%Y-%m-%d %H:%M:%S"));

	params.bPreview = IsDlgButtonChecked(IDC_CHECK_PREVIEW) == BST_CHECKED;
	params.bBackup = !params.bPreview && IsDlgButtonChecked(IDC_CHECK_BACKUP) == BST_CHECKED;
	if (params.bBackup)
	{
		GetDlgItemText(IDC_EDIT_BACKUP, str);
		str.Trim();
		if (str.IsEmpty())
		{
			AfxMessageBox(_T("请输入备份表名，或取消勾选备份。"));
			return false;
		}
		params.strBackupMeasurement = ToUtf8(str);
		if (params.strBackupMeasurement == params.strMeasurement)
		{
			AfxMessageBox(_T("备份表名不能与原表名相同。"));
			return false;
		}
	}
	return true;
}

void CTestInfluxDB_2Dlg::SetBusy(bool bBusy)
{
	static const UINT ids[] = {
		IDC_EDIT_URL, IDC_EDIT_DB, IDC_EDIT_MEASUREMENT, IDC_EDIT_TAG, IDC_EDIT_LOW, IDC_EDIT_HIGH,
		IDC_EDIT_BAND, IDC_DT_START, IDC_DT_END, IDC_CHECK_UTC, IDC_CHECK_PREVIEW, IDC_CHECK_BACKUP
	};
	for (int i = 0; i < sizeof(ids) / sizeof(ids[0]); i++)
	{
		GetDlgItem(ids[i])->EnableWindow(!bBusy);
	}
	GetDlgItem(IDC_EDIT_BACKUP)->EnableWindow(!bBusy && IsDlgButtonChecked(IDC_CHECK_BACKUP) == BST_CHECKED);
	m_btnStart.SetWindowText(bBusy ? _T("停止处理") : _T("开始处理"));
	m_btnStart.EnableWindow(TRUE);
}

void CTestInfluxDB_2Dlg::OnBnClickedStart()
{
	if (m_pThread != NULL)
	{
		InterlockedExchange(&m_bStop, TRUE);
		m_btnStart.EnableWindow(FALSE);
		Log(_T("正在停止……"));
		return;
	}

	ProcessParams params;
	if (!CollectParams(params))
	{
		return;
	}

	if (!params.bPreview)
	{
		CString strMsg;
		strMsg.Format(_T("将修正标签 %s 在 %s ~ %s 内超出量程 [%g, %g] 的数据，并直接覆盖写回原标签。\n\n%s\n\n确定开始处理吗？"),
			(LPCTSTR)FromUtf8(params.strTag), (LPCTSTR)params.strStartText, (LPCTSTR)params.strEndText, params.fLow, params.fHigh,
			params.bBackup ? (LPCTSTR)(_T("被修改点的原始值会先备份到表 ") + FromUtf8(params.strBackupMeasurement) + _T("。"))
				: _T("注意：未勾选备份，原始值将无法恢复！"));
		if (AfxMessageBox(strMsg, MB_YESNO | MB_ICONWARNING) != IDYES)
		{
			return;
		}
	}

	m_params = params;
	{
		CSingleLock lock(&m_lock, TRUE);
		m_chart.Reset(kChartBuckets);
		m_chart.low = params.fLow;
		m_chart.high = params.fHigh;
		m_chart.strLeft = params.strStartText;
		m_chart.strRight = params.strEndText;
		m_bChartDirty = true;
		m_llRead = 0;
		m_llOutOfRange = 0;
		m_llModified = 0;
		m_llSegments = 0;
		m_llWritten = 0;
		m_iProgress = 0;
	}

	CString strMsg;
	strMsg.Format(_T("开始处理：标签 %s，量程 [%g, %g]，过渡带 %g%%，时间 %s ~ %s%s"),
		(LPCTSTR)FromUtf8(params.strTag), params.fLow, params.fHigh, params.fBandPercent,
		(LPCTSTR)params.strStartText, (LPCTSTR)params.strEndText, params.bPreview ? _T("（仅预览）") : _T(""));
	Log(strMsg);

	m_bStop = FALSE;
	m_pThread = AfxBeginThread(ProcessThreadProc, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (m_pThread == NULL)
	{
		Log(_T("创建处理线程失败。"));
		return;
	}
	m_pThread->m_bAutoDelete = FALSE;
	SetBusy(true);
	m_pThread->ResumeThread();
}

LRESULT CTestInfluxDB_2Dlg::OnAppDone(WPARAM wParam, LPARAM /*lParam*/)
{
	WaitForThread();
	SetBusy(false);
	RefreshStatus();

	CString strMsg;
	switch ((int)wParam)
	{
	case RESULT_OK:
		strMsg = m_params.bPreview ? _T("预览完成（未写入数据库）。") : _T("处理完成。");
		break;
	case RESULT_STOPPED:
		strMsg = _T("已停止。已写入的数据不会回退。");
		break;
	default:
		strMsg = _T("处理出错，已中止，请查看日志。");
		break;
	}
	Log(strMsg);
	return 0;
}

void CTestInfluxDB_2Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_REFRESH)
	{
		RefreshStatus();
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CTestInfluxDB_2Dlg::RefreshStatus()
{
	CString str;
	int iProgress = 0;
	bool bChart = false;
	CTrendCtrl::Data chart;
	{
		CSingleLock lock(&m_lock, TRUE);
		iProgress = m_iProgress;
		str.Format(_T("进度 %.1f%%    读取 %I64d 点，超量程 %I64d 点（%I64d 段），修正 %I64d 点，已写入 %I64d 点"),
			m_iProgress / 10.0, m_llRead, m_llOutOfRange, m_llSegments, m_llModified, m_llWritten);
		if (m_bChartDirty)
		{
			chart = m_chart;
			m_bChartDirty = false;
			bChart = true;
		}
	}
	m_progress.SetPos(iProgress);
	CString strOld;
	GetDlgItemText(IDC_STATIC_STATUS, strOld);
	if (strOld != str)
	{
		SetDlgItemText(IDC_STATIC_STATUS, str);
	}
	if (bChart)
	{
		m_trend.SetData(chart);
	}
}

/////////////////////////////////////////////////////////////////////////////
// 工作线程

UINT CTestInfluxDB_2Dlg::ProcessThreadProc(LPVOID pParam)
{
	CTestInfluxDB_2Dlg* pDlg = (CTestInfluxDB_2Dlg*)pParam;
	int iResult = RESULT_ERROR;
	try
	{
		iResult = pDlg->RunProcess();
	}
	catch (...)
	{
		pDlg->Log(_T("处理过程中发生异常。"));
		iResult = RESULT_ERROR;
	}
	pDlg->PostMessage(WM_APP_DONE, (WPARAM)iResult, 0);
	return 0;
}


bool CTestInfluxDB_2Dlg::WriteLines(InfluxdbClient& client, const std::string& strLines, LPCTSTR szWhat)
{
	if (strLines.empty())
	{
		return true;
	}
	std::string strWriteUrl = m_params.strUrl + "/write?";
	std::string strResponse;
	long lHttpCode = 0;
	int iRet = client.WriteEx(strWriteUrl, m_params.strDB, strLines, strResponse, kWriteTimeout, &lHttpCode);
	if (iRet != 0 || (lHttpCode != 204 && lHttpCode != 200))
	{
		CString strMsg;
		strMsg.Format(_T("写入%s失败：curl=%d（%s），HTTP=%ld，%s"), szWhat, iRet,
			(LPCTSTR)FromUtf8(curl_easy_strerror((CURLcode)iRet)), lHttpCode, (LPCTSTR)FromUtf8(strResponse));
		Log(strMsg);
		return false;
	}
	return true;
}

bool CTestInfluxDB_2Dlg::FlushWrites(InfluxdbClient& client, WriteBuffer& buf)
{
	if (buf.nLines == 0)
	{
		return true;
	}
	// 先写备份，备份成功后再覆盖原值
	if (!WriteLines(client, buf.strBackup, _T("备份数据")) || !WriteLines(client, buf.strLines, _T("修正数据")))
	{
		return false;
	}
	buf.llWritten += buf.nLines;
	buf.strLines.clear();
	buf.strBackup.clear();
	buf.nLines = 0;

	CSingleLock lock(&m_lock, TRUE);
	m_llWritten = buf.llWritten;
	return true;
}

// 处理平滑器输出：更新趋势图；修改过的点加入写入缓冲，缓冲满了就写入数据库
bool CTestInfluxDB_2Dlg::HandleResults(InfluxdbClient& client, const std::string& strTags,
	const std::vector<SmoothResult>& results, WriteBuffer& buf)
{
	const ProcessParams& p = m_params;
	const long long llSpan = p.llEndNs - p.llStartNs > 0 ? p.llEndNs - p.llStartNs : 1;

	if (!results.empty())
	{
		CSingleLock lock(&m_lock, TRUE);
		for (size_t i = 0; i < results.size(); i++)
		{
			const SmoothResult& r = results[i];
			long long llIdx = (r.t - p.llStartNs) * kChartBuckets / llSpan;
			size_t idx = llIdx < 0 ? 0 : (llIdx >= kChartBuckets ? kChartBuckets - 1 : (size_t)llIdx);
			if (!m_chart.has[idx])
			{
				m_chart.has[idx] = 1;
				m_chart.origMin[idx] = m_chart.origMax[idx] = r.orig;
				m_chart.newMin[idx] = m_chart.newMax[idx] = r.val;
			}
			else
			{
				if (r.orig < m_chart.origMin[idx]) m_chart.origMin[idx] = r.orig;
				if (r.orig > m_chart.origMax[idx]) m_chart.origMax[idx] = r.orig;
				if (r.val < m_chart.newMin[idx]) m_chart.newMin[idx] = r.val;
				if (r.val > m_chart.newMax[idx]) m_chart.newMax[idx] = r.val;
			}
		}
		m_bChartDirty = true;
	}

	const std::string strMeasLine = EscapeLine(p.strMeasurement, false) + strTags;
	const std::string strBackupLine = EscapeLine(p.strBackupMeasurement, false) + strTags;
	for (size_t i = 0; i < results.size(); i++)
	{
		const SmoothResult& r = results[i];
		if (!r.modified)
		{
			continue;
		}
		buf.llModified++;
		if (p.bPreview)
		{
			continue;
		}

		// 只写 value 字段，时间戳与原点完全一致（纳秒），从而覆盖原值
		char szLine[128];
		sprintf_s(szLine, " value=%.6f %I64d\n", r.val, r.t);
		buf.strLines += strMeasLine + szLine;
		if (p.bBackup)
		{
			sprintf_s(szLine, " value=%.6f %I64d\n", r.orig, r.t);
			buf.strBackup += strBackupLine + szLine;
		}
		buf.nLines++;
		if (buf.nLines >= kLinesPerWrite && !FlushWrites(client, buf))
		{
			return false;
		}
	}
	return true;
}

int CTestInfluxDB_2Dlg::RunProcess()
{
	const ProcessParams& p = m_params;
	const long long llSpan = p.llEndNs - p.llStartNs > 0 ? p.llEndNs - p.llStartNs : 1;
	const std::string strQueryUrl = p.strUrl + "/query?epoch=ns&";

	InfluxdbClient client;
	WriteBuffer buf;
	// 每个序列（标签组合）一个平滑器。通常只有 TagName 一个标签，即只有一个序列
	std::map<std::string, CDataSmoother> smoothers;
	std::vector<SmoothResult> results;

	long long llA = p.llStartNs;
	bool bLast = false;
	while (!bLast)
	{
		if (m_bStop)
		{
			// 缓冲中的点都已最终确定，写完再退出
			FlushWrites(client, buf);
			return RESULT_STOPPED;
		}

		long long llB = llA + kChunkNs;
		if (llB >= p.llEndNs)
		{
			llB = p.llEndNs;
			bLast = true;
		}

		char szCond[128];
		sprintf_s(szCond, bLast ? "time >= %I64d AND time <= %I64d" : "time >= %I64d AND time < %I64d", llA, llB);
		std::string strQuery = "SELECT \"value\" FROM " + QuoteIdent(p.strMeasurement)
			+ " WHERE \"TagName\" = " + QuoteString(p.strTag) + " AND " + szCond + " GROUP BY *";

		std::string strResponse;
		int iRet = client.query(strQueryUrl, p.strDB, strQuery, strResponse, kQueryTimeout);
		if (iRet != 0)
		{
			CString strMsg;
			strMsg.Format(_T("查询 InfluxDB 失败（curl=%d：%s），请检查地址 %s 是否正确、InfluxDB 是否已启动。"),
				iRet, (LPCTSTR)FromUtf8(curl_easy_strerror((CURLcode)iRet)), (LPCTSTR)FromUtf8(p.strUrl));
			Log(strMsg);
			return RESULT_ERROR;
		}

		Json::Reader reader;
		Json::Value root;
		if (!reader.parse(strResponse, root) || !root.isObject())
		{
			Log(_T("无法解析 InfluxDB 返回的数据：") + FromUtf8(strResponse.substr(0, 300)));
			return RESULT_ERROR;
		}
		std::string strError;
		if (root["error"].isString())
		{
			strError = root["error"].asString();
		}
		else if (root["results"].isArray() && root["results"].size() > 0 && root["results"][0u]["error"].isString())
		{
			strError = root["results"][0u]["error"].asString();
		}
		if (!strError.empty())
		{
			Log(_T("InfluxDB 返回错误：") + FromUtf8(strError));
			return RESULT_ERROR;
		}

		const Json::Value& series = root["results"][0u]["series"];
		for (Json::ArrayIndex s = 0; series.isArray() && s < series.size(); s++)
		{
			const Json::Value& one = series[s];

			// 由返回的标签组合生成写入用的标签串，保证写回的是同一个序列
			std::string strTags;
			const Json::Value& tags = one["tags"];
			if (tags.isObject())
			{
				Json::Value::Members names = tags.getMemberNames();
				for (size_t k = 0; k < names.size(); k++)
				{
					std::string strVal = tags[names[k]].isString() ? tags[names[k]].asString() : "";
					if (!strVal.empty())
					{
						strTags += "," + EscapeLine(names[k], true) + "=" + EscapeLine(strVal, true);
					}
				}
			}
			if (strTags.empty())
			{
				strTags = ",TagName=" + EscapeLine(p.strTag, true);
			}

			std::vector<SmoothPoint> pts;
			const Json::Value& values = one["values"];
			for (Json::ArrayIndex i = 0; values.isArray() && i < values.size(); i++)
			{
				const Json::Value& row = values[i];
				if (!row.isArray() || row.size() < 2 || !row[0u].isNumeric() || !row[1u].isNumeric())
				{
					continue;
				}
				SmoothPoint pt;
				pt.t = row[0u].asInt64();
				pt.v = row[1u].asDouble();
				pts.push_back(pt);
			}

			std::map<std::string, CDataSmoother>::iterator it = smoothers.find(strTags);
			if (it == smoothers.end())
			{
				it = smoothers.insert(std::make_pair(strTags, CDataSmoother())).first;
				it->second.Init(p.fLow, p.fHigh, p.fBandPercent);
				if (smoothers.size() > 1)
				{
					Log(_T("注意：该标签名对应多个序列，将分别处理：") + FromUtf8(strTags.substr(1)));
				}
			}

			results.clear();
			it->second.Push(pts, results);
			if (!HandleResults(client, strTags, results, buf))
			{
				return RESULT_ERROR;
			}
		}

		// 最后一批：把各平滑器中尚未输出的数据全部输出并写入
		if (bLast)
		{
			for (std::map<std::string, CDataSmoother>::iterator it = smoothers.begin(); it != smoothers.end(); ++it)
			{
				results.clear();
				it->second.Flush(results);
				if (!HandleResults(client, it->first, results, buf))
				{
					return RESULT_ERROR;
				}
			}
			if (!FlushWrites(client, buf))
			{
				return RESULT_ERROR;
			}
		}

		// 更新统计
		long long llRead = 0, llOut = 0, llSeg = 0;
		for (std::map<std::string, CDataSmoother>::const_iterator it = smoothers.begin(); it != smoothers.end(); ++it)
		{
			llRead += it->second.Stats().total;
			llOut += it->second.Stats().outOfRange;
			llSeg += it->second.Stats().segments;
		}
		{
			CSingleLock lock(&m_lock, TRUE);
			m_llRead = llRead;
			m_llOutOfRange = llOut;
			m_llSegments = llSeg;
			m_llModified = buf.llModified;
			m_iProgress = (int)((llB - p.llStartNs) * 1000 / llSpan);
		}

		llA = llB;
	}

	CString strMsg;
	if (smoothers.empty())
	{
		strMsg.Format(_T("在该时间范围内没有查到标签 %s 的数据，请检查标签名、表名和时间范围。"), (LPCTSTR)FromUtf8(p.strTag));
	}
	else
	{
		CSingleLock lock(&m_lock, TRUE);
		strMsg.Format(_T("共读取 %I64d 点，超量程 %I64d 点（%I64d 段），修正 %I64d 点，写入 %I64d 点。"),
			m_llRead, m_llOutOfRange, m_llSegments, m_llModified, m_llWritten);
	}
	Log(strMsg);
	return RESULT_OK;
}
