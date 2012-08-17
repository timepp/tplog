#include "stdafx.h"
#include "logcontroller.h"
#include "outputdevice_debugoutput.h"
#include "outputdevice_pipe.h"
#include "outputdevice_sharememory.h"
#include "outputdevice_file.h"
#include <process.h>

#define LOG_CONFIG_SECTION_NAME L"LOG_CONFIG"

#define CHECK_STATE \
	if (!m_constructed) return BDLOG_E_BEFORE_CONSTRUCT; \
	if (m_destructed) return BDLOG_E_DESTRUCTED; \
	if (!m_inited) return BDLOG_E_NOT_INITED;


CLogController::CLogController()
: m_inited(false)
, m_destructed(false)
, m_monitorThread(NULL)
, m_monitorThreadQuitEvent(NULL)
, m_odsLen(0)
, m_logID(1)
, m_pid(0)
, m_calldepthTlsIndex(0)
, m_hConfigKey(NULL)
{
	LOGFUNC;
	::InitializeCriticalSection(&m_csLog);

	m_configpath[0] = 0;
	m_constructed = true;
}

CLogController::~CLogController()
{
	LOGFUNC;
	UnInit();
	m_destructed = true;

	::DeleteCriticalSection(&m_csLog);
}

HRESULT CLogController::Init(const wchar_t* configname)
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);

	if (!m_constructed) return BDLOG_E_BEFORE_CONSTRUCT;
	if (m_destructed) return BDLOG_E_DESTRUCTED;
	if (m_inited) return BDLOG_E_ALREADY_INITED;

	m_pid = ::GetCurrentProcessId();
	m_calldepthTlsIndex = ::TlsAlloc();
	m_accurateTime.Init();

	if (configname && configname[0])
	{
		int bufsize = _countof(m_configpath);
		lstrcpynW(m_configpath, L"Software\\Baidu\\BDLOG\\", bufsize);
		m_configpath[bufsize-1] = L'\0';

		int len = static_cast<int>(wcslen(m_configpath));
		lstrcpynW(m_configpath + len, configname, bufsize - len - 1);

		::RegCreateKeyExW(HKEY_CURRENT_USER, m_configpath, 0, NULL, 0, KEY_QUERY_VALUE|KEY_NOTIFY, NULL, &m_hConfigKey, NULL);
		if (!m_hConfigKey)
		{
			LOGWINERR(L"打开配置键值");
		}
	}

	m_inited = true;
	return S_OK;
}

HRESULT CLogController::UnInit()
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);
	CHECK_STATE;

	m_inited = false;
	
	for (size_t i = 0; i < m_odsLen; i++)
	{
		LOG(L"关闭日志设备: ", m_ods[i]->name);
		m_ods[i]->pDevice->Close();
		m_ods[i]->pDevice->DecreaseRefCounter();
		delete m_ods[i]->filter;
		delete m_ods[i];
	}
	m_odsLen = 0;

	if (m_monitorThread)
	{
		LOG(L"关闭监控线程");
		if (m_monitorThreadQuitEvent)
		{
			::SetEvent(m_monitorThreadQuitEvent);
		}
		if (::WaitForSingleObject(m_monitorThread, 5000) == WAIT_TIMEOUT)
		{
			LOG(L"监控线程等待超时");
		}
		DWORD dwExitCode;
		::GetExitCodeThread(m_monitorThread, &dwExitCode);
		::CloseHandle(m_monitorThread);
		m_monitorThread = NULL;
	}

	::TlsFree(m_calldepthTlsIndex);
	if (m_monitorThreadQuitEvent)
	{
		::CloseHandle(m_monitorThreadQuitEvent);
		m_monitorThreadQuitEvent = NULL;
	}
	if (m_hConfigKey)
	{
		::RegCloseKey(m_hConfigKey);
		m_hConfigKey = NULL;
	}

	return S_OK;
}

HRESULT CLogController::MonitorConfigChange()
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);
	CHECK_STATE;

	if (!m_configpath[0] || !m_hConfigKey)
	{
		return BDLOG_E_FUNCTION_UNAVAILBLE;
	}

	// "是否监视配置文件" 以配置文件中的设置为主
	DWORD mcc;
	DWORD mcc_len = 4;
	if (::RegQueryValueExW(m_hConfigKey, L"monitor_config_change", NULL, NULL, (LPBYTE)&mcc, &mcc_len) == ERROR_SUCCESS)
	{
		if (mcc == 0) return S_FALSE;
	}

	m_monitorThreadQuitEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_monitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, NULL);
	if (!m_monitorThread)
	{
		LOGWINERR(L"创建监视线程失败");
	}

	return S_OK;
}

HRESULT CLogController::AddOutputDevice(const wchar_t* name, LogOutputDeviceType type, const wchar_t* config)
{
	MULTI_TRHEAD_GUARD(m_csLog);
	CHECK_STATE;

	ILogOutputDevice* device = NULL;

#define DEV_ENTRY(type, className) case type: device = new className; break;
	switch (type)
	{
		DEV_ENTRY(LODT_NULL,             CLOD_Null);
		DEV_ENTRY(LODT_CONSOLE,          CLOD_Null);
		DEV_ENTRY(LODT_DEBUGOUTPUT,      CLOD_DebugOutput);
		DEV_ENTRY(LODT_PIPE,             CLOD_Pipe);
		DEV_ENTRY(LODT_SHARED_MEMORY,    CLOD_ShareMemory);
		DEV_ENTRY(LODT_FILE,             CLOD_File);
	default:break;
	}
#undef DEV_ENTRY

	return AddCustomOutputDevice(name, device, config);
}

HRESULT CLogController::AddCustomOutputDevice(const wchar_t* name, ILogOutputDevice* device, const wchar_t* config)
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);
	CHECK_STATE;

	for (size_t i = 0; i < m_odsLen; i++)
	{
		if (wcscmp(m_ods[i]->name, name) == 0)
		{
			return BDLOG_E_OUTPUT_DEVICE_ALREADY_EXIST;
		}
	}
	if (m_odsLen >= _countof(m_ods))
	{
		return BDLOG_E_OUTPUT_DEVICE_FULL;
	}

	OutputDevice* od = new OutputDevice;
	wcsncpy_s(od->name, name, _TRUNCATE);
	od->pDevice = device;
	od->defaultOption.SetOptionString(config);
	od->enabled = false;	
	od->filter = NULL;
	od->pDevice->IncreaseRefCounter();
	wchar_t overlayConfig[1024];
	GetLogOutputDeviceOverlayConfig(name, overlayConfig, _countof(overlayConfig));
	ChangeOutputDeviceOverlayConfig(od, overlayConfig);

	m_ods[m_odsLen++] = od;

	return S_OK;
}

HRESULT CLogController::RemoveOutputDevice(const wchar_t* name)
{
	MULTI_TRHEAD_GUARD(m_csLog);
	CHECK_STATE;

	HRESULT ret = BDLOG_E_OUTPUT_DEVICE_NOT_FOUND;

	for (size_t i = 0; i < m_odsLen; )
	{
		if (!name || wcscmp(m_ods[i]->name, name) == 0)
		{
			ret = S_OK;
			LOG(L"关闭日志设备: ", m_ods[i]->name);
			m_ods[i]->pDevice->Close();
			m_ods[i]->pDevice->DecreaseRefCounter();
			delete m_ods[i]->filter;
			delete m_ods[i];
			for (size_t j = i; j < --m_odsLen; j++) m_ods[j] = m_ods[j+1];
		}
		else
		{
			i++;
		}
	}

	return ret;
}

HRESULT CLogController::IncreaseCallDepth()
{
	CHECK_STATE;

	LPVOID data = TlsGetValue(m_calldepthTlsIndex);
	++reinterpret_cast<INT_PTR&>(data);
	TlsSetValue(m_calldepthTlsIndex, data);

	return S_OK;
}

HRESULT CLogController::DecreaseCallDepth()
{
	CHECK_STATE;

	LPVOID data = TlsGetValue(m_calldepthTlsIndex);
	--reinterpret_cast<INT_PTR&>(data);
	TlsSetValue(m_calldepthTlsIndex, data);

	return S_OK;
}

HRESULT CLogController::SetLogUserContext(const wchar_t* prefix)
{
	MULTI_TRHEAD_GUARD(m_csLog);
	CHECK_STATE;

	wcsncpy_s(m_userContext, prefix, _TRUNCATE);
	
	return S_OK;
}

BOOL CLogController::NeedLog(LogLevel level, const wchar_t* tag)
{
	if (!m_constructed || m_destructed || !m_inited) return FALSE;
	MULTI_TRHEAD_GUARD(m_csLog);
	if (!m_constructed || m_destructed || !m_inited) return FALSE;

	LogItem item = {};
	item.level = level;
	item.tag = tag;
	BOOL needlog = false;
	for (size_t i = 0; i < m_odsLen; i++)
	{
		if (m_ods[i]->enabled && (!m_ods[i]->filter || m_ods[i]->filter->Meet(&item)))
		{
			needlog = true;
			break;
		}
	}
	return needlog;
}

HRESULT CLogController::Log(LogLevel level, const wchar_t* tag, const wchar_t* content)
{
	MULTI_TRHEAD_GUARD(m_csLog);
	CHECK_STATE;

	LogItem item;
	AccurateTime t = m_accurateTime.GetTime();
	item.id = m_logID;
	item.unixTime = t.unix_time;
	item.microSecond = t.micro_second;
	item.level = level;
	item.tag = tag;
	item.content = content;
	item.tid = ::GetCurrentThreadId();
	item.pid = m_pid;
	item.depth = reinterpret_cast<INT_PTR>(TlsGetValue(m_calldepthTlsIndex));
	item.userContext = m_userContext;

	for (size_t i = 0; i < m_odsLen; i++)
	{
		if (!m_ods[i]->enabled) continue;

		if (m_ods[i]->filter && !m_ods[i]->filter->Meet(&item))
		{
			continue;
		}

		m_ods[i]->pDevice->Write(&item);
	}

	m_logID++;

	return S_OK;
}

HRESULT CLogController::ChangeOutputDeviceConfig(const wchar_t* name, const wchar_t* config)
{
	MULTI_TRHEAD_GUARD(m_csLog);
	CHECK_STATE;

	for (size_t i = 0; i < m_odsLen; i++)
	{
		if (wcscmp(m_ods[i]->name, name) == 0)
		{
			return ChangeOutputDeviceOverlayConfig(m_ods[i], config);
		}
	}

	return BDLOG_E_OUTPUT_DEVICE_NOT_FOUND;
}

HRESULT CLogController::GetLogOutputDeviceOverlayConfig(const wchar_t* name, wchar_t* buffer, size_t len)
{
	buffer[0] = 0;

	if (m_hConfigKey)
	{
		WCHAR key[256] = L"ld_";
		wcsncat_s(key, name, _TRUNCATE);
		DWORD readlen = len;
		::RegQueryValueExW(m_hConfigKey, key, NULL, NULL, (LPBYTE)buffer, &readlen);
	}

	return S_OK;
}

unsigned int CLogController::MonitorThread(void* pParam)
{
	// 监视目录的辅助类
	class CEventDirMonitor
	{
	public:
		CEventDirMonitor(HKEY hkey): m_hKey(hkey)
		{
			m_evt = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			ReadChange();
		}
		~CEventDirMonitor()
		{
			::CloseHandle(m_evt);
		}
		operator HANDLE()
		{
			return m_evt;
		}
		void ReadChange()
		{
			::RegNotifyChangeKeyValue(m_hKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET, m_evt, TRUE);
		}
	private:
		HANDLE m_evt;
		HKEY m_hKey;
	};

	LOG(L"监视线程已启动");
	CLogController* pController = static_cast<CLogController*>(pParam);
	if (!pController)
	{
		LOG(L"参数不正确，监视线程异常退出");
		return 1;
	}

	HANDLE quitEvt = pController->m_monitorThreadQuitEvent;
	CEventDirMonitor m(pController->m_hConfigKey);
	HANDLE evts[] = {m, quitEvt};

	for (;;)
	{
		DWORD dwRet = ::WaitForMultipleObjects(_countof(evts), evts, FALSE, INFINITE);
		if (dwRet == WAIT_FAILED)
		{
			// 等待失败，不等了以免死循环
			LOG(L"等待事件失败，监视线程退出");
			break;
		}
		else if (dwRet == WAIT_OBJECT_0 + 1 || dwRet == WAIT_ABANDONED_0 + 1)
		{
			LOG(L"收到退出信号，监视线程退出");
			break;
		}
		else if (dwRet == WAIT_OBJECT_0 + 0 || dwRet == WAIT_ABANDONED_0 + 0)
		{
			LOG(L"监视目录内容变化");
			pController->OnConfigFileChange();
			m.ReadChange();
		}
	}

	return 0;
}

HRESULT CLogController::OnConfigFileChange()
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);
	CHECK_STATE;

	wchar_t overlayConfig[1024];
	for (size_t i = 0; i < m_odsLen; i++)
	{
		GetLogOutputDeviceOverlayConfig(m_ods[i]->name, overlayConfig, _countof(overlayConfig));
		if (wcscmp(overlayConfig, m_ods[i]->overlayConfig) != 0)
		{
			ChangeOutputDeviceOverlayConfig(m_ods[i], overlayConfig);
		}
	}

	return S_OK;
}


HRESULT CLogController::ChangeOutputDeviceOverlayConfig(OutputDevice* device, const wchar_t* config)
{
	CLogOption overlayOption;
	wcsncpy_s(device->overlayConfig, config, _TRUNCATE);
	overlayOption.SetOptionString(config);
	overlayOption.Append(device->defaultOption);

	bool enabled = device->enabled;
	device->enabled = overlayOption.GetOptionAsBool(L"enable", device->enabled);

	if (enabled != device->enabled)
	{
		if (device->enabled == true)
		{
			device->pDevice->Open(&overlayOption);
		}
		else
		{
			device->pDevice->Close();
		}
	}
	else if (device->enabled)
	{
		device->pDevice->OnConfigChange(&overlayOption);
	}

	CLogFilterCreator c;
	const wchar_t* filterstr = overlayOption.GetOption(L"filter", NULL);
	if (filterstr)
	{
		CLogFilter* filter = c.CreateFilter(filterstr, wcslen(filterstr));
		delete device->filter;
		device->filter = filter;
	}

	return S_OK;
}
