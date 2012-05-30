#include "stdafx.h"
#include "outputdevice_sharememory.h"

CLOD_ShareMemory::CLOD_ShareMemory()
{
	m_smName[0] = L'\0';
}

void CLOD_ShareMemory::GetShareMemoryName(ILogOption* opt, wchar_t* name, size_t len)
{
	const wchar_t* namePrefix = opt->GetOption(L"nameprefix", L"BDXLOG_SHAREMEMORY_V1.0_");
	wcsncpy_s(name, len, namePrefix, _TRUNCATE);
	wcsncat_s(name, len, tostr(GetCurrentProcessId()), _TRUNCATE);
}

HRESULT CLOD_ShareMemory::Open(ILogOption* opt)
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

HRESULT CLOD_ShareMemory::Close()
{
	m_shareMemory.Close();
	m_smName[0] = L'\0';

	return S_OK;
}

HRESULT CLOD_ShareMemory::Flush()
{
	return S_OK;
}

HRESULT CLOD_ShareMemory::Write(const LogItem* item)
{
	if (!m_shareMemory.IsOpen())
	{
		return BDLOG_E_DEVICE_NOT_READY;
	}

	MemLogQuene& q = m_shareMemory.GetDataAs<MemLogQuene>();
	MemLogQuene::MemLogRecord& record = q.record[q.rear];
	q.rear = (q.rear + 1) % _countof(q.record);
	if (q.rear == q.head)
	{
		q.head = (q.head + 1) % _countof(q.record);
	}
	record.unixTime = item->unixTime;
	record.milliSecond = item->microSecond / 1000;
	record.tid = item->tid;
	wcsncpy_s(record.content, item->content, _TRUNCATE);

	return S_OK;
}

HRESULT CLOD_ShareMemory::OnConfigChange(ILogOption* opt)
{
	wchar_t newname[256];
	GetShareMemoryName(opt, newname, _countof(newname));
	if (wcscmp(m_smName, newname) != 0)
	{
		wcsncpy_s(m_smName, newname, _TRUNCATE);
		m_shareMemory.Close();
		m_shareMemory.Open(m_smName, sizeof(MemLogQuene), FALSE);
	}

	return S_OK;
}
