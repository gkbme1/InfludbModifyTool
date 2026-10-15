
// TestInfluxDB_2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TestInfluxDB_2.h"
#include "TestInfluxDB_2Dlg.h"
#include "afxdialogex.h"


#include <iostream>
#include "json/json.h"


#include "LightEvent.h"
#include "WinRedefine.h"
#include <stdlib.h>
#include <vector>

time_t g_StartTime = 0;
int g_ProcessAllPointNum = 0;
int g_ProcessAllDayNum = 0;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#include "QueryInfluxdbThread.h"

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框


char g_strQueryUrlq[200] = "http://localhost:8086/query?";//"http://172.21.48.28:8086/query?"
char g_strWriteUrlq[200] = "http://localhost:8086/write?";//"http://172.21.48.28:8086/write?";

char g_strCreatUrlq[200] = "http://localhost:8086/query?q=CREATE+DATABASE+mydb1";//"http://172.21.48.28:8086/query?q=CREATE+DATABASE+mydb1";
char g_strDBName[200] = "mydb1";

DWORD WINAPI Event_Maker(int *iIndex);

DWORD WINAPI Event_Maker22(int *iIndex);

InfluxdbClient* g_pClient = NULL;

const int MAXTAGATRIBUTION_LENGTH = 512 + 1;//点项值最大长度
const int MAXTAGATRIBUTION_COUNT = 360;//点表最大列数


NeedModifyPoints  **pNeedModifyPointList = NULL;

vector<NeedModifyPoints*> g_AllTagListInfo;


BOOL g_bExit = FALSE;



int g_CurrentProcessPointNumIndex = 0;
int g_CurrentProcessDayIndex = 0;
#define TIMER1 1

time_t FormatTime3(char * szTime)
{
	struct tm tm1;
	time_t time1;


	sscanf(szTime, "%4d/%2d/%2d/%2d/%2d/%2d",
		&tm1.tm_year,
		&tm1.tm_mon,
		&tm1.tm_mday,
		&tm1.tm_hour,
		&tm1.tm_min,
		&tm1.tm_sec);

	tm1.tm_year -= 1900;
	tm1.tm_mon--;


	tm1.tm_isdst = -1;

	time1 = mktime(&tm1);
	return time1;
}



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


// CTestInfluxDB_2Dlg 对话框




const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static	bool  IsBase64(const char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}
//转码
std::string Base64Encode(const char * bytes_to_encode, unsigned int in_len) {
	{
		std::string ret;
		int i = 0;
		int j = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		while (in_len--)
		{
			char_array_3[i++] = *(bytes_to_encode++);
			if (i == 3)
			{
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;
				for (i = 0; (i < 4); i++)
				{
					ret += base64_chars[char_array_4[i]];
				}
				i = 0;
			}
		}
		if (i)
		{
			for (j = i; j < 3; j++)
			{
				char_array_3[j] = '\0';
			}

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (j = 0; (j < i + 1); j++)
			{
				ret += base64_chars[char_array_4[j]];
			}

			while ((i++ < 3))
			{
				ret += '=';
			}

		}
		return ret;
	}
}
std::string Base64Decode(std::string const & encoded_string) {
	int in_len = (int)encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && IsBase64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}
	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}


CTestInfluxDB_2Dlg::CTestInfluxDB_2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestInfluxDB_2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pix = 0;
	m_Result = _T("");
	//  m_cur_ret = 0;
	//  m_current_Times = 0;
	//  m_ErroNum = 0;
	m_cur_ret = 0;
	m_ErroNum = 0;
	//  m_current_Times = _T("");
	m_current_Times = 0;
	m_strResult = _T("");
	m_ProcessAllDays = 0;
	m_ProcessAllPointNum = 0;
	m_StartProcesPointIndex = 0;
	m_StartProcessDayIndex = 0;
	m_bIsInProcessing = false;
}

void CTestInfluxDB_2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListMacsPoint);
	DDX_Text(pDX, IDC_EDIT_ALL_DAYS, m_ProcessAllDays);
	DDV_MinMaxInt(pDX, m_ProcessAllDays, 0, 1000);
	DDX_Text(pDX, IDC_EDIT_ALL_POINTNUM, m_ProcessAllPointNum);
	DDV_MinMaxInt(pDX, m_ProcessAllPointNum, 0, 1000);
	DDX_Text(pDX, IDC_EDIT_STAERT_POINTINDEX, m_StartProcesPointIndex);
	DDV_MinMaxInt(pDX, m_StartProcesPointIndex, 1, 1000);
	DDX_Text(pDX, IDC_EDIT_START_DAYINDEX, m_StartProcessDayIndex);
	DDV_MinMaxInt(pDX, m_StartProcessDayIndex, 1, 1000);
	DDX_Control(pDX, IDOK_START_STOP, m_Start_Stop_Process);
}

BEGIN_MESSAGE_MAP(CTestInfluxDB_2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CTestInfluxDB_2Dlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CTestInfluxDB_2Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK2, &CTestInfluxDB_2Dlg::OnBnClickedStartProcess)
	ON_WM_TIMER()
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


	SetTimer(TIMER1,1000,NULL);



	CString strPrompt = _T("序号"); 
	m_ListMacsPoint.InsertColumn( 0, strPrompt, LVCFMT_LEFT, 50);
	strPrompt = _T("旧点名"); ;
	m_ListMacsPoint.InsertColumn( 1, strPrompt, LVCFMT_LEFT, 200);

	strPrompt= _T("新点名"); ;
	m_ListMacsPoint.InsertColumn( 2, strPrompt, LVCFMT_LEFT, 200);

	strPrompt= _T("第几个点"); ;
	m_ListMacsPoint.InsertColumn( 3, strPrompt, LVCFMT_LEFT, 70);

	strPrompt= _T("第几天"); ;
	m_ListMacsPoint.InsertColumn( 4, strPrompt, LVCFMT_LEFT, 50);

	strPrompt= _T("时间"); ;
	m_ListMacsPoint.InsertColumn( 5, strPrompt, LVCFMT_LEFT, 100);

	strPrompt= _T("设定值"); ;
	m_ListMacsPoint.InsertColumn( 6, strPrompt, LVCFMT_LEFT, 100);

	strPrompt= _T("偏差值"); ;
	m_ListMacsPoint.InsertColumn( 7, strPrompt, LVCFMT_LEFT, 100);

	m_ListMacsPoint.InitListCtrl();	


	// TODO: 在此添加额外的初始化代码
	char chDebug[1000] = {0};
	int iTick1 = GetTickCount();


	m_pHttpClient = new CHttpClient();
	m_pClient = new InfluxdbClient();
	g_pClient = m_pClient;



	//创建数据库
	string strUrl = g_strCreatUrlq;//"http://localhost:8086/query?q=CREATE+DATABASE+mydb1";

	//string strUrlw = "http://localhost:6666/write?"
	int iRet = -1;
	string strResponse;
	iRet = m_pClient->CreateDB(g_strQueryUrlq, g_strDBName,strResponse);
	cout<<"iRet="<<iRet<<",Response="<<strResponse<<endl;

	m_pix = 1280;
	UpdateData(FALSE);


	//初始化列表控件
	/*m_ListCtrl.SetExtendedStyle( m_ListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_AUTOAUTOARRANGE );
	m_ListCtrl.InsertColumn( 1, _T("序号"), LVCFMT_CENTER, 40 );
	m_ListCtrl.InsertColumn( 2, _T("时间"), LVCFMT_CENTER, 150 );

	m_ListCtrl.InsertColumn( 3, _T("最小值"), LVCFMT_CENTER, 100 );
	m_ListCtrl.InsertColumn( 4, _T("最小值质量位"), LVCFMT_CENTER, 100 );
	m_ListCtrl.InsertColumn( 5, _T("最大值"), LVCFMT_CENTER, 100 );
	m_ListCtrl.InsertColumn( 6, _T("最大值质量位"), LVCFMT_CENTER, 100 );

	m_ListCtrl.InsertColumn( 7, _T("取舍值"), LVCFMT_CENTER, 100 );
	m_ListCtrl.InsertColumn( 8, _T("取舍值质量位"), LVCFMT_CENTER, 100 );


	m_ListCtrl2.SetExtendedStyle( m_ListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_AUTOAUTOARRANGE );
	m_ListCtrl2.InsertColumn( 1, _T("序号"), LVCFMT_CENTER, 40 );
	m_ListCtrl2.InsertColumn( 2, _T("时间"), LVCFMT_CENTER, 150 );

	m_ListCtrl2.InsertColumn( 3, _T("最小值"), LVCFMT_CENTER, 100 );
	m_ListCtrl2.InsertColumn( 4, _T("最小值质量位"), LVCFMT_CENTER, 100 );
	m_ListCtrl2.InsertColumn( 5, _T("最大值"), LVCFMT_CENTER, 100 );
	m_ListCtrl2.InsertColumn( 6, _T("最大值质量位"), LVCFMT_CENTER, 100 );

	m_ListCtrl2.InsertColumn( 7, _T("取舍值"), LVCFMT_CENTER, 100 );
	m_ListCtrl2.InsertColumn( 8, _T("取舍值质量位"), LVCFMT_CENTER, 100 );


	m_ListCtrl3.SetExtendedStyle( m_ListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_AUTOAUTOARRANGE );
	m_ListCtrl3.InsertColumn( 1, _T("序号"), LVCFMT_CENTER, 40 );
	m_ListCtrl3.InsertColumn( 2, _T("时间"), LVCFMT_CENTER, 150 );

	m_ListCtrl3.InsertColumn( 3, _T("最小值"), LVCFMT_CENTER, 100 );
	m_ListCtrl3.InsertColumn( 4, _T("最小值质量位"), LVCFMT_CENTER, 100 );
	m_ListCtrl3.InsertColumn( 5, _T("最大值"), LVCFMT_CENTER, 100 );
	m_ListCtrl3.InsertColumn( 6, _T("最大值质量位"), LVCFMT_CENTER, 100 );

	m_ListCtrl3.InsertColumn( 7, _T("取舍值"), LVCFMT_CENTER, 100 );
	m_ListCtrl3.InsertColumn( 8, _T("取舍值质量位"), LVCFMT_CENTER, 100 );

	*/
	int iTick2 = GetTickCount();
	sprintf(chDebug,"gao0708 Time :%d ",iTick2-iTick1);
	OutputDebugStringA(chDebug);


	ReadTagsFromCSV();

	
	int iTick3 = GetTickCount();
	sprintf(chDebug,"gao0708 Time :%d 2222 ",iTick3-iTick2);
	OutputDebugStringA(chDebug);	



	//for(int k =0;k<1;k++)
	//{
	//	
	//	//m_QueryInfluxdbThread[k].m_iIndex = k+1;
	//	m_QueryInfluxdbThread[k].m_iIndex = 1;
	//	m_QueryInfluxdbThread[k].StartThread();

	//}
	
	int iTick4 = GetTickCount();
	sprintf(chDebug,"gao0708 Time :%d 333 ",iTick4-iTick3);
	OutputDebugStringA(chDebug);	

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTestInfluxDB_2Dlg::ReadTagsFromCSV()
{
	
    char fileName[MAX_PATH + 1];
	GetModuleFileNameA(0, fileName, sizeof(fileName));//改为进程启动路径,当前路径是会变化的 .
	string filePath = fileName;//..\gtplus\bin\gtplus.exe
	filePath = filePath.substr(0, filePath.rfind('\\'));//Left(filePath.ReverseFind('\\'));//..\gtplus\bin
	
	filePath += "\\Points.csv";//..\gtplus\Runtime\  

	string fileNameStr = filePath;
	FILE * fp = fopen(fileNameStr.c_str(), "rb");
	if (fp == nullptr)
	{
		return;
	}
	fseek(fp, 0L, SEEK_END);
	int len = ftell(fp);
	if (0 == len)
	{
		fclose(fp);
		return;
	}

	char *fileBuffer = new char[len + 1];
	memset(fileBuffer, 0L, len + 1);
	fseek(fp, 0L, SEEK_SET);
	fread(fileBuffer, len, 1, fp);
	fclose(fp);

	char ** charTemStringBuffer = new char*[MAXTAGATRIBUTION_COUNT];
	int *temStringBufferLength = new int[MAXTAGATRIBUTION_COUNT];
	for (int n = 0; n < MAXTAGATRIBUTION_COUNT; n++)
	{
		charTemStringBuffer[n] = new char[MAXTAGATRIBUTION_LENGTH];
		temStringBufferLength[n] = 0;
	}


	int iFindPosLast = 0;
	int iFindPosCurrent = 0;

	int iRowNum = 0; //行数
	int iColNum = 0;//每一行的字节数 即行按字节算的列数
	int iTagNum = 0;



	pNeedModifyPointList = new NeedModifyPoints*[100];

	for(int mm=0;mm<100;mm++)
	{
		pNeedModifyPointList[mm] = new NeedModifyPoints[100];
	}
	for (int k = 0; k < len; k++)
	{
		if (fileBuffer[k] == '\n')//行结束标记
		{
					
			ProcessOneData(iRowNum, iColNum,100, charTemStringBuffer,pNeedModifyPointList);
			NeedModifyPoints theNeedModifyPoints;
			theNeedModifyPoints.chNewPointName;
			theNeedModifyPoints.chNewPointName;


			iColNum = 0;
			iFindPosLast = k + 1;
			continue;

		}

		if (fileBuffer[k] == ',')
		{
			if ((iColNum + 1) > MAXTAGATRIBUTION_COUNT)
			{
				break;
			}
			iFindPosCurrent = k;
			int iStartPos = iFindPosLast;
			int iEndPos = iFindPosCurrent;
			if (iEndPos - iStartPos >= MAXTAGATRIBUTION_LENGTH)
			{
				delete charTemStringBuffer[iColNum];
				charTemStringBuffer[iColNum] = new char[iEndPos - iStartPos + 1];
			}
			memcpy(charTemStringBuffer[iColNum], fileBuffer + iStartPos, iEndPos - iStartPos);
			temStringBufferLength[iColNum] = iEndPos - iStartPos;
			charTemStringBuffer[iColNum][iEndPos - iStartPos] = 0;
			iFindPosLast = iFindPosCurrent + 1;
			iColNum++;
		}
	}

	int iMaxDay = 0;
	for(int cc = 0;cc<100;cc++)		
	{
		for(int dd = 0;dd<100;dd++)
		{
			pNeedModifyPointList[cc][dd];
			if(pNeedModifyPointList[cc][dd].iIndex_Day>0)
			{
				if(pNeedModifyPointList[cc][dd].iIndex_Day>iMaxDay)
				{
					iMaxDay = pNeedModifyPointList[cc][dd].iIndex_Day;
				}
				g_AllTagListInfo.push_back(pNeedModifyPointList[cc]+dd);
			}
		}
	}
	g_ProcessAllDayNum = iMaxDay;
	for(int cc = 0;cc<g_AllTagListInfo.size();cc++)		
	{

			g_AllTagListInfo[cc];
			char chDebug[1000] = {0};
			sprintf(chDebug,"gao0708 cc:%d DayIndex = %d TagName:%s-%s Middle:%f Offset:%f",cc,
				g_AllTagListInfo[cc]->iIndex_Day,g_AllTagListInfo[cc]->chNewPointName,g_AllTagListInfo[cc]->chOldPointName,
				g_AllTagListInfo[cc]->fMiddleValue,g_AllTagListInfo[cc]->fOffet);
			OutputDebugStringA(chDebug);
	}

	m_ListMacsPoint.SetItemCount(g_AllTagListInfo.size());

	m_ProcessAllDays = g_ProcessAllDayNum;
	m_ProcessAllPointNum = g_ProcessAllPointNum;
	m_StartProcesPointIndex = 1;
	m_StartProcessDayIndex = 1;
	UpdateData(FALSE);
}


void CTestInfluxDB_2Dlg::ProcessOneData(int &CurrentRowNum, int iAllColNum,int iAllRowNum, char ** charTemStringBuffer,NeedModifyPoints  **pNeedModifyPointList)
{

	//新的行
	CurrentRowNum++;

	//点序号索引 = (总行数-2)*(当前列数-2) + 当前行数-2
	int iPointIndex = -1;

	//iPointIndex = (iAllRowNum-2)*(CurrentColNum-2) + (CurrentRowNum-2);




	//第一行信息，原始点名
	if (CurrentRowNum == 1)
	{
		g_ProcessAllPointNum = iAllColNum-2;
		for (int ww = 2; ww < iAllColNum; ww++)
		{
			string strOldPointName = charTemStringBuffer[ww];
			iPointIndex = (ww-2);
			
			for(int zz = 0;zz<100;zz++)
			{
				//第iPointIndex个点的所有天的点名都是该自字段
				memcpy(pNeedModifyPointList[iPointIndex][zz].chOldPointName,strOldPointName.c_str(),strOldPointName.length());
			}
		}

	}
	//第一行信息，当前点名
	if (CurrentRowNum == 2)
	{
		for (int ww = 2; ww < iAllColNum; ww++)
		{
			string strNewPointName = charTemStringBuffer[ww];
			iPointIndex = (ww-2);			
			for(int zz = 0;zz<100;zz++)
			{
				memcpy(pNeedModifyPointList[iPointIndex][zz].chNewPointName,strNewPointName.c_str(),strNewPointName.length());
			}

		}
	}


	if (CurrentRowNum > 2)
	{
		if(CurrentRowNum == 3)
		{
			//起始时间
			string strTime = charTemStringBuffer[0];
			//解析年月日
			int Pos1 = strTime.find("/");			
			string strYear = strTime.substr(0,Pos1);
			strTime = strTime.substr(Pos1+1,strTime.length());
			int Pos2 = strTime.find("/");
			string strMonth = strTime.substr(0,Pos2);
			
			strTime = strTime.substr(Pos2+1,strTime.length());
			int Pos3 = strTime.find("/");	
			string strDay = strTime.substr(0,Pos3);

			int iYear = atoi(strYear.c_str());
			int iMonth = atoi(strMonth.c_str());
			int iDay = atoi(strDay.c_str());
			char chStartTime[255] = {0};
			sprintf(chStartTime,"%04d/%02d/%02d/00/00/00",iYear,iMonth,iDay);

			g_StartTime = FormatTime3(chStartTime);
		}
		int iIndexDay = -1;
		for (int ww = 1; ww < iAllColNum; ww++)
		{
			if(ww == 1)
			{
				iIndexDay = atoi(charTemStringBuffer[ww]);
			}
			else
			{
				string strUp_DownLimitValue = charTemStringBuffer[ww];
				int iPos = strUp_DownLimitValue.find("-");
				int iLength = strUp_DownLimitValue.length();
				string strMidValue = strUp_DownLimitValue.substr(0,iPos);
				string strOffsetValue = strUp_DownLimitValue.substr(iPos+1,iLength);

				iPointIndex = (ww-2) ;	

				if(strMidValue.empty() || strOffsetValue.empty())
				{
					pNeedModifyPointList[iPointIndex][CurrentRowNum-2].fMiddleValue = pNeedModifyPointList[iPointIndex][1].fMiddleValue ;
					pNeedModifyPointList[iPointIndex][CurrentRowNum-2].fOffet = pNeedModifyPointList[iPointIndex][1].fOffet;
				}
				else
				{
					pNeedModifyPointList[iPointIndex][CurrentRowNum-2].fMiddleValue = atof(strMidValue.c_str());
					pNeedModifyPointList[iPointIndex][CurrentRowNum-2].fOffet = atof(strOffsetValue.c_str());
				}


				pNeedModifyPointList[iPointIndex][CurrentRowNum-2].iIndex_Day = iIndexDay;//从1开始
				pNeedModifyPointList[iPointIndex][CurrentRowNum-2].iPointIndex = iPointIndex+1;//从1开始
			}

		}
	}



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

std::string TCHAR2STRING(TCHAR *STR)
{

 int iLen = WideCharToMultiByte(CP_ACP, 0,STR, -1, NULL, 0, NULL, NULL);

 char* chRtn =new char[iLen*sizeof(char)];

 WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);

std::string str(chRtn);

return str;

}
void STRING2TCHAR(string strData,wchar_t *pwszString,int bufferlength)
{

 int iLen = MultiByteToWideChar(CP_ACP, 0,strData.c_str(), -1, NULL, 0);

 //wchar_t *pwszString =new wchar_t[iLen];

 int iCopyLenth = bufferlength>iLen?iLen:bufferlength;
 MultiByteToWideChar(CP_ACP, NULL, strData.c_str(), -1, pwszString, iCopyLenth);

return ;
}

time_t FormatTime2(char * szTime)
{
	struct tm tm1;
	time_t time1;


	sscanf(szTime, "%4d-%2d-%2dT%2d:%2d:%2d.%03Z",
		&tm1.tm_year,
		&tm1.tm_mon,
		&tm1.tm_mday,
		&tm1.tm_hour,
		&tm1.tm_min,
		&tm1.tm_sec);

	tm1.tm_year -= 1900;
	tm1.tm_mon--;


	tm1.tm_isdst = -1;

	time1 = mktime(&tm1);
	return time1;
}
DWORD WINAPI Event_Maker22(int *iIndex)
{
	int iThisIndex = *iIndex;

	time_t starttime;
	time_t endtime;


	starttime = FormatTime2("20180602113000");

	starttime += (iThisIndex-1) * 60 * 60 * 24 * 30;
	endtime = starttime + 60 * 60 * 24 * 30;


	string strFilePath = "D:\\99se\\BOM.png";
	FILE *fp;
	fp = fopen(strFilePath.c_str(), "rb");
	if (fp != NULL)
	{
		fseek(fp, 0L, SEEK_END);
		int nFileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char *pBuffer = new char[nFileLen + 1];
		memset(pBuffer, 0, nFileLen);
		fread(pBuffer, nFileLen, 1, fp);
		fclose(fp);

		string mytring = Base64Encode(pBuffer, nFileLen);

		//mytring = "abcdefghijkl";

		int iTempLength = mytring.length();
		int iTemp1 = iTempLength / 2;

		string mytring_22temp = mytring.substr(0, iTemp1);
		string mytring_33temp = mytring.substr(iTemp1, iTempLength);

		string mytring_temp1 = "\"";
		string mytring_temp2 = "\"";

		mytring_temp1 += mytring_22temp;
		mytring_temp1 += "\"";

		mytring_temp2 += mytring_33temp;
		mytring_temp2 += "\"";

		string strUrlw = g_strWriteUrlq;//"http://localhost:8086/write?";
		string strDatasw;
		//strDatasw="cpu_load_short,host=server01 value=0.74 \n cpu_load_short,host=server02 value=0.84";
		string strResponse;
		int reqBufSize = 20000 * (9 + 8 + 32 + 1 + 6 + 20 + 10 + 1 + 20);
		char *reqBuf = new char[reqBufSize];
		int iOffset = 0;



		int iQuality = 0;
		int iValue = 0;
		int iNum = 0;
		//for(int i=0;i<60*60*24*31;i++)

		int iOffset_temp = endtime - starttime;

		int ALL_Num = 300;
		int iRet = 0;


		for (int i = 0; i < iOffset_temp; i++)
		{
			iQuality = 1;// rand() % 10;
			iValue = rand() % 100 + 1;
			starttime = time(NULL);

			sprintf(reqBuf + iOffset, "TagValue2,TagName=DDBTag1 value1=%s,value2=%s,value=1 %u000000000\n", mytring_temp1.c_str(), mytring_temp2.c_str(), (starttime) + 8 * 60 * 60);
			//sprintf(reqBuf + iOffset, "TagValue,TagName=DDBTag%d,Quality=%d value=%d %u000000000\n", j, 1, iValue, (starttime + i) + 8 * 60 * 60);

			strDatasw = reqBuf;
			//iRet = g_pClient->Write(strUrlw, "mydb1", strDatasw, strResponse);
			int aaa = 0;

			aaa = 1;
			Sleep(1000);
		}
	}

	return 1;
}
DWORD WINAPI Event_Maker(int *iIndex)
{

	int iThisIndex = *iIndex;

	// TODO: 在此添加控件通知处理程序代码
	//写数据
	string strUrlw = g_strWriteUrlq;//"http://localhost:8086/write?";
	string strDatasw;
	//strDatasw="cpu_load_short,host=server01 value=0.74 \n cpu_load_short,host=server02 value=0.84";
	string strResponse;
	int reqBufSize = 20000 * (9 + 8 + 32 + 1 + 6 + 20 + 10 + 1 + 20);
	char *reqBuf = new char[reqBufSize];
	int iOffset = 0;

	time_t starttime;	
	time_t endtime;
	

	starttime = FormatTime2("20190211000000");
	endtime = FormatTime2("20190301000000");


	int iQuality = 0;
	int iValue = 0;
	int iNum = 0;
	//for(int i=0;i<60*60*24*31;i++)

	int iOffset_temp = endtime - starttime;

	int ALL_Num = 300;
	string strData;
	for (int j = (iThisIndex-1)*ALL_Num +1; j <= (iThisIndex) * ALL_Num; j++)
	{

		iNum = 0;
		for (int i = 0; i < iOffset_temp; i++)
		{
			//		for (int j = 1; j < 2; j++)
			{
				//iQuality = 1;// rand() % 10;
				//iValue = rand() % 100 + 1;
				sprintf(reqBuf, "TagValue,TagName=DDBTag%d,Quality=%d value=%d %u000000000\n", j, 1, iValue,(starttime + i) + 8 * 60 * 60);
				strData += reqBuf;
				//iOffset = strlen(reqBuf);


			}
			iNum++;
			if (iNum == 10000 || i == iOffset_temp - 1)
			{
				strDatasw = reqBuf;
				int iTick1 = GetTickCount();
				int iRet ;//= g_pClient->Write(strUrlw, "mydb1", strDatasw, strResponse);

				char chDebug[255];
				sprintf(chDebug, "gao0704 iThisIndex:%d time:%d", iThisIndex, GetTickCount()- iTick1);
				OutputDebugStringA(chDebug);

				memset(reqBuf, 0, reqBufSize);
				iOffset = 0;
				iNum = 0;
			}
		}
	}

	return 1;
}


void CTestInfluxDB_2Dlg::OnBnClickedButton1()
{
	unsigned int dwThreadID;

	for (int i = 1; i < 10; i++)
	{
		int *p_index = new int;

		*p_index = i;

		HANDLE m_hThreadHandle = (HANDLE)::_beginthreadex(NULL, 0, (unsigned int(__stdcall *)(void *))Event_Maker, p_index, 0, &dwThreadID);

		Sleep(10000);
	}


	return;
	string strFilePath = "D:\\99se\\BOM.png";
	FILE *fp;
	fp = fopen(strFilePath.c_str(), "rb");
	if (fp != NULL)
	{
		fseek(fp, 0L, SEEK_END);
		int nFileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char *pBuffer = new char[nFileLen + 1];
		memset(pBuffer, 0, nFileLen);
		fread(pBuffer, nFileLen, 1, fp);
		fclose(fp);

		string mytring = Base64Encode(pBuffer, nFileLen);
		
		//mytring = "abcdefghijkl";

		int iTempLength = mytring.length();
		int iTemp1 = iTempLength / 2;

		string mytring_22temp = mytring.substr(0, iTemp1);
		string mytring_33temp = mytring.substr(iTemp1, iTempLength);

		string mytring_temp1 = "\"";
		string mytring_temp2 = "\"";

		mytring_temp1 += mytring_22temp;
		mytring_temp1 += "\"";

		mytring_temp2 += mytring_33temp;
		mytring_temp2 += "\"";

		string strUrlw = g_strWriteUrlq;//"http://localhost:8086/write?";
		string strDatasw;
		//strDatasw="cpu_load_short,host=server01 value=0.74 \n cpu_load_short,host=server02 value=0.84";
		string strResponse;
		int reqBufSize = 20000 * (9 + 8 + 32 + 1 + 6 + 20 + 10 + 1 + 20);
		char *reqBuf = new char[reqBufSize];
		int iOffset = 0;

		time_t starttime;
		time_t endtime;


		starttime = FormatTime2("20180402113000");
		endtime = FormatTime2("20190402114000");


		int iQuality = 0;
		int iValue = 0;
		int iNum = 0;
		//for(int i=0;i<60*60*24*31;i++)

		int iOffset_temp = endtime - starttime;

		int ALL_Num = 300;
		int iRet = 0;
		for (int i = 0; i < iOffset_temp; i++)
		{
			iQuality = 1;// rand() % 10;
			iValue = rand() % 100 + 1;
			sprintf(reqBuf + iOffset, "TagValue2,TagName=DDBTag%1 value1=%s,value2=%s,value=1 %u000000000\n", mytring_temp1.c_str(), mytring_temp2.c_str(), (starttime + i) + 8 * 60 * 60);
			//sprintf(reqBuf + iOffset, "TagValue,TagName=DDBTag%d,Quality=%d value=%d %u000000000\n", j, 1, iValue, (starttime + i) + 8 * 60 * 60);

			strDatasw = reqBuf;
			//iRet = g_pClient->Write(strUrlw, "mydb1", strDatasw, strResponse);
			int aaa = 0;
			aaa = 1;		
		}
	}


	

	for (int i = 1; i < 11; i++)
	{
		int *p_index = new int;

		*p_index = i;

		HANDLE m_hThreadHandle = (HANDLE)::_beginthreadex(NULL, 0, (unsigned int(__stdcall *)(void *))Event_Maker, p_index, 0, &dwThreadID);

	}


	//sprintf(reqBuf+iOffset,"TagValue,TagName=%s,Quality=%d value=%d  1434055562000000000\n","Tag0001",2,1);


	wchar_t wcharreturn[1000];
//	STRING2TCHAR(strResponse,wcharreturn,1000);
//	GetDlgItem(IDC_EDIT1)->SetWindowText( wcharreturn );
			

	int a=0;
	a = 2;
}
void CTestInfluxDB_2Dlg::ShowResultToList(int iIndex,PointValue *pPointValue,int iReadTagNum)
{
	TCHAR tchIndex[10] = {0};
	TCHAR tchQuality_MAX[50] = {0};
	TCHAR tchQuality_MIN[50] = {0};
	TCHAR tchQuality[50] = {0};


	if(iIndex == 0)
	{
		m_ListCtrl.DeleteAllItems();
		//遍历数组显示出所有像素点值

		for(int k = 0;k<iReadTagNum;k++)
		{
			m_ListCtrl.InsertItem( k, _T("") );
			wsprintf(tchIndex,_T("%d"),k+1);
			m_ListCtrl.SetItemText( k, 0, tchIndex);		
			m_ListCtrl.SetItemText( k, 1, (pPointValue+k)->tcTime);	


			TCHAR tcValue_MAX[64];
			TCHAR tcValue_MIN[64];
			TCHAR tcValue[64];

			char chTemp[50]={0};
			sprintf(chTemp,"%.4f",(pPointValue+k)->fValue_MAX);
			STRING2TCHAR(chTemp,tcValue_MAX,64);

		
			sprintf(chTemp,"%.4f",(pPointValue+k)->fValue_MIN);
			STRING2TCHAR(chTemp,tcValue_MIN,64);
			m_ListCtrl.SetItemText( k, 2, tcValue_MIN);	
			wsprintf(tchQuality_MIN,_T("%d"),(pPointValue+k)->iQuality_MIN);
			m_ListCtrl.SetItemText( k, 3, tchQuality_MIN);


			m_ListCtrl.SetItemText( k, 4, tcValue_MAX);	
			wsprintf(tchQuality_MAX,_T("%d"),(pPointValue+k)->iQuality_MAX);
			m_ListCtrl.SetItemText( k, 5, tchQuality_MAX);


			sprintf(chTemp,"%.4f",(pPointValue+k)->fValue);
			STRING2TCHAR(chTemp,tcValue,64);
			m_ListCtrl.SetItemText( k, 6, tcValue);	
			wsprintf(tchQuality,_T("%d"),(pPointValue+k)->iQuality);
			m_ListCtrl.SetItemText( k, 7, tchQuality);
			
		}
	}
	else if(iIndex == 1)
	{
		m_ListCtrl2.DeleteAllItems();
		//遍历数组显示出所有像素点值

		for(int k = 0;k<iReadTagNum;k++)
		{
			m_ListCtrl2.InsertItem( k, _T("") );
			wsprintf(tchIndex,_T("%d"),k+1);
			m_ListCtrl2.SetItemText( k, 0, tchIndex);		
			m_ListCtrl2.SetItemText( k, 1, (pPointValue+k)->tcTime);	


			TCHAR tcValue_MAX[64];
			TCHAR tcValue_MIN[64];
			TCHAR tcValue[64];

			char chTemp[50]={0};
			sprintf(chTemp,"%.4f",(pPointValue+k)->fValue_MAX);
			STRING2TCHAR(chTemp,tcValue_MAX,64);

		
			sprintf(chTemp,"%.4f",(pPointValue+k)->fValue_MIN);
			STRING2TCHAR(chTemp,tcValue_MIN,64);
			m_ListCtrl2.SetItemText( k, 2, tcValue_MIN);	
			wsprintf(tchQuality_MIN,_T("%d"),(pPointValue+k)->iQuality_MIN);
			m_ListCtrl2.SetItemText( k, 3, tchQuality_MIN);


			m_ListCtrl2.SetItemText( k, 4, tcValue_MAX);	
			wsprintf(tchQuality_MAX,_T("%d"),(pPointValue+k)->iQuality_MAX);
			m_ListCtrl2.SetItemText( k, 5, tchQuality_MAX);


			sprintf(chTemp,"%.4f",(pPointValue+k)->fValue);
			STRING2TCHAR(chTemp,tcValue,64);
			m_ListCtrl2.SetItemText( k, 6, tcValue);	
			wsprintf(tchQuality,_T("%d"),(pPointValue+k)->iQuality);
			m_ListCtrl2.SetItemText( k, 7, tchQuality);
			
		}
	}
	else if(iIndex == 1)
	{
		m_ListCtrl3.DeleteAllItems();
		//遍历数组显示出所有像素点值

		for(int k = 0;k<iReadTagNum;k++)
		{
			m_ListCtrl3.InsertItem( k, _T("") );
			wsprintf(tchIndex,_T("%d"),k+1);
			m_ListCtrl3.SetItemText( k, 0, tchIndex);		
			m_ListCtrl3.SetItemText( k, 1, (pPointValue+k)->tcTime);	


			TCHAR tcValue_MAX[64];
			TCHAR tcValue_MIN[64];
			TCHAR tcValue[64];

			char chTemp[50]={0};
			sprintf(chTemp,"%.4f",(pPointValue+k)->fValue_MAX);
			STRING2TCHAR(chTemp,tcValue_MAX,64);

		
			sprintf(chTemp,"%.4f",(pPointValue+k)->fValue_MIN);
			STRING2TCHAR(chTemp,tcValue_MIN,64);
			m_ListCtrl3.SetItemText( k, 2, tcValue_MIN);	
			wsprintf(tchQuality_MIN,_T("%d"),(pPointValue+k)->iQuality_MIN);
			m_ListCtrl3.SetItemText( k, 3, tchQuality_MIN);


			m_ListCtrl3.SetItemText( k, 4, tcValue_MAX);	
			wsprintf(tchQuality_MAX,_T("%d"),(pPointValue+k)->iQuality_MAX);
			m_ListCtrl3.SetItemText( k, 5, tchQuality_MAX);


			sprintf(chTemp,"%.4f",(pPointValue+k)->fValue);
			STRING2TCHAR(chTemp,tcValue,64);
			m_ListCtrl3.SetItemText( k, 6, tcValue);	
			wsprintf(tchQuality,_T("%d"),(pPointValue+k)->iQuality);
			m_ListCtrl3.SetItemText( k, 7, tchQuality);
			
		}
	}
}

void CTestInfluxDB_2Dlg::QuerySinglePointTrend(time_t starttime,time_t endtime,char chTagName[32],int pix,int iIndex)
{
	CTime cdate_start;
	CTime cdate_end;
	cdate_start = starttime;
	cdate_end = endtime;

	int interval = (endtime-starttime)/pix;
	//查询最大值、最小值，将其拼成一个值
	PointValue *pPointValue = NULL;
	PointValue *pPointValue_temp = NULL;
	pPointValue = new PointValue[pix];
	memset(pPointValue,0,pix*sizeof(PointValue));

	//查询最小值	
	//查询数据
	string strResponse1;
	string strResponse2;
	string strUrlq = g_strQueryUrlq;//"http://localhost:8086/query?";
	char chQuery_MIN[2000] = {0};
	char chQuery_MAX[2000] = {0};

	//add by gzh below
	//先从连续查询表中查询，若该表中记录不完整，则接着从当前表中继续查询，并将两个结果拼接起来
	//add by gzh above

	//fill(none)
	sprintf(chQuery_MIN,"SELECT MIN(min),quality_min FROM TagValue_CONTINUOUS where name_min ='%s'  AND time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
		chTagName,cdate_start.GetYear(),cdate_start.GetMonth(),cdate_start.GetDay(),cdate_start.GetHour(),cdate_start.GetMinute(),cdate_start.GetSecond(),
		cdate_end.GetYear(),cdate_end.GetMonth(),cdate_end.GetDay(),cdate_end.GetHour(),cdate_end.GetMinute(),cdate_end.GetSecond(),
		interval);
	sprintf(chQuery_MAX,"SELECT MAX(max),quality_max FROM TagValue_CONTINUOUS where name_max = '%s' AND time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
		chTagName,cdate_start.GetYear(),cdate_start.GetMonth(),cdate_start.GetDay(),cdate_start.GetHour(),cdate_start.GetMinute(),cdate_start.GetSecond(),
		cdate_end.GetYear(),cdate_end.GetMonth(),cdate_end.GetDay(),cdate_end.GetHour(),cdate_end.GetMinute(),cdate_end.GetSecond(),
		interval);

	int iRet1 = m_pClient->query(strUrlq,"mydb1",chQuery_MIN,strResponse1,1000);
	cout<<"iRet="<<",Response="<<strResponse1<<endl;

	int iRet2 = m_pClient->query(strUrlq,"mydb1",chQuery_MAX,strResponse2,1000);
	cout<<"iRet="<<",Response="<<strResponse2<<endl;

	//引入json解析
	Json::Reader reader_1;
	Json::Value root_1;		
	string sTagTime;
	float fTagValue = 0.0000;

	int iQuality = 0;
	string sQuality;

	int iReadTagNum = 0;
	int iReadTagNum_EX = 0;
	//add by gzh below
	//记录不存在记录的序号
	//最后一个有值的索引位置
	int iLastValueIndex = -1;//默认所有记录都在连续查询表中存在
	//add by gzh above

	
	//从字符串中读取数据
	if(reader_1.parse(strResponse1,root_1))
	{
		if (root_1["results"].isArray())
		{
			if(root_1["results"][0]["series"].isArray())
			{
				if(root_1["results"][0]["series"][0]["values"].isArray())
				{
					int nArraySize = root_1["results"][0]["series"][0]["values"].size();  
					
					iReadTagNum = nArraySize>pix?pix:nArraySize;
					//pPointValue = new PointValue[nArraySize];

					memset(pPointValue,0,iReadTagNum*sizeof(PointValue));
					for (int i=0; i<iReadTagNum; i++)
					{       
						if(root_1["results"][0]["series"][0]["values"][i].isArray())
						{

							CStringA strDebug;
							strDebug.Format("gao1015 %d \r\n", i);
							OutputDebugStringA(strDebug);

							sTagTime = root_1["results"][0]["series"][0]["values"][i][0].asCString();

							
							wchar_t tchTagTime[50]={0};
							STRING2TCHAR(sTagTime,tchTagTime,50);
							lstrcpy((pPointValue+i)->tcTime,tchTagTime);
							if(root_1["results"][0]["series"][0]["values"][i][1].isNull())
							{
								//int a = 0;
								//a = 20;
								
							}
							else
							{
								iLastValueIndex = i;
								fTagValue = root_1["results"][0]["series"][0]["values"][i][1].asFloat();
								//iQuality = root_1["results"][0]["series"][0]["values"][i][2].asInt();

								sQuality = root_1["results"][0]["series"][0]["values"][i][2].asCString();
								iQuality = atoi(sQuality.c_str());

								(pPointValue+i)->iQuality_MIN = iQuality;

								(pPointValue+i)->fValue_MIN = fTagValue;
								//char chTemp[50]={0};
								//sprintf(chTemp,"%.4f",fTagValue);

								//STRING2TCHAR(chTemp,(pPointValue+i)->tcValue_MIN,64);
																
																								
							}
						}
												
					}
					//int a = 0;
					//a = 20;
				}
				
			}
		}
	}


	//查询最大值	

	//引入json解析
	//从字符串中读取数据
	if(reader_1.parse(strResponse2,root_1))
	{
		if (root_1["results"].isArray())
		{
			if(root_1["results"][0]["series"].isArray())
			{
				if(root_1["results"][0]["series"][0]["values"].isArray())
				{
					int nArraySize = root_1["results"][0]["series"][0]["values"].size();  
					for (int i=0; i<nArraySize; i++)
					{       
						if(root_1["results"][0]["series"][0]["values"][i].isArray())
						{
							sTagTime = root_1["results"][0]["series"][0]["values"][i][0].asCString();

							//pPointValue = iQuality;
							wchar_t tchTagTime[50]={0};
							STRING2TCHAR(sTagTime,tchTagTime,50);

							if(root_1["results"][0]["series"][0]["values"][i][1].isNull())
							{
								//int a = 0;
								//a = 20;								
							}
							else
							{		
								fTagValue = root_1["results"][0]["series"][0]["values"][i][1].asFloat();
								sQuality = root_1["results"][0]["series"][0]["values"][i][2].asCString();
								//iQuality = root_1["results"][0]["series"][0]["values"][i][2].asInt();
								iQuality = atoi(sQuality.c_str());
								if(lstrcmp(tchTagTime,(pPointValue+i)->tcTime)==0)
								{
									(pPointValue+i)->iQuality_MAX = iQuality;
								
									(pPointValue+i)->fValue_MAX = fTagValue;
									//char chTemp[50]={0};
									//sprintf(chTemp,"%.4f",fTagValue);									
									//STRING2TCHAR(chTemp,(pPointValue+i)->tcValue_MAX,64);
									
									/*char *tcTemp;
									int decimal, sign;
									tcTemp = _fcvt( fTagValue, 50, &decimal, &sign );
									STRING2TCHAR(tcTemp,(pPointValue+i)->tcValue_MAX,64);*/

									//wsprintf((pPointValue+i)->tcValue_MAX,_T("%f"),fTagValue);
									//lstrcpy((pPointValue+i)->tcTime,tchTagTime);			
								}																					
							}

						}
												
					}
					int a = 0;
					a = 20;
				}
				
			}
		}
	}


	//若在连续查询没有查询完全
	if(iLastValueIndex < pix-1)
	{
		//需要从 iNoValueIndex位置继续往后查询
		pPointValue_temp = pPointValue+(iLastValueIndex+1);
		time_t starttime_temp;
		starttime_temp = starttime+(iLastValueIndex+1)*interval;

		//查询最小值	
		//查询数据
		string strResponse1_temp;
		string strResponse2_temp;

		char chQuery_MIN_temp[2000] = {0};
		char chQuery_MAX_temp[2000] = {0};

		CTime cdate_start_temp;
		cdate_start_temp = starttime_temp;
		//add by gzh below
		//先从连续查询表中查询，若该表中记录不完整，则接着从当前表中继续查询，并将两个结果拼接起来
		//add by gzh above

		//fill(none)
		sprintf(chQuery_MIN_temp,"SELECT MIN(value),Quality FROM TagValue where TagName ='%s'  AND time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
			chTagName,cdate_start_temp.GetYear(),cdate_start_temp.GetMonth(),cdate_start_temp.GetDay(),cdate_start_temp.GetHour(),cdate_start_temp.GetMinute(),cdate_start_temp.GetSecond(),
			cdate_end.GetYear(),cdate_end.GetMonth(),cdate_end.GetDay(),cdate_end.GetHour(),cdate_end.GetMinute(),cdate_end.GetSecond(),
			interval);
		sprintf(chQuery_MAX_temp,"SELECT MAX(value),Quality FROM TagValue where TagName = '%s' AND time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
			chTagName,cdate_start_temp.GetYear(),cdate_start_temp.GetMonth(),cdate_start_temp.GetDay(),cdate_start_temp.GetHour(),cdate_start_temp.GetMinute(),cdate_start_temp.GetSecond(),
			cdate_end.GetYear(),cdate_end.GetMonth(),cdate_end.GetDay(),cdate_end.GetHour(),cdate_end.GetMinute(),cdate_end.GetSecond(),
			interval);

		int iRet1_temp = m_pClient->query(strUrlq,"mydb1",chQuery_MIN_temp,strResponse1_temp,1000);
		cout<<"iRet="<<",Response="<< strResponse1_temp <<endl;

		int iRet2_temp = m_pClient->query(strUrlq,"mydb1",chQuery_MAX_temp,strResponse2_temp,1000);
		cout<<"iRet="<<",Response="<< strResponse2_temp <<endl;


		//从字符串中读取数据
		if(reader_1.parse(strResponse1_temp,root_1))
		{
			if (root_1["results"].isArray())
			{
				if(root_1["results"][0]["series"].isArray())
				{
					if(root_1["results"][0]["series"][0]["values"].isArray())
					{
						int nArraySize = root_1["results"][0]["series"][0]["values"].size();  											
						iReadTagNum_EX = nArraySize > (pix - iLastValueIndex - 1) ? (pix - iLastValueIndex - 1) : nArraySize;
						
						memset(pPointValue,0,nArraySize*sizeof(PointValue));
						for (int i=0; i<iReadTagNum_EX; i++)
						{       
							if(root_1["results"][0]["series"][0]["values"][i].isArray())
							{

								CStringA strDebug;
								strDebug.Format("gao1015 %d \r\n", i);
								OutputDebugStringA(strDebug);

								sTagTime = root_1["results"][0]["series"][0]["values"][i][0].asCString();

							
								wchar_t tchTagTime[50]={0};
								STRING2TCHAR(sTagTime,tchTagTime,50);
								lstrcpy((pPointValue_temp+i)->tcTime,tchTagTime);
								if(root_1["results"][0]["series"][0]["values"][i][1].isNull())
								{
									//int a = 0;
									//a = 20;
								}
								else
								{

									fTagValue = root_1["results"][0]["series"][0]["values"][i][1].asFloat();
									//iQuality = root_1["results"][0]["series"][0]["values"][i][2].asInt();

									sQuality = root_1["results"][0]["series"][0]["values"][i][2].asCString();
									iQuality = atoi(sQuality.c_str());

									(pPointValue_temp+i)->iQuality_MIN = iQuality;

									(pPointValue_temp+i)->fValue_MIN = fTagValue;
									//char chTemp[50]={0};
									//sprintf(chTemp,"%.4f",fTagValue);

									//STRING2TCHAR(chTemp,(pPointValue+i)->tcValue_MIN,64);
																
																								
								}
							}
												
						}
						//int a = 0;
						//a = 20;
					}
				
				}
			}
		}


		//查询最大值	

		//引入json解析
		//从字符串中读取数据
		if(reader_1.parse(strResponse2_temp,root_1))
		{
			if (root_1["results"].isArray())
			{
				if(root_1["results"][0]["series"].isArray())
				{
					if(root_1["results"][0]["series"][0]["values"].isArray())
					{
						int nArraySize = root_1["results"][0]["series"][0]["values"].size();  
						iReadTagNum = nArraySize > (pix - iLastValueIndex - 1) ? (pix - iLastValueIndex - 1) : nArraySize;
						for (int i=0; i<iReadTagNum; i++)
						{       
							if(root_1["results"][0]["series"][0]["values"][i].isArray())
							{
								sTagTime = root_1["results"][0]["series"][0]["values"][i][0].asCString();

								//pPointValue = iQuality;
								wchar_t tchTagTime[50]={0};
								STRING2TCHAR(sTagTime,tchTagTime,50);

								if(root_1["results"][0]["series"][0]["values"][i][1].isNull())
								{
									//int a = 0;
									//a = 20;								
								}
								else
								{		
									fTagValue = root_1["results"][0]["series"][0]["values"][i][1].asFloat();
									sQuality = root_1["results"][0]["series"][0]["values"][i][2].asCString();
									//iQuality = root_1["results"][0]["series"][0]["values"][i][2].asInt();
									iQuality = atoi(sQuality.c_str());
									if(lstrcmp(tchTagTime,(pPointValue_temp+i)->tcTime)==0)
									{
										(pPointValue_temp+i)->iQuality_MAX = iQuality;
								
										(pPointValue_temp+i)->fValue_MAX = fTagValue;
										//char chTemp[50]={0};
										//sprintf(chTemp,"%.4f",fTagValue);									
										//STRING2TCHAR(chTemp,(pPointValue+i)->tcValue_MAX,64);
									
										/*char *tcTemp;
										int decimal, sign;
										tcTemp = _fcvt( fTagValue, 50, &decimal, &sign );
										STRING2TCHAR(tcTemp,(pPointValue+i)->tcValue_MAX,64);*/

										//wsprintf((pPointValue+i)->tcValue_MAX,_T("%f"),fTagValue);
										//lstrcpy((pPointValue+i)->tcTime,tchTagTime);			
									}																					
								}

							}
												
						}
						int a = 0;
						a = 20;
					}
				
				}
			}
		}

	}


	//查询一次整个查询间隔内的均值
	char chQuery_mean_temp[2000] = { 0 };
	double dTagValue_meann = 0.0000;


	string strResponse1_mean_temp;
	//fill(none)
	sprintf(chQuery_mean_temp, "SELECT mean(value) FROM TagValue where TagName ='%s'  AND time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
		chTagName, cdate_start.GetYear(), cdate_start.GetMonth(), cdate_start.GetDay(), cdate_start.GetHour(), cdate_start.GetMinute(), cdate_start.GetSecond(),
		cdate_end.GetYear(), cdate_end.GetMonth(), cdate_end.GetDay(), cdate_end.GetHour(), cdate_end.GetMinute(), cdate_end.GetSecond(),
		endtime - starttime);


	int iRet1_mean = m_pClient->query(strUrlq, "mydb1", chQuery_mean_temp, strResponse1_mean_temp, 1000);
	cout << "iRet=" << ",Response=" << strResponse1_mean_temp << endl;


	//引入json解析

	//从字符串中读取数据
	if (reader_1.parse(strResponse1_mean_temp, root_1))
	{
		if (root_1["results"].isArray())
		{
			if (root_1["results"][0]["series"].isArray())
			{
				if (root_1["results"][0]["series"][0]["values"].isArray())
				{
					int nArraySize = root_1["results"][0]["series"][0]["values"].size();

					iReadTagNum = nArraySize > pix ? pix : nArraySize;
					//pPointValue = new PointValue[nArraySize];

					memset(pPointValue, 0, iReadTagNum * sizeof(PointValue));
					for (int i = 0; i < 1; i++)
					{
						if (root_1["results"][0]["series"][0]["values"][i].isArray())
						{

							CStringA strDebug;
							strDebug.Format("gao1015 %d \r\n", i);
							OutputDebugStringA(strDebug);
							if (root_1["results"][0]["series"][0]["values"][i][1].isNull())
							{

							}
							else
							{
								dTagValue_meann = root_1["results"][0]["series"][0]["values"][i][1].asFloat();
							}
						}

					}
					//int a = 0;
					//a = 20;
				}

			}
		}
	}





	TCHAR tchIndex[10] = {0};
	TCHAR tchQuality_MAX[50] = {0};
	TCHAR tchQuality_MIN[50] = {0};
	TCHAR tchQuality[50] = {0};
	if(iLastValueIndex < pix-1)
	{
		iReadTagNum = iLastValueIndex+1+iReadTagNum_EX;
	}
	else
	{
		iReadTagNum = pix;
	}

	//int *iMinIndex = new int[iReadTagNum];
	//int *iMaxIndex = new int[iReadTagNum];

	//int iMinIndex[1280];
	//int iMaxIndex[1280];

	//double *dMinValue = new double[pix];
	//double *dMaxValue = new double[pix];

	////将坏点剔除
	//int iRealValueNum = 0;
	//for(int m=0;m<iReadTagNum;m++)
	//{
	//	iMinIndex[m]=0;
	//	iMaxIndex[m]=0;
	//}
	//for(int m=0;m<iReadTagNum;m++)
	//{
	//	if(0 != lstrcmp((pPointValue+m)->tcTime,_T("")))
	//	{
	//		iMinIndex[iRealValueNum]=m;
	//		iMaxIndex[iRealValueNum]=m;
	//		dMinValue[iRealValueNum]=(pPointValue+m)->fValue_MIN;
	//		dMaxValue[iRealValueNum]=(pPointValue+m)->fValue_MAX;
	//		iRealValueNum++;
	//	}
	//}
	//int iMinNo = 0;
	//int iMaxNo = 0;


	//double dMidValue = 0;
	//for(int k = 0;k<iRealValueNum-1;k++)
	//{
	//	//将所有的最大值和最小值排序
	//	//依次取最小的和最大的，重复的则跳过取下一个，直到遍历完所有序号
	//	for(int j=k+1;j>0;j--)
	//	{
	//		 if(dMinValue[j]<dMinValue[j-1])
	//		 {
	//			 iMinNo = iMinIndex[j];
	//			 iMinIndex[j] = iMinIndex[j-1];
	//			 iMinIndex[j-1] = iMinNo;

	//			 dMidValue = dMinValue[j];
	//			 dMinValue[j] = dMinValue[j-1];
	//			 dMinValue[j-1] = dMidValue;
	//		 }
	//		 else
	//		 {
	//			break;
	//		 }
	//	}	
	//}


	////将所有的最大值和最小值排序
	////依次取最小的和最大的，重复的则跳过取下一个，直到遍历完所有序号
	//for(int k=0;k<iRealValueNum-1;k++)
	//{
	//	for(int j=k+1;j>0;j--)
	//	{
	//		if(dMaxValue[j]>dMaxValue[j-1])
	//		{ 
	//			//继续往前比较
	//			iMaxNo = iMaxIndex[j];
	//			iMaxIndex[j] = iMaxIndex[j-1];
	//			iMaxIndex[j-1] = iMaxNo;

	//			 dMidValue = dMaxValue[j];
	//			 dMaxValue[j] = dMaxValue[j-1];
	//			 dMaxValue[j-1] = dMidValue;
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}
	//}	


	//map<int,int> indexmap;
	//for(int k = 0;k<iRealValueNum;k++)
	//{
	//	map<int,int>::iterator it;
	//	it = indexmap.find(iMinIndex[k]);
	//	if(it==indexmap.end())
	//	{
	//		(pPointValue+iMinIndex[k])->fValue = (pPointValue+iMinIndex[k])->fValue_MIN;
	//		(pPointValue+iMinIndex[k])->iQuality = (pPointValue+iMinIndex[k])->iQuality_MIN;
	//		indexmap[iMinIndex[k]] = k;
	//	}
	//	it = indexmap.find(iMaxIndex[k]);
	//	if(it==indexmap.end())
	//	{
	//		(pPointValue+iMaxIndex[k])->fValue = (pPointValue+iMaxIndex[k])->fValue_MAX;
	//		(pPointValue+iMaxIndex[k])->iQuality = (pPointValue+iMaxIndex[k])->iQuality_MAX;
	//		indexmap[iMaxIndex[k]] = k;
	//	}

	//}

	for(int k = 0;k< iReadTagNum;k++)
	{
		double sub_min = fabs(dTagValue_meann - (pPointValue + k)->fValue_MIN);
		double sub_max = fabs(dTagValue_meann - (pPointValue + k)->fValue_MIN);

		if(sub_min> sub_max)
		{
			(pPointValue+k)->fValue = (pPointValue+k)->fValue_MIN;
			(pPointValue+k)->iQuality = (pPointValue+k)->iQuality_MIN;
		}
		else
		{
			(pPointValue + k)->fValue = (pPointValue + k)->fValue_MAX;
			(pPointValue + k)->iQuality = (pPointValue + k)->iQuality_MAX;
		}
	}

	ShowResultToList(iIndex,pPointValue,iReadTagNum);
	

}


void CTestInfluxDB_2Dlg::QuerySinglePointTrendEX(time_t starttime,time_t endtime,char chTagName[64],int pix,int iIndex,float fMD,float fMU,char chTagName_temp[64])
{


			char chDebug[500]={0};

			sprintf(chDebug,"gao0510 starttime:%u endtime:%u m_chTagName: start 111111",starttime,endtime);
			OutputDebugStringA(chDebug);

	CTime cdate_start;
	CTime cdate_end;
	cdate_start = starttime;
	cdate_end = endtime;

	int interval = (endtime-starttime)/pix;
	//查询最大值、最小值，将其拼成一个值
	PointValue_EX *pPointValue = NULL;
	pPointValue = new PointValue_EX[pix];
	memset(pPointValue,0,pix*sizeof(PointValue_EX));

	//查询最小值	
	//查询数据
	string strResponse1;
	string strResponse2;
	string strUrlq = g_strQueryUrlq;//"http://localhost:8086/query?";
	char chQuery_MIN[2000] = {0};
	char chQuery_MAX[2000] = {0};

	//add by gzh below
	//先从连续查询表中查询，若该表中记录不完整，则接着从当前表中继续查询，并将两个结果拼接起来
	//add by gzh above

	//fill(none)
	sprintf(chQuery_MIN,"SELECT * FROM TagValue where TagName ='%s'  AND time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'",
		chTagName,cdate_start.GetYear(),cdate_start.GetMonth(),cdate_start.GetDay(),cdate_start.GetHour(),cdate_start.GetMinute(),cdate_start.GetSecond(),
		cdate_end.GetYear(),cdate_end.GetMonth(),cdate_end.GetDay(),cdate_end.GetHour(),cdate_end.GetMinute(),cdate_end.GetSecond());
	//sprintf(chQuery_MAX,"SELECT MAX(max),quality_max FROM TagValue_CONTINUOUS where name_max = '%s' AND time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
	//	chTagName,cdate_start.GetYear(),cdate_start.GetMonth(),cdate_start.GetDay(),cdate_start.GetHour(),cdate_start.GetMinute(),cdate_start.GetSecond(),
	//	cdate_end.GetYear(),cdate_end.GetMonth(),cdate_end.GetDay(),cdate_end.GetHour(),cdate_end.GetMinute(),cdate_end.GetSecond(),
	//	interval);



			sprintf(chDebug,"gao0510 22222");
			OutputDebugStringA(chDebug);



	int iRet1 = m_pClient->query(strUrlq,"mydb1",chQuery_MIN,strResponse1,1000);
	cout<<"iRet="<<",Response="<<strResponse1<<endl;

	
				sprintf(chDebug,"gao0510 333333");
			OutputDebugStringA(chDebug);


	//memcpy(chDebug,strResponse1.c_str(),min(200,strResponse1.length()));
	////sprintf(chDebug,"gao0510 m_chTagName: iRet1:%d 22222",iRet1);
	//
	//OutputDebugStringA(chDebug);


	//int iRet2 = m_pClient->query(strUrlq,"mydb1",chQuery_MAX,strResponse2,1000);
	//cout<<"iRet="<<",Response="<<strResponse2<<endl;

	//引入json解析
	Json::Reader reader_1;
	Json::Value root_1;		
	string sTagTime;
	float fTagValue = 0.0000;

	int iQuality = 0;
	string sQuality;

	int iReadTagNum = 0;
	int iReadTagNum_EX = 0;
	//add by gzh below
	//记录不存在记录的序号
	//最后一个有值的索引位置
	int iLastValueIndex = -1;//默认所有记录都在连续查询表中存在
	//add by gzh above

	
	//从字符串中读取数据
	if(reader_1.parse(strResponse1,root_1))
	{

		if (root_1["results"].isArray())
		{
			if(root_1["results"][0]["series"].isArray())
			{



				if(root_1["results"][0]["series"][0]["values"].isArray())
				{




					int nArraySize = root_1["results"][0]["series"][0]["values"].size();  
					
					iReadTagNum = nArraySize>pix?pix:nArraySize;
					//pPointValue = new PointValue[nArraySize];

					memset(pPointValue,0,iReadTagNum*sizeof(PointValue_EX));
					for (int i=0; i<iReadTagNum; i++)
					{       
						if(root_1["results"][0]["series"][0]["values"][i].isArray())
						{

							//CStringA strDebug;
							//strDebug.Format("gao1015 %d \r\n", i);
							//OutputDebugStringA(strDebug);



							sTagTime = root_1["results"][0]["series"][0]["values"][i][0].asCString();

							
							//wchar_t tchTagTime[50]={0};
							//STRING2TCHAR(sTagTime,tchTagTime,50);


							(pPointValue+i)->valueTime = FormatTime2(( char*)(sTagTime.c_str()));

							//lstrcpy((pPointValue+i)->tcTime,tchTagTime);
							if(root_1["results"][0]["series"][0]["values"][i][1].isNull())
							{
								//int a = 0;
								//a = 20;
								
							}
							else
							{
								iLastValueIndex = i;
								fTagValue = root_1["results"][0]["series"][0]["values"][i][3].asFloat();
								iQuality = root_1["results"][0]["series"][0]["values"][i][1].asInt();



								if(fTagValue>fMU)
								{
									fTagValue = fMU;
								}
								else if(fTagValue<fMD)
								{
									fTagValue = fMD;
								}
								//sQuality = root_1["results"][0]["series"][0]["values"][i][2].asCString();
								//iQuality = atoi(sQuality.c_str());

								(pPointValue+i)->iQuality_MIN = iQuality;

								(pPointValue+i)->fValue_MIN = fTagValue;
								//char chTemp[50]={0};
								//sprintf(chTemp,"%.4f",fTagValue);

								//STRING2TCHAR(chTemp,(pPointValue+i)->tcValue_MIN,64);
																
																								
							}
						}
												
					}
					//int a = 0;
					//a = 20;
				}
				
			}
		}
	}



					sprintf(chDebug,"gao0510 m_chTagName  iReadTagNum:%d 3333",iReadTagNum);
			OutputDebugStringA(chDebug);

	//写数据
	string strUrlw = g_strWriteUrlq;//"http://localhost:8086/write?";
	string strDatasw;
	//strDatasw="cpu_load_short,host=server01 value=0.74 \n cpu_load_short,host=server02 value=0.84";
	string strResponse;
	int reqBufSize = 20000 ;
	char *reqBuf = new char[reqBufSize];
	int iOffset = 0;




	int iNum = 0;
	//for(int i=0;i<60*60*24*31;i++)

	int iOffset_temp = endtime - starttime;

	int ALL_Num = 300;
	string strData;
	for (int j =0; j < iReadTagNum; j++)
	{

		//		for (int j = 1; j < 2; j++)
		{
			sprintf(reqBuf , "TagValue,TagName=%s Quality=%d,value=%f %u000000000\n", chTagName_temp, (pPointValue+j)->iQuality_MIN,(pPointValue+j)->fValue_MIN ,(pPointValue+j)->valueTime + 8 * 60 * 60);
			strData += reqBuf;


		}

		if(g_bExit)
		{
			//软件退出
			return;
		}
		iNum++;
		if (iNum == 10000 || j == (iReadTagNum-1))
		{
			//strDatasw = reqBuf;
			int iTick1 = GetTickCount();
			int iRet = g_pClient->Write(strUrlw, "mydb1", strData, strResponse);


			sprintf(chDebug,"gao0510 m_chTagName  iRet:%d 555  strResponse:%s",iRet,strResponse.c_str());
			OutputDebugStringA(chDebug);

			memset(reqBuf, 0, reqBufSize);
			iOffset = 0;
			iNum = 0;
			strData = "";
		}
		
	}

	if(pPointValue)
	{
		delete pPointValue;
		pPointValue = NULL;
	}
	if(reqBuf)
	{
		delete reqBuf;
		reqBuf = NULL;
	}

}








void CTestInfluxDB_2Dlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	
	UpdateData(TRUE);
	 CTime cdate_start;
	 time_t starttime;
	( (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER1) ) -> GetTime(cdate_start);    //得到控件
	starttime = cdate_start.GetTime();

	 CTime cdate_end;
	 time_t endtime;
	( (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER2) ) -> GetTime(cdate_end);    //得到控件
	endtime = cdate_end.GetTime();

	int timeOut = 1000;

	int iQueryTagNum = 1;
	//开辟线程进行分点查询
	//总共开辟100个线程 首先找到空闲的线程
	//若找不到则自己处理
	int *pIndex = new int[iQueryTagNum];

	int iFoudThreadNum = 0;
	for(int k =0;k<MAXIMUM_WAIT_OBJECTS;k++)
	{
		if(m_QueryInfluxdbThread[k].m_iState == STATE_IDLE)
		{
			pIndex[iFoudThreadNum] = k;
			m_QueryInfluxdbThread[k].m_iState = STATE_REBUILDING;
			m_QueryInfluxdbThread[k].m_starttime = starttime;
			m_QueryInfluxdbThread[k].m_endtime = endtime;
			m_QueryInfluxdbThread[k].m_pix = m_pix;
			//m_QueryInfluxdbThread[k].m_pInfluxDB_2Dlg = this;
			m_QueryInfluxdbThread[k].m_iIndex = iFoudThreadNum;

			char chTagName[32]={0};
			sprintf(chTagName,"Tag000%d",iFoudThreadNum+1);
			memcpy(m_QueryInfluxdbThread[k].m_chTagName,chTagName,32);
			iFoudThreadNum++;
			if(iFoudThreadNum>=iQueryTagNum)
			{
				break;
			}
		}		
	}

	DWORD waitResult=0;
	if(iFoudThreadNum<iQueryTagNum)
	{
		//找不到足够多的空闲线程则自身查询
		
	}
	else 
	{
		//启动线程
		for(int i=0;i<iFoudThreadNum;i++)
		{
			int index = pIndex[i];
			//输入查询的时间段 标签名 像素点信息
			m_QueryInfluxdbThread[index].m_hReadEventOK.event_reset();


			m_QueryInfluxdbThread[index].m_hReadEvent.event_set();
		}
		if(timeOut!=0)
		{
			CLightEvent hEv[MAXIMUM_WAIT_OBJECTS];

			if(iFoudThreadNum>MAXIMUM_WAIT_OBJECTS)
			{
				//AfxMessageBox(_T("too much DDB, modify your configuration please![数据过于分散，来自超过64个DDB，请修改组态]"));
				OutputDebugString(_T("too much DDB, modify your configuration please![数据过于分散，来自超过64个DDB，请修改组态]"));
				return ;
			}
			for(int i=0;i<iFoudThreadNum;i++)
			{
				int index = pIndex[i];
				hEv[i] = m_QueryInfluxdbThread[index].m_hReadEventOK;
			}
			waitResult = WaitForMultipleEventsComm(iFoudThreadNum, hEv, TRUE, timeOut);//第3参数应该是true，等待所有都ok，在分析，
			if (waitResult == WAIT_TIMEOUT) 
			{
			}	
		}
	}

	if (waitResult == WAIT_TIMEOUT) 
		return ;
	else
		return ;


	/*
	//根据查询的起始时间和结束时间、像素点计算出查询间隔。

	//m_Datatime1.SetFormat( L"yyyy-MM-dd   HH:mm:ss" );

	int interval = (endtime-starttime)/m_pix;


	//查询最大值、最小值，将其拼成一个值
	PointValue *pPointValue = NULL;
	
	//查询最小值	
	//查询数据
	string strResponse1;
	string strResponse2;
	string strUrlq = "http://localhost:8086/query?";
	char chQuery_MIN[2000] = {0};
	char chQuery_MAX[2000] = {0};
	//fill(none)
	sprintf(chQuery_MIN,"SELECT MIN(value),Quality FROM %s  where time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
		"Tag0001",cdate_start.GetYear(),cdate_start.GetMonth(),cdate_start.GetDay(),cdate_start.GetHour(),cdate_start.GetMinute(),cdate_start.GetSecond(),
		cdate_end.GetYear(),cdate_end.GetMonth(),cdate_end.GetDay(),cdate_end.GetHour(),cdate_end.GetMinute(),cdate_end.GetSecond(),
		interval);
	sprintf(chQuery_MAX,"SELECT MAX(value),Quality FROM %s  where time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
		"Tag0001",cdate_start.GetYear(),cdate_start.GetMonth(),cdate_start.GetDay(),cdate_start.GetHour(),cdate_start.GetMinute(),cdate_start.GetSecond(),
		cdate_end.GetYear(),cdate_end.GetMonth(),cdate_end.GetDay(),cdate_end.GetHour(),cdate_end.GetMinute(),cdate_end.GetSecond(),
		interval);

	int iRet1 = m_pClient->query(strUrlq,"mydb1",chQuery_MIN,strResponse1,1000);
	cout<<"iRet="<<",Response="<<strResponse1<<endl;

	int iRet2 = m_pClient->query(strUrlq,"mydb1",chQuery_MAX,strResponse2,1000);
	cout<<"iRet="<<",Response="<<strResponse2<<endl;

	//引入json解析
	Json::Reader reader_1;
	Json::Value root_1;		
	string sTagTime;
	float fTagValue = 0.0000;

	int iQuality = 0;
	string sQuality;

	int iReadTagNum = 0;

	//从字符串中读取数据
	if(reader_1.parse(strResponse1,root_1))
	{
		if (root_1["results"].isArray())
		{
			if(root_1["results"][0]["series"].isArray())
			{
				if(root_1["results"][0]["series"][0]["values"].isArray())
				{
					int nArraySize = root_1["results"][0]["series"][0]["values"].size();  
					
					iReadTagNum = nArraySize;
					pPointValue = new PointValue[nArraySize];
					memset(pPointValue,0,nArraySize*sizeof(PointValue));
					for (int i=0; i<nArraySize-1; i++)
					{       
						if(root_1["results"][0]["series"][0]["values"][i].isArray())
						{

							CStringA strDebug;
							strDebug.Format("gao1015 %d \r\n", i);
							OutputDebugStringA(strDebug);

							sTagTime = root_1["results"][0]["series"][0]["values"][i][0].asCString();

							
							wchar_t tchTagTime[50]={0};
							STRING2TCHAR(sTagTime,tchTagTime,50);
							lstrcpy((pPointValue+i)->tcTime,tchTagTime);
							if(root_1["results"][0]["series"][0]["values"][i][1].isNull())
							{
								//int a = 0;
								//a = 20;
							}
							else
							{

								fTagValue = root_1["results"][0]["series"][0]["values"][i][1].asFloat();
								//iQuality = root_1["results"][0]["series"][0]["values"][i][2].asInt();

								sQuality = root_1["results"][0]["series"][0]["values"][i][2].asCString();
								iQuality = atoi(sQuality.c_str());

								(pPointValue+i)->iQuality_MIN = iQuality;

								(pPointValue+i)->fValue_MIN = fTagValue;
								//char chTemp[50]={0};
								//sprintf(chTemp,"%.4f",fTagValue);

								//STRING2TCHAR(chTemp,(pPointValue+i)->tcValue_MIN,64);
																
																								
							}
						}
												
					}
					//int a = 0;
					//a = 20;
				}
				
			}
		}
	}


	//查询最大值	


	//引入json解析
	//从字符串中读取数据
	if(reader_1.parse(strResponse2,root_1))
	{
		if (root_1["results"].isArray())
		{
			if(root_1["results"][0]["series"].isArray())
			{
				if(root_1["results"][0]["series"][0]["values"].isArray())
				{
					int nArraySize = root_1["results"][0]["series"][0]["values"].size();  
					for (int i=0; i<nArraySize; i++)
					{       
						if(root_1["results"][0]["series"][0]["values"][i].isArray())
						{
							sTagTime = root_1["results"][0]["series"][0]["values"][i][0].asCString();

							//pPointValue = iQuality;
							wchar_t tchTagTime[50]={0};
							STRING2TCHAR(sTagTime,tchTagTime,50);

							if(root_1["results"][0]["series"][0]["values"][i][1].isNull())
							{
								//int a = 0;
								//a = 20;								
							}
							else
							{		
								fTagValue = root_1["results"][0]["series"][0]["values"][i][1].asFloat();
								sQuality = root_1["results"][0]["series"][0]["values"][i][2].asCString();
								//iQuality = root_1["results"][0]["series"][0]["values"][i][2].asInt();
								iQuality = atoi(sQuality.c_str());
								if(lstrcmp(tchTagTime,(pPointValue+i)->tcTime)==0)
								{
									(pPointValue+i)->iQuality_MAX = iQuality;
								
									(pPointValue+i)->fValue_MAX = fTagValue;
									//char chTemp[50]={0};
									//sprintf(chTemp,"%.4f",fTagValue);									
									//STRING2TCHAR(chTemp,(pPointValue+i)->tcValue_MAX,64);
									
									//char *tcTemp;
									//int decimal, sign;
									//tcTemp = _fcvt( fTagValue, 50, &decimal, &sign );
									//STRING2TCHAR(tcTemp,(pPointValue+i)->tcValue_MAX,64);

									//wsprintf((pPointValue+i)->tcValue_MAX,_T("%f"),fTagValue);
									//lstrcpy((pPointValue+i)->tcTime,tchTagTime);			
								}																					
							}

						}
												
					}
					int a = 0;
					a = 20;
				}
				
			}
		}
	}
	TCHAR tchIndex[10] = {0};
	TCHAR tchQuality_MAX[50] = {0};
	TCHAR tchQuality_MIN[50] = {0};
	TCHAR tchQuality[50] = {0};
	int *iMinIndex = new int[iReadTagNum];
	int *iMaxIndex = new int[iReadTagNum];
	for(int m=0;m<iReadTagNum;m++)
	{
		iMinIndex[m]=m;
		iMaxIndex[m]=m;
	}
	int iMinNo = 0;
	int iMaxNo = 0;
	for(int k = 0;k<iReadTagNum-1;k++)
	{
		//将所有的最大值和最小值排序
		//依次取最小的和最大的，重复的则跳过取下一个，直到遍历完所有序号
		for(int j=k;j<iReadTagNum-1;j++)
		{
			 if((pPointValue+iMinIndex[j+1])->fValue_MIN<(pPointValue+iMinIndex[j])->fValue_MIN)
			 {
				 iMinNo = iMinIndex[j];
				 iMinIndex[j] = iMinIndex[j+1];
				 iMinIndex[j+1] = iMinNo;
			 }
		}	
	}

	for(int k = 0;k<iReadTagNum-1;k++)
	{
		//将所有的最大值和最小值排序
		//依次取最小的和最大的，重复的则跳过取下一个，直到遍历完所有序号
		for(int j=k;j<iReadTagNum-1;j++)
		{
			 if((pPointValue+iMaxIndex[j+1])->fValue_MAX>(pPointValue+iMaxIndex[j])->fValue_MAX)
			 {
				 iMaxNo = iMaxIndex[j];
				 iMaxIndex[j] = iMaxIndex[j+1];
				 iMaxIndex[j+1] = iMaxNo;
			 }
		}	
	}

	map<int,int> indexmap;
	for(int k = 0;k<iReadTagNum;k++)
	{
		map<int,int>::iterator it;
		it = indexmap.find(iMinIndex[k]);
		if(it==indexmap.end())
		{
			(pPointValue+iMinIndex[k])->fValue = (pPointValue+iMinIndex[k])->fValue_MIN;
			(pPointValue+iMinIndex[k])->iQuality = (pPointValue+iMinIndex[k])->iQuality_MIN;
			indexmap[iMinIndex[k]] = k;
		}
		it = indexmap.find(iMaxIndex[k]);
		if(it==indexmap.end())
		{
			(pPointValue+iMaxIndex[k])->fValue = (pPointValue+iMaxIndex[k])->fValue_MAX;
			(pPointValue+iMaxIndex[k])->iQuality = (pPointValue+iMaxIndex[k])->iQuality_MAX;
			indexmap[iMaxIndex[k]] = k;
		}
	}
	
	m_ListCtrl.DeleteAllItems();
	//遍历数组显示出所有像素点值

	for(int k = 0;k<iReadTagNum;k++)
	{
		m_ListCtrl.InsertItem( k, _T("") );
		wsprintf(tchIndex,_T("%d"),k+1);
		m_ListCtrl.SetItemText( k, 0, tchIndex);		
		m_ListCtrl.SetItemText( k, 1, (pPointValue+k)->tcTime);	


		TCHAR tcValue_MAX[64];
		TCHAR tcValue_MIN[64];
		TCHAR tcValue[64];

		char chTemp[50]={0};
		sprintf(chTemp,"%.4f",(pPointValue+k)->fValue_MAX);
		STRING2TCHAR(chTemp,tcValue_MAX,64);

		
		sprintf(chTemp,"%.4f",(pPointValue+k)->fValue_MIN);
		STRING2TCHAR(chTemp,tcValue_MIN,64);
		m_ListCtrl.SetItemText( k, 2, tcValue_MIN);	
		wsprintf(tchQuality_MIN,_T("%d"),(pPointValue+k)->iQuality_MIN);
		m_ListCtrl.SetItemText( k, 3, tchQuality_MIN);


		m_ListCtrl.SetItemText( k, 4, tcValue_MAX);	
		wsprintf(tchQuality_MAX,_T("%d"),(pPointValue+k)->iQuality_MAX);
		m_ListCtrl.SetItemText( k, 5, tchQuality_MAX);


		sprintf(chTemp,"%.4f",(pPointValue+k)->fValue);
		STRING2TCHAR(chTemp,tcValue,64);
		m_ListCtrl.SetItemText( k, 6, tcValue);	
		wsprintf(tchQuality,_T("%d"),(pPointValue+k)->iQuality);
		m_ListCtrl.SetItemText( k, 7, tchQuality);
			
	}
	*/
}


void CTestInfluxDB_2Dlg::OnBnClickedQuery222Data()
{
	// TODO: 在此添加控件通知处理程序代码
	char chTagName[64] = { 0 };
	sprintf(chTagName, "DDBTag");

	time_t starttime;
	time_t endtime;


	starttime = FormatTime2("20180402113000");
	endtime = FormatTime2("20180402123000");
	CTime cdate_start_temp = starttime;
	CTime cdate_end = endtime;

	int pix = 5;
	int interval = (endtime - starttime) / pix;
	//查询最大值、最小值，将其拼成一个值
	PointValue *pPointValue = NULL;
	PointValue *pPointValue_temp = NULL;
	pPointValue = new PointValue[pix];
	memset(pPointValue, 0, pix * sizeof(PointValue));

	//查询最小值	
	//查询数据
	string strResponse1_temp;
	string strResponse2_temp;
	string strUrlq = g_strQueryUrlq;//"http://localhost:8086/query?";
	char chQuery_MIN_temp[2000] = { 0 };
	char chQuery_MAX_temp[2000] = { 0 };


	//fill(none)
	sprintf(chQuery_MIN_temp, "SELECT value1,value2,MIN(value) FROM TagValue2 where TagName ='%s'  AND time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
		chTagName, cdate_start_temp.GetYear(), cdate_start_temp.GetMonth(), cdate_start_temp.GetDay(), cdate_start_temp.GetHour(), cdate_start_temp.GetMinute(), cdate_start_temp.GetSecond(),
		cdate_end.GetYear(), cdate_end.GetMonth(), cdate_end.GetDay(), cdate_end.GetHour(), cdate_end.GetMinute(), cdate_end.GetSecond(),
		interval);
	//sprintf(chQuery_MAX_temp, "SELECT MAX(value),Quality FROM TagValue2 where TagName = '%s' AND time>='%4d-%02d-%02dT%02d:%02d:%02dZ' AND time <='%4d-%02d-%02dT%02d:%02d:%02dZ'  group by time(%ds)",
	//	chTagName, cdate_start_temp.GetYear(), cdate_start_temp.GetMonth(), cdate_start_temp.GetDay(), cdate_start_temp.GetHour(), cdate_start_temp.GetMinute(), cdate_start_temp.GetSecond(),
	//	cdate_end.GetYear(), cdate_end.GetMonth(), cdate_end.GetDay(), cdate_end.GetHour(), cdate_end.GetMinute(), cdate_end.GetSecond(),
	//	interval);

	int iRet1_temp = m_pClient->query(strUrlq, "mydb1", chQuery_MIN_temp, strResponse1_temp, 1000);
	cout << "iRet=" << ",Response=" << strResponse1_temp << endl;

	//int iRet2_temp = m_pClient->query(strUrlq, "mydb1", chQuery_MAX_temp, strResponse2_temp, 1000);
	//cout << "iRet=" << ",Response=" << strResponse2_temp << endl;

	Json::Reader reader_1;
	Json::Value root_1;
	int iReadTagNum_EX = 0;
	int iLastValueIndex = -1;//默认所有记录都在连续查询表中存在

	string sTagTime;
	float fTagValue = 0.0000;

	int iQuality = 0;
	string sQuality;
	string sValue1;
	string sValue2;

	//从字符串中读取数据
	if (reader_1.parse(strResponse1_temp, root_1))
	{
		if (root_1["results"].isArray())
		{
			if (root_1["results"][0]["series"].isArray())
			{
				if (root_1["results"][0]["series"][0]["values"].isArray())
				{
					int nArraySize = root_1["results"][0]["series"][0]["values"].size();
					iReadTagNum_EX = nArraySize > (pix - iLastValueIndex - 1) ? (pix - iLastValueIndex - 1) : nArraySize;

					memset(pPointValue, 0, nArraySize * sizeof(PointValue));
					for (int i = 0; i < iReadTagNum_EX; i++)
					{
						if (root_1["results"][0]["series"][0]["values"][i].isArray())
						{

							CStringA strDebug;
							strDebug.Format("gao1015 %d \r\n", i);
							OutputDebugStringA(strDebug);

							sTagTime = root_1["results"][0]["series"][0]["values"][i][0].asCString();


							wchar_t tchTagTime[50] = { 0 };
							STRING2TCHAR(sTagTime, tchTagTime, 50);
							//lstrcpy((pPointValue_temp + i)->tcTime, tchTagTime);
							if (root_1["results"][0]["series"][0]["values"][i][1].isNull())
							{
								//int a = 0;
								//a = 20;
							}
							else
							{

								fTagValue = root_1["results"][0]["series"][0]["values"][i][1].asFloat();
								//iQuality = root_1["results"][0]["series"][0]["values"][i][2].asInt();

								sValue1 = root_1["results"][0]["series"][0]["values"][i][2].asCString();

								//sValue2 = root_1["results"][0]["series"][0]["values"][i][3].asCString();
								
								string strFilePath = "D:\\99se\\BOM222.png";
								FILE *fp;
								fp = fopen(strFilePath.c_str(), "wb");
								if (fp != NULL)
								{
									//sQuality += "=";
									sQuality = sValue1;
									sQuality += sValue2;
									int iTemp = sQuality.length();
									string mytring = Base64Decode(sQuality);

									int nFileLen = mytring.length();
									fseek(fp, 0, SEEK_SET);
									char *pBuffer = new char[nFileLen + 1];
									memset(pBuffer, 0, nFileLen);
									memcpy(pBuffer, mytring.c_str(), nFileLen);
									fwrite(pBuffer, nFileLen, 1, fp);
									fclose(fp);
								}
								
								iQuality = atoi(sQuality.c_str());

								(pPointValue_temp + i)->iQuality_MIN = iQuality;

								(pPointValue_temp + i)->fValue_MIN = fTagValue;
								//char chTemp[50]={0};
								//sprintf(chTemp,"%.4f",fTagValue);

								//STRING2TCHAR(chTemp,(pPointValue+i)->tcValue_MIN,64);


							}
						}

					}
					//int a = 0;
					//a = 20;
				}

			}
		}
	}

}


void CTestInfluxDB_2Dlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	g_bExit = TRUE;
	CDialogEx::OnClose();
}


void CTestInfluxDB_2Dlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	g_bExit = TRUE;
	CDialogEx::OnCancel();
}


void CTestInfluxDB_2Dlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码

}


void CTestInfluxDB_2Dlg::OnBnClickedStartProcess()
{
	// TODO: 在此添加控件通知处理程序代码

	if(!m_bIsInProcessing)
	{
		g_bExit = false;
		m_bIsInProcessing = true;
		UpdateData(TRUE);
		m_QueryInfluxdbThread[0].m_iStartProcessPointIndex = m_StartProcesPointIndex;
		m_QueryInfluxdbThread[0].m_iStartProcessDayIndex = m_StartProcessDayIndex;
		m_QueryInfluxdbThread[0].StartThread();



		((CEdit*)GetDlgItem(IDC_EDIT_STAERT_POINTINDEX))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_START_DAYINDEX))->EnableWindow(false);
		m_Start_Stop_Process.SetWindowTextW(_T("停止查询"));
	}
	else
	{
		m_bIsInProcessing = false;
		g_bExit = true;

		((CEdit*)GetDlgItem(IDC_EDIT_STAERT_POINTINDEX))->EnableWindow(true);
		((CEdit*)GetDlgItem(IDC_EDIT_START_DAYINDEX))->EnableWindow(true);
		m_Start_Stop_Process.SetWindowTextW(_T("开始查询"));
	}
	
}


void CTestInfluxDB_2Dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	switch(nIDEvent)
	{
	case 1:
		{
			CString strPointIndex;
			CString strDataIndex;
			strPointIndex.Format(_T("%d"),g_CurrentProcessPointNumIndex);
			strDataIndex.Format(_T("%d"),g_CurrentProcessDayIndex);
			GetDlgItem(IDC_EDIT_STAERT_POINTINDEX2)->SetWindowTextW(strPointIndex);
			GetDlgItem(IDC_EDIT_START_DAYINDEX2)->SetWindowTextW(strDataIndex);
		}
		break;
	default:
		{
		}

	}
	

	CDialogEx::OnTimer(nIDEvent);
}
