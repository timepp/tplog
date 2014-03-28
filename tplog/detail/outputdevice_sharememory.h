#pragma once

#include "outputdevice_common.h"
#include "sharememory.h"

#pragma pack(push, 1)
struct MemLogQuene
{
	struct MemLogRecord
	{
		INT64 unixTime;
		INT32 milliSecond;
		UINT32 tid;
		WCHAR content[120];
	};

	UINT32 totalLength;                      //�����ڴ�Ĵ�С
	UINT32 recordCount;                 //��־���д�С
	UINT32 recordLength;                      //������־��С
	UINT32 head;                         //����ͷ
	UINT32 rear;                         //����β
	MemLogRecord record[1000]; //��������
};
#pragma pack(pop)

class CLOD_ShareMemory : public CLogOutputDeviceBase
{
private:
	CSharingMemory m_shareMemory;
	wchar_t m_smName[256];

public:
	virtual HRESULT Open(ILogOption* opt)
	{
		GetShareMemoryName(opt, m_smName, _countof(m_smName));
		HRESULT hr = m_shareMemory.Open(m_smName, sizeof(MemLogQuene), FALSE);
		ENSURE_SUCCEED(hr);

		MemLogQuene& q = m_shareMemory.GetDataAs<MemLogQuene>();
		q.totalLength = sizeof(q);
		q.recordCount = _countof(q.record);
		q.recordLength = sizeof(q.record[0]);
		q.head = 0;
		q.rear = 0;

		return S_OK;
	}

	virtual HRESULT Close()
	{
		m_shareMemory.Close();
		m_smName[0] = L'\0';

		return S_OK;
	}

	virtual HRESULT Write(const LogItem* item)
	{
		if (!m_shareMemory.IsOpen())
		{
			return TPLOG_E_DEVICE_NOT_READY;
		}

		MemLogQuene& q = m_shareMemory.GetDataAs<MemLogQuene>();
		MemLogQuene::MemLogRecord& record = q.record[q.rear];
		q.rear = (q.rear + 1) % _countof(q.record);
		if (q.rear == q.head)
		{
			q.head = (q.head + 1) % _countof(q.record);
		}
		record.unixTime = 0;
		record.milliSecond = 0;
		record.tid = item->tid;
		TRUNCATED_COPY(record.content, item->content);

		return S_OK;
	}

	virtual HRESULT Flush()
	{
		return S_OK;
	}

	virtual HRESULT OnConfigChange(ILogOption* opt)
	{
		wchar_t newname[256];
		GetShareMemoryName(opt, newname, _countof(newname));
		if (wcscmp(m_smName, newname) != 0)
		{
			TRUNCATED_COPY(m_smName, newname);
			m_shareMemory.Close();
			m_shareMemory.Open(m_smName, sizeof(MemLogQuene), FALSE);
		}

		return S_OK;
	}

	CLOD_ShareMemory()
	{
		m_smName[0] = L'\0';
	}

private:
	static void GetShareMemoryName(ILogOption* opt, wchar_t* name, size_t len)
	{
		const wchar_t* namePrefix = opt->GetOption(L"nameprefix", L"TPLOG_SHAREMEMORY_V1.0_");
		textstream(name, len) << namePrefix << ToStr(GetCurrentProcessId(), L"%u");
	}
};

