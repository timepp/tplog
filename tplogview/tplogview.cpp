// lgui.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "config.h"
#include "mainframe.h"
#include <time.h>
#include "optiondlg.h"
#include "logicvis.h"
#include "servicehelper.h"
#include "scripthost.h"
#include "updater.h"

CAppModule _Module;
HWND g_activeModlessDlg = NULL;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	CModlessDlgKeyProcessor mdkp;

	Updater::NotifyNewVersion();

	if (CConfig::Instance()->GetConfig().log_config_path.empty())
	{
		COptionDlg dlg;
		dlg.SetWizzardMode(true);
		dlg.DoModal();
	}

	_Module.AddMessageLoop(&theLoop);
	theLoop.AddMessageFilter(&mdkp);

	ServiceHelper::GetVisualLogic()->Init();
	ServiceHelper::GetLogPropertyDB()->Init();
	
	CMainFrame g_mainframe;
	if(g_mainframe.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	g_mainframe.ShowWindow(nCmdShow);

	// update thread
	if (_time64(NULL) - CConfig::Instance()->GetConfig().syscfg.last_check_update_time > 86400)
	{
		CConfig::Instance()->GetConfig().syscfg.last_check_update_time = _time64(NULL);
		// Updater::CheckAndUpdate(TRUE);
	}

	int nRet = theLoop.Run();

	ServiceHelper::GetVisualLogic()->Uninit();

	theLoop.RemoveMessageFilter(&mdkp);
	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR lpstrCmdLine, int nCmdShow)
{
	::OleInitialize(NULL);
	ON_LEAVE(::OleUninitialize());

	::LoadLibrary(L"Riched20.dll");

	CConfig* cfg = CConfig::Instance();
	cfg->Load(L"");

	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof(iccx);
	iccx.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES;
	::InitCommonControlsEx(&iccx);


	GD.module = hInstance;
	int argc = 0;
	LPWSTR* argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

	if (argc >= 2)
	{
		GD.strXLogFile = argv[1];
	}

	HRESULT hRes = _Module.Init(NULL, hInstance);
	(hRes);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();

	CConfig::Instance()->Save();

	tp::global_servicemgr().destroy_all_services();

	return nRet;
}
