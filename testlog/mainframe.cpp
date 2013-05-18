#include "stdafx.h"
#include "mainframe.h"
#include <atltime.h>
#include <wtl/atldlgs.h>
#include <math.h>
#include <share.h>
#include <shlobj.h>

extern CAppModule _Module;

#undef MULTI_THREAD_GUARD
#define MULTI_THREAD_GUARD(cs) \
	cs.Enter(); \
	ON_LEAVE_1(cs.Leave(), CCriticalSection&, cs)

#define KEY_IS_DOWN(vk) ((GetKeyState(vk) & 0x8000) != 0)

CMainFrame::CMainFrame()
: m_bShowToolbar(true)
, m_bShowStatusbar(true)
{
}

void CMainFrame::CreateReBar()
{
	struct ToolBarButtonInfo
	{
		int cmdID;
		BYTE state;
		BYTE style;
		UINT iconID;
	} tbbi[] =
	{
		{ID_START_TEST,   TBSTATE_ENABLED, BTNS_BUTTON, 0},
		{0,                  0, BTNS_SEP,    0},
	};

	m_toolbar.Create(m_hWnd, 0, 0, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	m_toolbar.GetToolTips().SetDlgCtrlID(IDC_TOOLBAR);
	m_toolbarImageList.Create(16, 16, ILC_COLOR32|ILC_MASK, 10, 10);
	m_toolbar.SetImageList(m_toolbarImageList);
	int index = 0;
	for (size_t i = 0; i < _countof(tbbi); ++i, ++index)
	{
		if (tbbi[i].iconID != 0)
		{
			m_toolbarImageList.AddIcon(::LoadIconW(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCEW(tbbi[i].iconID)));
			m_toolbar.InsertButton(index, tbbi[i].cmdID, tbbi[i].style, tbbi[i].state, m_toolbarImageList.GetImageCount()-1, 0, NULL);
		}
		else
		{
			m_toolbar.InsertButton(index, 0, BTNS_SEP, 0, 7, 0, NULL);
		}
	}

	m_cmdbar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_cmdbar.AttachMenu(GetMenu());
//	m_cmdbar.LoadImages(IDR_MAINFRAME);
//	SetMenu(NULL);


	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(m_cmdbar);
	AddSimpleReBarBand(m_toolbar, NULL, TRUE);
	m_rebar = m_hWndToolBar;

	UIAddToolBar(m_toolbar);
//	UISetCheck(ID_TOGGLE_TOOLBAR, true);
}

void CMainFrame::CreateStatusBar()
{
	int panes[] = {ID_DEFAULT_PANE};
	m_wndStatusbar.Create(m_hWnd);
	m_wndStatusbar.SetPanes(panes, _countof(panes), true);

	m_hWndStatusBar = m_wndStatusbar;
	UIAddStatusBar(m_hWndStatusBar);
//	UISetCheck(ID_TOGGLE_STATUSBAR, true);
}

void CMainFrame::CreateList()
{
	DWORD dwStyle = LVS_REPORT|LVS_OWNERDATA;
	DWORD dwExStyle = LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER;
//	dwExStyle |= LVS_EX_GRIDLINES;

	m_list.Create(m_hWnd, rcDefault, NULL, WS_CHILD|WS_VISIBLE|dwStyle);
	m_list.SetExtendedListViewStyle(dwExStyle);
	m_list.SetDlgCtrlID(IDC_LIST);
	m_listFont.CreateFontW(-12, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"宋体");
	//m_list.SetFont(m_listFont);
	m_listImageList.Create(16, 16, ILC_COLOR32|ILC_MASK, 2, 2);
	m_listImageList.AddIcon(::LoadIconW(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCEW(0)));
	m_list.SetImageList(m_listImageList, LVSIL_SMALL);

	struct
	{
		LPCWSTR name;
		int width;
	}columnInfo[] =
	{
		L"", 20,
		L"结果", 60, 
		L"类别", 200,
		L"说明", 500,
		L"附加信息", 200,
	};
	for (int i = 0; i < _countof(columnInfo); i++)
	{
		m_list.AddColumn(columnInfo[i].name, i);
		m_list.SetColumnWidth(i, columnInfo[i].width);
	}

	m_hWndClient = m_list;
}

void CMainFrame::OnSysCommand(UINT id, CPoint pt)
{
	SetMsgHandled(FALSE);
}

LRESULT CMainFrame::OnCreate( LPCREATESTRUCTW /*cs*/ )
{
//	CreateReBar();
	CreateStatusBar();
	CreateList();
	UpdateUI();

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	SetWindowText(L"tplogtest");
	PostMessage(WM_COMMAND, ID_START_TEST);

	return 0;
}

LRESULT CMainFrame::OnViewToolbar(WORD , WORD , HWND , BOOL& )
{
	UpdateUI();
	return 0;
}

LRESULT CMainFrame::OnViewStatusbar(WORD , WORD , HWND , BOOL& )
{
	UpdateUI();
	return 0;
}

void CMainFrame::UpdateUI()
{
	UpdateLayout();
}

void CMainFrame::OnTimer(UINT nID)
{
}

LRESULT CMainFrame::OnNMDblclkList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pNMHDR;

	return 0;
}

LRESULT CMainFrame::OnNMCustomdrawList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		return CDRF_NOTIFYITEMDRAW;
	}
	if (pNMCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		int index = static_cast<int>(pNMCD->nmcd.dwItemSpec);
		const tp::TestResult& tr = m_results[index];
		if (!tr.success)
		{
			pNMCD->clrTextBk = RGB(255, 224, 224);
			//return CDRF_NOTIFYSUBITEMDRAW;
		}
	}
	if (pNMCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT|CDDS_ITEM|CDDS_SUBITEM))
	{
		if (pNMCD->iSubItem == 4)
		{
			pNMCD->clrTextBk = RGB(0, 255, 0);
		}
	}
	return CDRF_DODEFAULT;
}

LRESULT CMainFrame::OnNMRclickList(int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& /*bHandled*/)
{
	int index = m_list.GetSelectedIndex();
	if (index < 0)
	{
		return 0;
	}

	POINT pt;
	::GetCursorPos(&pt);
	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, 0U, L"清除此条之前的所有日志(&D)");
	menu.AppendMenu(MF_STRING, 0U, L"设置/取消书签(&M)");

	menu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);

	return 0;
}

BOOL CMainFrame::PreTranslateMessage( MSG* pMsg )
{
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();

	return TRUE;
}

LRESULT CMainFrame::OnNMClick(int , LPNMHDR pNMHDR, BOOL& )
{
	return 0;
}

LRESULT CMainFrame::OnListStateChanged(int, LPNMHDR pNMHDR, BOOL&)
{
	LPNMLISTVIEW pInfo = (LPNMLISTVIEW)pNMHDR;
	m_list.RedrawItems(0, -1);
	m_list.RedrawWindow();
	return 0;
}

LRESULT CMainFrame::OnListGetDispInfo(int /**/, LPNMHDR pNMHDR, BOOL& /**/)
{
	static const LPCWSTR cname[] = {L"", L"调试", L"信息", L"警告", L"错误", L"严重"};

	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem = &(pDispInfo)->item;

	int index = pItem->iItem;
	int subindex = pItem->iSubItem;
	size_t textlen = static_cast<size_t>(pItem->cchTextMax);

	const tp::TestResult& tr = m_results[index];

	wchar_t cvtbuf[1024];
	if (pItem->mask & LVIF_TEXT)
	{
		switch(subindex)
		{
		case 1:
			wcsncpy_s(pItem->pszText, textlen, tr.success?L"成功":L"失败", _TRUNCATE);
			break;
		case 2:
			wcsncpy_s(pItem->pszText, textlen, tr.block->name, _TRUNCATE);
			break;
		case 3:
			wcsncpy_s(pItem->pszText, textlen, tr.operation.c_str(), _TRUNCATE);
			break;
		case 4:
			wcsncpy_s(pItem->pszText, textlen, tr.comment.c_str(), _TRUNCATE);
			break;
		}
	}
	if (pItem->mask & LVIF_IMAGE)
	{
		pItem->iImage = 0;
	}

	return 0;
}

void CMainFrame::OnDestroy()
{
	::PostQuitMessage(0);
}

void CMainFrame::OnMenuSelect(UINT nID, UINT flags, HMENU hMenu)
{
	if (flags == 0xFFFF && hMenu == NULL)
	{
		m_wndStatusbar.SetPaneText(0, L"");
	}
	else
	{
		WCHAR buffer[1024] = L"";
		AtlLoadString(nID, buffer, _countof(buffer));
		LPWSTR p = wcschr(buffer, L'\n');
		if (p != NULL) *p = L'\0';
		m_wndStatusbar.SetPaneText(0, buffer);
	}
}


void CMainFrame::OnContextMenu(HWND hWnd, CPoint pt)
{
	if (hWnd == m_hWnd)
	{
		SetMsgHandled(FALSE);
	}
	else if (hWnd == m_list.m_hWnd)
	{
		int index = m_list.GetSelectedIndex();
		if (index < 0)
		{
			return;
		}

		CPoint ptClient;
		if (pt.x == -1 && pt.y == -1)
		{
			CRect rcBounds;
			m_list.GetItemRect(index, &rcBounds, LVIR_BOUNDS);
			ptClient.x = (rcBounds.left + rcBounds.right) / 2;
			ptClient.y = rcBounds.bottom;
			pt = ptClient;
			m_list.ClientToScreen(&pt);
		}
		else
		{
			ptClient = pt;
			m_list.ScreenToClient(&ptClient);
		}

		CRect rcHdr;
		CHeaderCtrl hdrCtrl = m_list.GetHeader();
		hdrCtrl.GetWindowRect(&rcHdr);
		if (ptClient.y > rcHdr.Height())
		{
			CMenu menu;
			menu.CreatePopupMenu();

			menu.AppendMenu(MF_STRING, 0U, L"跳到函数头/尾(&F)");


			menu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
		}
	}
}

LRESULT CMainFrame::OnStartTest(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_list.DeleteAllItems();
	HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, work_thread, this, 0, NULL);
	::CloseHandle(thread);
	return 0;
}

unsigned int CMainFrame::work_thread(void* param)
{
	CMainFrame* frame = static_cast<CMainFrame*>(param);
	tp::unittest& ut = tp::unittest::instance();
	ut.set_test_output(frame);
	ut.run_test();
	return 0;
}

static std::wstring blockname;
void CMainFrame::BlockBegin(const tp::TestBlock& block)
{
	blockname = block.name;
}
void CMainFrame::OutputResult(const tp::TestResult& res)
{
	m_results.push_back(res);
	m_list.SetItemCount(m_results.size());
}
void CMainFrame::TestEnd(int total, int succeeded)
{
	CStringW info;
	info.Format(L"总数: %d, 成功: %d, 失败: %d", total, succeeded, total-succeeded);
	m_wndStatusbar.SetPaneText(0, info);
}
