#pragma  once
#include "resource.h"
#include "config.h"

class CFirsttimeDlg
	: public CDialogImpl<CFirsttimeDlg>
{
public:
	enum {IDD = IDD_FIRSTTIME};

	BEGIN_MSG_MAP(CFirsttimeDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

private:

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
	{
		CenterWindow(GetParent());
		return 0;
	}

	void OnClose()
	{
//		EndDialog(IDOK);
	}

	LRESULT OnOK(WORD , WORD , HWND , BOOL& )
	{
		CStringW text;
		GetDlgItem(IDC_COMBO_PRODUCT).GetWindowText(text);
		if (!text.IsEmpty())
		{
			CConfig::Instance()->GetConfig().log_config_path = text;
			EndDialog(IDOK);
		}
		
		return 0;
	}

	LRESULT OnCancel(WORD , WORD , HWND , BOOL& )
	{
		OnClose();
		return 0;
	}
};

