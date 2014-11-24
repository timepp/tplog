#include "stdafx.h"
#include "logcenter.h"
#include "helper.h"
#include <algorithm>
#include "resource.h"
#include "config.h"
#include "servicehelper.h"

class CAutoCriticalSection
{
	CCriticalSection& m_cs;
public:
	CAutoCriticalSection(CCriticalSection& cs) : m_cs(cs)
	{
		m_cs.Enter();
	}
	~CAutoCriticalSection()
	{
		m_cs.Leave();
	}
};

CLogCenter::CLogCenter()
{
	m_logID = 1;
	m_timerID = 0;
	m_buffer.reserve(1000);
	m_logDB.reserve(100000);
	m_autoEnablePipeDeviceReg = false;
    m_globalDbgOutputReader.set_global_mode(true);
}

CLogCenter::~CLogCenter()
{

}

void CLogCenter::Init()
{

}

void CLogCenter::ConnectToSource(LogSource source, LPCWSTR param)
{
    DisconnectFromSource(source);

    switch (source)
    {
    case LogSource::File:
        m_logFileReader.setpath(param);
        break;
    case LogSource::ShareMemory:
        m_logShareMemoryReader.setsmname(param);
        break;
    default:
        break;
    }

    tplog::log_reader * reader = GetLogReaderObject(source);
    reader->set_listener(this);
    if (SUCCEEDED(reader->start()))
    {
        CAutoCriticalSection cs(m_csListener);
        for (ListenerList::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
        {
            (*it)->OnConnect(source);
        }
    }

    SyncTimer();
}

void CLogCenter::DisconnectFromSource(LogSource source)
{
    tplog::log_reader * reader = GetLogReaderObject(source);
    reader->set_listener(nullptr);
    if (SUCCEEDED(reader->stop()))
    {
        CAutoCriticalSection cs(m_csListener);
        for (ListenerList::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
        {
            (*it)->OnConnect(source);
        }
    }

    SyncTimer();
}

bool CLogCenter::MonitoringSource(LogSource source) const
{
    tplog::log_reader * reader = const_cast<CLogCenter*>(this)->GetLogReaderObject(source);
    return reader->working();
}

tplog::log_reader * CLogCenter::GetLogReaderObject(LogSource source)
{
    switch (source)
    {
    case LogSource::Pipe:              return &m_logPipeReader;
    case LogSource::File:              return &m_logFileReader;
    case LogSource::DebugOutput:       return &m_dbgOutputReader;
    case LogSource::GlobalDebugOutput: return &m_globalDbgOutputReader;
    case LogSource::ShareMemory:       return &m_logShareMemoryReader;
    }
    return nullptr;
}

void CLogCenter::SyncTimer()
{
    if (MonitoringSource(LogSource::Pipe) ||
        MonitoringSource(LogSource::File) ||
        MonitoringSource(LogSource::DebugOutput) ||
        MonitoringSource(LogSource::GlobalDebugOutput) ||
        MonitoringSource(LogSource::ShareMemory))
    {
        if (m_timerID == 0)
        {
            m_timerID = SetTimer(NULL, 0, 500, &CLogCenter::TimerProc);
        }
    }
    else
    {
        if (m_timerID != 0)
        {
            if (KillTimer(NULL, m_timerID))
            {
                m_timerID = 0;
            }
        }
    }
}

void CLogCenter::AddListener(CLogCenterListener* pListener)
{
	CAutoCriticalSection cs(m_csListener);

	ListenerList::const_iterator it = std::find(m_listeners.begin(), m_listeners.end(), pListener);
	if (it == m_listeners.end())
	{
		m_listeners.push_back(pListener);
	}
}

void CLogCenter::RemoveListener(CLogCenterListener* pListener)
{
	CAutoCriticalSection cs(m_csListener);

	ListenerList::iterator it = std::find(m_listeners.begin(), m_listeners.end(), pListener);
	if (it != m_listeners.end())
	{
		m_listeners.erase(it);
	}
}

void CLogCenter::on_new_log(tplog::logitem* li)
{
	CAutoCriticalSection cs(m_csLogBuffer);

	m_buffer.push_back(li);
}


void CLogCenter::on_notify(int notifyid, HRESULT hr)
{
	m_notifyTimerID = ::SetTimer(NULL, 0, 100, &CLogCenter::NotifyProc);
}

void CLogCenter::MergeBuffer()
{
	if (m_csLogDB.TryEnter())
	{
		ON_LEAVE_1(m_csLogDB.Leave(), CCriticalSection&, m_csLogDB);

		LogRange newrange;
		newrange.idmin = m_logID;

		{
			CAutoCriticalSection cs(m_csLogBuffer);
			for (LogBuffer::const_iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
			{
				LogInfo* li = new LogInfo;
				li->logid = m_logID++;
				li->item = *it;
				li->occupytime = 0;
				m_logDB.push_back(li);
			}
			m_buffer.clear();
		}

		newrange.idmax = m_logID;

		// update occupytime
		for (UINT64 i = newrange.idmin; i < newrange.idmax; i++)
		{
			LogInfo* li = GetLog(i);
			std::map<tplog::uint32_t, UINT64>::iterator it = m_lastLogInSameThread.find(li->item->log_tid);
			if (it == m_lastLogInSameThread.end())
			{
				m_lastLogInSameThread[li->item->log_tid] = i;
			}
			else
			{
				LogInfo* lastli = GetLog(it->second);
				lastli->occupytime = helper::GetElapsedTime(*lastli, *li);
				it->second = i;
			}
		}
		
		if (newrange.idmin < newrange.idmax)
		{
			CAutoCriticalSection cs(m_csListener);
			for (ListenerList::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
			{
				(*it)->OnNewLog(newrange);
			}
		}
	}
}

VOID CLogCenter::TimerProc( __in HWND hwnd, __in UINT uMsg, __in UINT_PTR idEvent, __in DWORD dwTime )
{
	ServiceHelper::GetLogCenter()->MergeBuffer();
}
VOID CLogCenter::NotifyProc( __in HWND hwnd, __in UINT uMsg, __in UINT_PTR idEvent, __in DWORD dwTime )
{
	ServiceHelper::GetLogCenter()->ShowNotifyMsg();
}

LogRange CLogCenter::GetLogRange()
{
	CAutoCriticalSection cs(m_csLogDB);

	LogRange range;
	if (!m_logDB.empty())
	{
		range.idmin = m_logDB.front()->logid;
		range.idmax = m_logDB.back()->logid + 1;
	}
	else
	{
		range.idmax = range.idmin = 0;
	}

	return range;
}

LogInfo* CLogCenter::GetLog(UINT64 logid)
{
	CAutoCriticalSection cs(m_csLogDB);

	if (m_logDB.empty())
	{
		return NULL;
	}

	UINT64 firstid = m_logDB.front()->logid;
	if (firstid > logid)
	{
		return NULL;
	}

	size_t index = static_cast<size_t>(logid - firstid);
	return m_logDB[index];
}

void CLogCenter::LockLog()
{
	m_csLogDB.Enter();
}

void CLogCenter::UnlockLog()
{
	m_csLogDB.Leave();
}


bool CLogCenter::EnablePipeDeviceReg(bool bEnable)
{
	CStringW desiredEnableString = bEnable? L"enable:true" : L"enable:false";

	HKEY hKey = NULL;
	::RegCreateKeyExW(HKEY_CURRENT_USER, m_cfgPathReg, 0, NULL, 0, KEY_QUERY_VALUE|KEY_SET_VALUE, NULL, &hKey, NULL);
	if (!hKey)
	{
		helper::GUIReportError(IDS(IDS_MODIFY_LOG_DEVICE_CONFIG_FROM_REGISTRY));
		return false;
	}

	WCHAR buffer[4096] = {0};
	DWORD buflen = sizeof(buffer);
	::RegQueryValueExW(hKey, L"ld_pipe", NULL, NULL, (LPBYTE)buffer, &buflen);

	CStringW bufR = buffer;
	CStringW bufW = buffer;
	bufR.MakeLower();
	int pos = bufR.Find(L"enable:");
	if (pos == -1)
	{
		bufW = desiredEnableString + L" " + bufW;
	}
	else
	{
		int pos2 = bufR.Find(L' ', pos);
		if (pos2 == -1) pos2 = bufR.GetLength();
		bufW = bufW.Mid(0, pos) + desiredEnableString + bufW.Mid(pos2);
	}

	if (bufW.Compare(buffer) != 0)
	{
		DWORD cbData = static_cast<DWORD>(bufW.GetLength() * 2 + 2);
		::RegSetValueExW(hKey, L"ld_pipe", 0, REG_SZ, (const BYTE*)(LPCWSTR)bufW, cbData);
		::RegCloseKey(hKey);
		return true;
	}

	::RegCloseKey(hKey);
	return false;
}

size_t CLogCenter::source_count() const
{
	if (m_logPipeReader.working())
	{
		return m_logPipeReader.source_count();
	}
	else
	{
		return m_logFileReader.source_count();
	}
}

tplog::lsi_vec_t CLogCenter::get_sources() const
{
	if (m_logPipeReader.working())
	{
		return m_logPipeReader.get_sources();
	}
	else
	{
		return m_logFileReader.get_sources();
	}
}

void CLogCenter::ClearAllLog()
{
	CAutoCriticalSection cs(m_csLogDB);

	m_lastLogInSameThread.clear();
	ClearOldLog((UINT64)-1);
	m_logID = 1;
}

void CLogCenter::ClearOldLog(UINT64 logid)
{
	CAutoCriticalSection cs(m_csLogDB);

	LogDB::iterator it;
	for (it = m_logDB.begin(); it != m_logDB.end() && (*it)->logid < logid; ++it)
	{
		delete (*it)->item;
		delete *it;
	}
	m_logDB.erase(m_logDB.begin(), it);
}

void CLogCenter::ShowNotifyMsg()
{
	::KillTimer(NULL, m_notifyTimerID);
	
	::MessageBoxW(NULL, L"a", L"b", MB_OK);
}
