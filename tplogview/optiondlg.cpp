#include "stdafx.h"
#include "optiondlg.h"

COptionDlg::COptionDlg() : m_wizzardMode(false)
{
}

void COptionDlg::SetWizzardMode(bool m)
{
	m_wizzardMode = m;
}

LRESULT COptionDlg::OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
{
	CenterWindow(GetParent());

	CreateOptionPages();

	m_tree = GetDlgItem(IDC_TREE);
	m_tree.ModifyStyle(0, TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT);

	if (m_wizzardMode)
	{
		CTreeItem itemSys = AddOptionPage(NULL, IDS(IDS_SYSTEM), &m_optSystem);
		SetWindowText(IDS(IDS_SETTING_WIZZARD));
		itemSys.Select();
	}
	else
	{
		CTreeItem itemView = AddOptionPage(NULL, IDS(IDS_SHOW), NULL);
		AddOptionPage(itemView, IDS(IDS_PREFMARK), &m_optPerfMark);
        CTreeItem itemSource = AddOptionPage(NULL, IDS(IDS_LOG_SOURCE), NULL);
        AddOptionPage(itemSource, IDS(IDS_LOG_SOURCE_PIPE), NULL);
        AddOptionPage(itemSource, IDS(IDS_LOG_SOURCE_DEBUG_OUTPUT), NULL);
        AddOptionPage(itemSource, IDS(IDS_LOG_SOURCE_FILE), NULL);
        AddOptionPage(itemSource, IDS(IDS_LOG_SOURCE_SHARE_MEMORY), NULL);
		CTreeItem itemSys = AddOptionPage(NULL, IDS(IDS_SYSTEM), &m_optSystem);
	
        ExpandAllChildItem(m_tree.GetRootItem());
		itemView.Select();
	}

	return 0;
}

LRESULT COptionDlg::OnOK(WORD , WORD , HWND , BOOL& )
{
	SaveAllPages();
	EndDialog(IDOK);
	return 0;
}
LRESULT COptionDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT COptionDlg::OnApply(WORD , WORD , HWND , BOOL& )
{
	SaveAllPages();
	return 0;
}

CTreeItem COptionDlg::AddOptionPage(CTreeItem root, LPCWSTR name, COptPage* page)
{
	CTreeItem item = m_tree.InsertItem(name, root, TVI_LAST);
	item.SetData((DWORD_PTR)page);
	return item;
}

void COptionDlg::ExpandAllChildItem(CTreeItem item)
{
	item.Expand();
	for (CTreeItem child = item.GetChild(); !child.IsNull(); child = child.GetNextSibling())
	{
		ExpandAllChildItem(child);
	}
}

void COptionDlg::CreateOptionPages()
{
	m_optPerfMark.Create(m_hWnd);
	m_optSystem.Create(m_hWnd);

	m_pages.push_back(&m_optPerfMark);
	m_pages.push_back(&m_optSystem);

	CWindow wnd = GetDlgItem(IDC_STATIC_OPTION_CLIENT);
	CRect rc;
	wnd.GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.DeflateRect(2, 2, 2, 2);

	for (size_t i = 0; i < m_pages.size(); i++)
	{
		dynamic_cast<CWindow&>(*m_pages[i]).MoveWindow(&rc);
	}
}

void COptionDlg::SaveAllPages()
{
	for (size_t i = 0; i < m_pages.size(); i++)
	{
		m_pages[i]->Save();
	}
}

void COptionDlg::SwitchToPage(COptPage* page)
{
	for (size_t i = 0; i < m_pages.size(); i++)
	{
		int showcmd = m_pages[i] == page? SW_SHOW : SW_HIDE;
		dynamic_cast<CWindow&>(*m_pages[i]).ShowWindow(showcmd);
	}
}


LRESULT COptionDlg::OnTvnSelchangedTree(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	// LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	
	COptPage* page = (COptPage*)m_tree.GetSelectedItem().GetData();
	SwitchToPage(page);

	return 0;
}
