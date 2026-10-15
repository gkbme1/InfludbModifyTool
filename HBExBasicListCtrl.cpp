// HBExBasicListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "HBExBasicListCtrl.h"
#include <vector>
using namespace std;
extern vector<NeedModifyPoints*> g_AllTagListInfo;
extern time_t g_StartTime;

CString& StringtoCString( CString& strDst,const std::string& strSrc )
{
	int nStrSourceLen = ::MultiByteToWideChar(CP_ACP, NULL, strSrc.c_str(), -1, NULL, 0);
	wchar_t *pwszString = new wchar_t[nStrSourceLen];
	if (NULL == pwszString)
		return strDst;
	memset(pwszString,0,nStrSourceLen);
	::MultiByteToWideChar(CP_ACP, NULL, strSrc.c_str(), -1, pwszString, nStrSourceLen);
	strDst.Format(_T("%s"),pwszString);

	delete []pwszString;
	return strDst;
}


// CHBExBasicListCtrl

IMPLEMENT_DYNAMIC(CHBExBasicListCtrl, CListCtrl)

CHBExBasicListCtrl::CHBExBasicListCtrl()
{
}

CHBExBasicListCtrl::~CHBExBasicListCtrl()
{
}


BEGIN_MESSAGE_MAP(CHBExBasicListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CHBExBasicListCtrl::OnGetdispinfo)
	ON_WM_MEASUREITEM_REFLECT()
	ON_NOTIFY_REFLECT(LVN_ODFINDITEM, &CHBExBasicListCtrl::OnLvnOdfinditem)
	ON_NOTIFY_REFLECT(LVM_FINDITEM, &CHBExBasicListCtrl::OnLvnOdfinditem)

	
END_MESSAGE_MAP()



// CHBExBasicListCtrl 消息处理程序
void CHBExBasicListCtrl::OnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{

    OnGetdispinfoDcs(pNMHDR, pResult);
}


void CHBExBasicListCtrl::OnGetdispinfoDcs(NMHDR *pNMHDR, LRESULT *pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;
	CString strPrompt(_T(""));

		int iItemIndx= pItem->iItem;
		char chIndex[20] = {0};
		CString stData;
		time_t CalculateTime;
		CTime ctCalTime;
		if (pItem->mask & LVIF_TEXT) //字符串缓冲区有效
		{
			switch(pItem->iSubItem){
			case 0: //填充数据项的名字
				sprintf(chIndex,"%d",iItemIndx+1);
				
				StringtoCString(stData,chIndex);
				lstrcpy(pItem->pszText,stData);
				break;
			case 1: //填充旧的点名
				StringtoCString(stData,g_AllTagListInfo[iItemIndx]->chOldPointName);
				lstrcpy(pItem->pszText,stData);
				break;
			case 2: //填充新的点名
				StringtoCString(stData,g_AllTagListInfo[iItemIndx]->chNewPointName);
				lstrcpy(pItem->pszText,stData);
				break;
			case 3: //第几个点
				sprintf(chIndex,"%d",g_AllTagListInfo[iItemIndx]->iPointIndex);
				
				StringtoCString(stData,chIndex);
				lstrcpy(pItem->pszText,stData);
				break;
			case 4: //第几天
				sprintf(chIndex,"%d",g_AllTagListInfo[iItemIndx]->iIndex_Day);
				
				StringtoCString(stData,chIndex);
				lstrcpy(pItem->pszText,stData);
				break;

			case 5: //填充时间

				//根据起始时间和时间偏移计算时间
				CalculateTime = g_StartTime+(g_AllTagListInfo[iItemIndx]->iIndex_Day-1)*24*60*60;
				ctCalTime = CalculateTime;
				stData = ctCalTime.Format("%Y/%m/%d");				
				lstrcpy(pItem->pszText,stData);
				break;

			case 6: //填充设定值

				sprintf(chIndex,"%f",g_AllTagListInfo[iItemIndx]->fMiddleValue);
				StringtoCString(stData,chIndex);
				lstrcpy(pItem->pszText,stData);
				break;
			case 7: //填充偏差值

				sprintf(chIndex,"%f",g_AllTagListInfo[iItemIndx]->fOffet);
				StringtoCString(stData,chIndex);
				lstrcpy(pItem->pszText,stData);
				break;
			default:
				break;
			}
		}

}

void CHBExBasicListCtrl::InitListCtrl()	//初始化ListCtrl
{	
	LONG lStyle;
	lStyle = GetWindowLong(m_hWnd, GWL_STYLE);//获取当前窗口风格		
	lStyle |=LVS_REPORT | LVS_SHOWSELALWAYS/* | LVS_SINGLESEL */ | LVS_OWNERDRAWFIXED | LVS_OWNERDATA; 	

	SetWindowLong(m_hWnd, GWL_STYLE, lStyle); //设置窗口风格

	DWORD dwStyle = GetExtendedStyle();
	
	dwStyle |= LVS_EX_FULLROWSELECT;
	SetExtendedStyle(dwStyle); //设置扩展风格
	SetBkColor(RGB(255, 255, 255)); //设置背景颜色
	SetTextBkColor(RGB(255, 255, 255)); //设置文本背景颜色
	SetTextColor(RGB(10, 10, 80)); //设置文本颜色

	//AutoSizeColumns();	//自动列宽
}


void CHBExBasicListCtrl::SetBackColor(COLORREF cr)
{
	m_bkClr = cr;

	SetBkColor(cr); //设置背景颜色
	SetTextBkColor(cr);
}

//lint -e438
//lint -e529
void CHBExBasicListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)	//ListCtrl自绘函数
{
	
	int nItem = lpDrawItemStruct->itemID;			//行号
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rcItem = lpDrawItemStruct->rcItem;		//行区域
	pDC->SetBkMode(TRANSPARENT);

	//CFont* oldFont;
	//CFont* newFont = new CFont();
	//newFont->CreatePointFont(100,_T("宋体"));
	//oldFont = pDC->SelectObject(newFont);

	if((lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		pDC->FillSolidRect(rcItem,RGB(49,106,197));  
		pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else
	{
		//设置交替行颜色
		if ( nItem & 1 )			
		{			
			pDC->FillSolidRect(rcItem,RGB(233,243,253));  
			pDC->SetTextColor(RGB(0,0,0));                
		}
		else
		{
			pDC->FillSolidRect(rcItem,RGB(255,255,255));  
			pDC->SetTextColor(RGB(0,0,0));               
		}	

	}
	

	//通过for循环绘制每行的文本
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT;
	CPen pen(PS_SOLID,1,RGB(255, 255, 255)); 
	CPen *OldPen = (CPen *)pDC->SelectObject(&pen);
	CRect rcSubItem;
	CString strText;

	for( int nColumnCount = 0; GetColumn(nColumnCount,&lvc); nColumnCount++ )   
	{
		GetSubItemRect(nItem,nColumnCount,LVIR_LABEL,rcSubItem);
		strText=GetItemText(nItem,nColumnCount);
		//绘制Item文本
		pDC->DrawText( strText, rcSubItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);	
	}


}
//lint +e438
//lint +e529

void CHBExBasicListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{	
	lpMeasureItemStruct->itemHeight = 22;
}

void CHBExBasicListCtrl::AutoSizeColumns()
{  
	SetRedraw(false);
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	ASSERT(pHeaderCtrl != NULL);
	for (int i=0; i < pHeaderCtrl->GetItemCount(); i++)
	{
		SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
	}
	SetRedraw(true);	
	Invalidate();	
}


void CHBExBasicListCtrl::OnLvnOdfinditem(NMHDR *pNMHDR, LRESULT *pResult)
{

	// pNMHDR has information about the item we should find
	// In pResult we should save which item that should be selected
	NMLVFINDITEM* pFindInfo = (NMLVFINDITEM*)pNMHDR;
 
	/* pFindInfo->iStart is from which item we should search.
	   We search to bottom, and then restart at top and will stop
	   at pFindInfo->iStart, unless we find an item that match
	 */
 
	// Set the default return value to -1
	// That means we didn't find any match.
	*pResult = -1;
 

	char chDebug[255];
	sprintf(chDebug,"gao0730 1111");
	OutputDebugStringA(chDebug);

	//Is search NOT based on string?
	if( (pFindInfo->lvfi.flags & LVFI_STRING) == 0 )
	{
		//This will probably never happend...
		return;
	}
 
	/*
		Let's look on a sample list;
		
		  Name
		  Anders
		* Anna
		  Annika
		  Bob
		  Emma
		  Emmanuel
		Anna is selected. 
		If "A" is written, Annika should be selected.
		If "AND" is written, Anders should be selected. 
		If "ANNK" is written, the selection should stay on Anna.
		If "E" is written, Emma should be selected.
    */
 
	//This is the string we search for
	CString searchstr = pFindInfo->lvfi.psz;
 
	sprintf(chDebug,"gao0730 Find: %s\n",searchstr);
	OutputDebugStringA(chDebug);

//	TRACE(_T("Find: %s\n"), searchstr);
	
	int startPos = pFindInfo->iStart;
	//Is startPos outside the list (happens if last item is selected)
	if(startPos >= GetItemCount())
		startPos = 0;
 
	int currentPos=startPos;
	
	//Let's search...
	//do
	//{		
	//	//Do this word begins with all characters in searchstr?
	//	if( _tcsnicmp(m_Data[currentPos].szMacsPointName, searchstr, searchstr.GetLength()) == 0)
	//	{
	//		//Select this item and stop search.
	//		*pResult = currentPos;
	//		break;
	//	}
 //
	//	//Go to next item
	//	currentPos++;
 //
	//	//Need to restart at top?
	//	if(currentPos >= GetItemCount())
	//		currentPos = 0;
 //
	////Stop if back to start
	//}while(currentPos != startPos);		

	//sprintf(chDebug,"gao0730 Find: %s\n",searchstr);
	//OutputDebugStringA(chDebug);



}
