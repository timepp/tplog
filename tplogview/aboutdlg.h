#pragma  once
#include "resource.h"
#include "whitebkdlg.h"
#include "config.h"
#include "helper.h"

const LPCWSTR g_tips[] =
{
	L"������ctrl+f�ڹ��˽���н�������Ŷ",
	L"��־��Ϣ�Ի������ѡ���Զ�����",
	L"�������ø������򣬰ѷ�����������־ͻ����ʾ",
};

class CAboutDlg
	: public CDialogImpl<CAboutDlg>
	, public CColoredDlgImpl
{
public:
	enum {IDD = IDD_ABOUT};

	BEGIN_MSG_MAP(CAboutDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BTN_DOWNLOAD, OnDownload)

		COMMAND_HANDLER(IDC_BUTTON_SHOW_VERSIONINFO, BN_CLICKED, OnBnClickedButtonShowVersioninfo)
		CHAIN_MSG_MAP(CColoredDlgImpl)
	END_MSG_MAP()

private:

	CHyperLink m_link;

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
	{
		CenterWindow(GetParent());

		CStringW strConfPath = CConfig::Instance()->GetConfigFilePath();
		m_link.SubclassWindow(GetDlgItem(IDC_STATIC_CONF_PATH));
		m_link.SetLabel(strConfPath);
		m_link.SetHyperLink(strConfPath);

		GetDlgItem(IDC_STATIC_LOG_CONFIG_PATH).SetWindowText(CConfig::Instance()->GetConfig().log_config_path.c_str());
		GetDlgItem(IDC_STATIC_VERSION).SetWindowText(helper::GetVersion());

		SetStaticTextColor(IDC_STATIC_INFO, RGB(0, 0, 255));
		CConfig::Instance()->Save();

		return 0;
	}
	void OnClose()
	{
		EndDialog(IDOK);
	}
	LRESULT OnOK(WORD , WORD , HWND , BOOL& )
	{
		OnClose();
		return 0;
	}
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& )
	{
		OnClose();
		return 0;
	}
	LRESULT OnDownload(WORD , WORD , HWND , BOOL& )
	{
		return 0;
	}
public:
	LRESULT OnBnClickedButtonShowVersioninfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
