#include "resource.h"
#include <shellapi.h>
#include <wtl/atlframe.h>
#include <wtl/atlcrack.h>
#include <wtl/atlmisc.h>
#include <wtl/atlctrls.h>
#include <wtl/atlctrlx.h>
#include <wtl/atlctrlw.h>
#include <tplib/include/unittest.h>

class CMainFrame 
	: public CFrameWindowImpl<CMainFrame>
	, public CUpdateUI<CMainFrame>
	, public CMessageFilter
	, public CIdleHandler
	, public tp::TestOutput
{
public:

	CMainFrame();

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_MSG_MAP(CMainFrame)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_MENUSELECT(OnMenuSelect)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		
		//NOTIFY_HANDLER(IDC_LIST, NM_DBLCLK, OnNMDblclkList)
		//NOTIFY_HANDLER(IDC_LIST, NM_CUSTOMDRAW, OnNMCustomdrawList)
		//NOTIFY_HANDLER(IDC_LIST, LVN_GETDISPINFO, OnListGetDispInfo)
		//NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListStateChanged)

		COMMAND_ID_HANDLER(ID_START_TEST, OnStartTest)

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

	virtual void BlockBegin(const tp::TestBlock& block);
	virtual void OutputResult(const tp::TestResult& res);
	virtual void TestEnd(int total, int succeeded);

private:

	void CreateReBar();
	void CreateStatusBar();
	void CreateList();

	void UpdateUI();

private:
	LRESULT OnCreate(LPCREATESTRUCTW cs);
	void OnDestroy();
	void OnTimer(UINT nID);
	void OnMenuSelect(UINT nID, UINT flags, HMENU hMenu);
	void OnContextMenu(HWND hWnd, CPoint pt);
	void OnSysCommand(UINT id, CPoint pt);
	LRESULT OnNMDblclkList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawList(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnNMRclickList(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnListGetDispInfo(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnListStateChanged(int, LPNMHDR pNMHDR, BOOL&);
	LRESULT OnNMClick(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnListFindItem(int /**/, LPNMHDR pNMHDR, BOOL& /**/);
	LRESULT OnViewToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStartTest(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
private:
	CToolBarCtrl m_toolbar;
	CCommandBarCtrl m_cmdbar;
	CReBarCtrl m_rebar;
	CImageList m_toolbarImageList;
	bool m_bShowToolbar;
	CFont m_editFont;

	CMultiPaneStatusBarCtrl m_wndStatusbar;
	bool m_bShowStatusbar;
	CIcon m_filterIcon;

	CListViewCtrl m_list;
	CImageList m_listImageList;
	CFont m_listFont;
};
