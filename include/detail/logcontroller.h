#pragma once

#include "../tplog.h"
#include "../tplog_synctime.h"

#include "logutil.h"
#include "logfilter.h"
#include "logoption.h"

#include "outputdevice_debugoutput.h"
#include "outputdevice_pipe.h"
#include "outputdevice_sharememory.h"
#include "outputdevice_file.h"

#define TPLOG_CONTROLLER_CHECK_STATE \
	if (!m_constructed) return TPLOG_E_BEFORE_CONSTRUCT; \
	if (m_destructed) return TPLOG_E_DESTRUCTED; \
	if (!m_inited) return TPLOG_E_NOT_INITED;

#define TPLOG_CONTROLLER_ENSURE_STATE_RETVAL(v) if (!m_constructed || m_destructed || !m_inited) return v


class CLogController : public ILogController
{
public:
	CLogController();
	virtual ~CLogController();

public:
	virtual HRESULT Init(const wchar_t* configpath);
	virtual HRESULT UnInit();
	virtual HRESULT MonitorConfigChange();
	virtual BOOL NeedLog(LogLevel level, const wchar_t* tag);
	virtual HRESULT Log(LogLevel level, const wchar_t* tag, const wchar_t* content);
	virtual HRESULT AddOutputDevice(const wchar_t* name, LogOutputDeviceType type, const wchar_t* config);
	virtual HRESULT AddCustomOutputDevice(const wchar_t* name, ILogOutputDevice* device, const wchar_t* config);
	virtual HRESULT RemoveOutputDevice(const wchar_t* name);
	virtual HRESULT ChangeOutputDeviceConfig(const wchar_t* name, const wchar_t* config);
	virtual HRESULT IncreaseCallDepth();
	virtual HRESULT DecreaseCallDepth();

	/** logcontroller��״̬
	 *  ��Ϊlogcontroller��ȫ�ֶ��������ڶ���δ����ʱ��Щ������ֵ����false��ǡ���ܹ�����Ӧ����˼
	 */
	bool m_inited;      // logcontroller�����Ƿ��ѳ�ʼ��(����ʹ���䷽��)
	bool m_constructed; // logcontroller�����Ƿ��ѹ���
	bool m_destructed;	// logcontroller�����Ƿ�������

private: // types

	struct OutputDevice
	{
		ILogOutputDevice* pDevice;
		wchar_t name[32];
		CLogOption defaultOption;
		wchar_t overlayConfig[1024];
		bool enabled;                             // �Ƿ�����
		CLogFilter* filter;                       // ������
	};
	
private:

	cslock m_csLog;
	HANDLE m_monitorThreadQuitEvent;
	HANDLE m_monitorThread;

	OutputDevice* m_ods[32];
	size_t m_odsLen;

	HKEY m_hConfigKey;

	unsigned __int64 m_logID;
	unsigned int m_pid;
	CLogAccurateTime m_accurateTime;
	DWORD m_calldepthTlsIndex;

private:
	HRESULT GetLogOutputDeviceOverlayConfig(const wchar_t* name, wchar_t* buffer, size_t len);
	HRESULT ChangeOutputDeviceOverlayConfig(OutputDevice* device, const wchar_t* config);
	HRESULT OnConfigFileChange();
	BOOL CanMonitorConfig();

	// �����߳�
	static DWORD WINAPI MonitorThread(void* pParam);
};



inline CLogController::CLogController()
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

	m_constructed = true;
}

inline CLogController::~CLogController()
{
	LOGFUNC;
	UnInit();
	m_destructed = true;
}

inline HRESULT CLogController::Init(const wchar_t* configpath)
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);

	if (!m_constructed) return TPLOG_E_BEFORE_CONSTRUCT;
	if (m_destructed) return TPLOG_E_DESTRUCTED;
	if (m_inited) return TPLOG_E_ALREADY_INITED;

	m_pid = ::GetCurrentProcessId();
	m_calldepthTlsIndex = ::TlsAlloc();
	m_accurateTime.Init();

	if (configpath && configpath[0])
	{
		::RegCreateKeyExW(HKEY_CURRENT_USER, configpath, 0, NULL, 0, KEY_QUERY_VALUE|KEY_NOTIFY, NULL, &m_hConfigKey, NULL);
		if (!m_hConfigKey)
		{
			LOGWINERR(L"�����ü�ֵ");
		}
	}

	m_inited = true;
	return S_OK;
}

inline HRESULT CLogController::UnInit()
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);
	TPLOG_CONTROLLER_CHECK_STATE;

	m_inited = false;
	
	for (size_t i = 0; i < m_odsLen; i++)
	{
		LOG(L"�ر���־�豸: ", m_ods[i]->name);
		m_ods[i]->pDevice->Close();
		m_ods[i]->pDevice->DecreaseRefCounter();
		delete m_ods[i]->filter;
		delete m_ods[i];
	}
	m_odsLen = 0;

	if (m_monitorThread)
	{
		LOG(L"�رռ���߳�");
		if (m_monitorThreadQuitEvent)
		{
			::SetEvent(m_monitorThreadQuitEvent);
		}
		if (::WaitForSingleObject(m_monitorThread, 5000) == WAIT_TIMEOUT)
		{
			LOG(L"����̵߳ȴ���ʱ");
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

inline HRESULT CLogController::MonitorConfigChange()
{
	LOGFUNC;

	if (!CanMonitorConfig())
	{
		return TPLOG_E_FUNCTION_UNAVAILBLE;
	}
	
	m_monitorThreadQuitEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_monitorThread = ::CreateThread(NULL, 0, MonitorThread, this, 0, NULL);
	if (!m_monitorThread)
	{
		LOGWINERR(L"���������߳�ʧ��");
	}

	return S_OK;
}

inline HRESULT CLogController::AddOutputDevice(const wchar_t* name, LogOutputDeviceType type, const wchar_t* config)
{
	MULTI_TRHEAD_GUARD(m_csLog);
	TPLOG_CONTROLLER_CHECK_STATE;

	ILogOutputDevice* device = NULL;

	switch (type)
	{
	case LODT_NULL:           device = new CLOD_Null;            break;
	case LODT_DEBUGOUTPUT:    device = new CLOD_DebugOutput;     break;
	case LODT_SHARED_MEMORY:  device = new CLOD_ShareMemory;     break;
	case LODT_FILE:           device = new CLOD_File;            break;
	case LODT_PIPE:           device = new CLOD_Pipe;            break;
	case LODT_CONSOLE:        return E_NOTIMPL;

	default:                  return E_INVALIDARG;
	}

	return AddCustomOutputDevice(name, device, config);
}

inline HRESULT CLogController::AddCustomOutputDevice(const wchar_t* name, ILogOutputDevice* device, const wchar_t* config)
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);
	TPLOG_CONTROLLER_CHECK_STATE;

	for (size_t i = 0; i < m_odsLen; i++)
	{
		if (wcscmp(m_ods[i]->name, name) == 0)
		{
			return TPLOG_E_OUTPUT_DEVICE_ALREADY_EXIST;
		}
	}
	if (m_odsLen >= _countof(m_ods))
	{
		return TPLOG_E_OUTPUT_DEVICE_FULL;
	}

	OutputDevice* od = new OutputDevice;
	textstream(od->name, _countof(od->name)) << name;
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

inline HRESULT CLogController::RemoveOutputDevice(const wchar_t* name)
{
	MULTI_TRHEAD_GUARD(m_csLog);
	TPLOG_CONTROLLER_CHECK_STATE;

	HRESULT ret = TPLOG_E_OUTPUT_DEVICE_NOT_FOUND;

	for (size_t i = 0; i < m_odsLen; )
	{
		if (!name || wcscmp(m_ods[i]->name, name) == 0)
		{
			ret = S_OK;
			LOG(L"�ر���־�豸: ", m_ods[i]->name);
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

inline HRESULT CLogController::IncreaseCallDepth()
{
	TPLOG_CONTROLLER_CHECK_STATE;

	LPVOID data = TlsGetValue(m_calldepthTlsIndex);
	++reinterpret_cast<INT_PTR&>(data);
	TlsSetValue(m_calldepthTlsIndex, data);

	return S_OK;
}

inline HRESULT CLogController::DecreaseCallDepth()
{
	TPLOG_CONTROLLER_CHECK_STATE;

	LPVOID data = TlsGetValue(m_calldepthTlsIndex);
	--reinterpret_cast<INT_PTR&>(data);
	TlsSetValue(m_calldepthTlsIndex, data);

	return S_OK;
}

inline BOOL CLogController::NeedLog(LogLevel level, const wchar_t* tag)
{
	TPLOG_CONTROLLER_ENSURE_STATE_RETVAL(FALSE);
	MULTI_TRHEAD_GUARD(m_csLog);
	TPLOG_CONTROLLER_ENSURE_STATE_RETVAL(FALSE);

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

inline HRESULT CLogController::Log(LogLevel level, const wchar_t* tag, const wchar_t* content)
{
	MULTI_TRHEAD_GUARD(m_csLog);
	TPLOG_CONTROLLER_CHECK_STATE;

	LogItem item;
	FILETIME t = m_accurateTime.GetTime();
	item.id = m_logID++;
	item.time = t;
	item.level = level;
	item.tag = tag;
	item.content = content;
	item.tid = ::GetCurrentThreadId();
	item.pid = m_pid;
	item.depth = (DWORD)TlsGetValue(m_calldepthTlsIndex);

	for (size_t i = 0; i < m_odsLen; i++)
	{
		if (!m_ods[i]->enabled) continue;

		if (m_ods[i]->filter && !m_ods[i]->filter->Meet(&item))
		{
			continue;
		}

		m_ods[i]->pDevice->Write(&item);
	}

	return S_OK;
}

inline HRESULT CLogController::ChangeOutputDeviceConfig(const wchar_t* name, const wchar_t* config)
{
	MULTI_TRHEAD_GUARD(m_csLog);
	TPLOG_CONTROLLER_CHECK_STATE;

	for (size_t i = 0; i < m_odsLen; i++)
	{
		if (wcscmp(m_ods[i]->name, name) == 0)
		{
			return ChangeOutputDeviceOverlayConfig(m_ods[i], config);
		}
	}

	return TPLOG_E_OUTPUT_DEVICE_NOT_FOUND;
}

inline HRESULT CLogController::GetLogOutputDeviceOverlayConfig(const wchar_t* name, wchar_t* buffer, size_t len)
{
	buffer[0] = 0;

	if (m_hConfigKey)
	{
		WCHAR key[256];
		textstream(key, _countof(key)) << L"ld_" << name;
		DWORD readlen = len;
		::RegQueryValueExW(m_hConfigKey, key, NULL, NULL, (LPBYTE)buffer, &readlen);
	}

	return S_OK;
}

inline DWORD WINAPI CLogController::MonitorThread(void* pParam)
{
	// ����Ŀ¼�ĸ�����
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

	LOG(L"�����߳�����");
	CLogController* pController = static_cast<CLogController*>(pParam);
	if (!pController)
	{
		LOG(L"��������ȷ�������߳��쳣�˳�");
		return 1;
	}

	HANDLE quitEvt = pController->m_monitorThreadQuitEvent;
	CEventDirMonitor m(pController->m_hConfigKey);
	HANDLE evts[2] = {m, quitEvt};

	for (;;)
	{
		DWORD dwRet = ::WaitForMultipleObjects(_countof(evts), evts, FALSE, INFINITE);
		if (dwRet == WAIT_FAILED)
		{
			// �ȴ�ʧ�ܣ�������������ѭ��
			LOG(L"�ȴ��¼�ʧ�ܣ������߳��˳�");
			break;
		}
		else if (dwRet == WAIT_OBJECT_0 + 1 || dwRet == WAIT_ABANDONED_0 + 1)
		{
			LOG(L"�յ��˳��źţ������߳��˳�");
			break;
		}
		else if (dwRet == WAIT_OBJECT_0 + 0 || dwRet == WAIT_ABANDONED_0 + 0)
		{
			LOG(L"����Ŀ¼���ݱ仯");
			pController->OnConfigFileChange();
			m.ReadChange();
		}
	}

	return 0;
}

inline HRESULT CLogController::OnConfigFileChange()
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);
	TPLOG_CONTROLLER_CHECK_STATE;

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


inline HRESULT CLogController::ChangeOutputDeviceOverlayConfig(OutputDevice* device, const wchar_t* config)
{
	CLogOption overlayOption;
	TRUNCATED_COPY(device->overlayConfig, config);
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

inline BOOL CLogController::CanMonitorConfig()
{
	LOGFUNC;
	MULTI_TRHEAD_GUARD(m_csLog);
	TPLOG_CONTROLLER_ENSURE_STATE_RETVAL(FALSE);

	if (!m_hConfigKey)
	{
		return FALSE;
	}

	DWORD mcc;
	DWORD mcc_len = 4;
	if (::RegQueryValueExW(m_hConfigKey, L"monitor_config_change", NULL, NULL, (LPBYTE)&mcc, &mcc_len) == ERROR_SUCCESS)
	{
		// If there exists a value named "monitor_config_change", and it's value is 0, we disable the monitor.
		if (mcc == 0) return FALSE;
	}

	return TRUE;
}
