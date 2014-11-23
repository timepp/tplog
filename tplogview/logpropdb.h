#pragma once

#include <string>
#include <set>

#include "serviceid.h"
#include "logcenter.h"

class CLogPropertyDB : public tp::service_impl<SID_LogPropertyDB>, public CLogCenterListener
{
public:
	typedef std::set<std::wstring> TagSet;
	typedef std::set<UINT> LevelSet;
	TP_SET_DEPENDENCIES(create, );
	TP_SET_DEPENDENCIES(destroy, SID_LogCenter);

public:
	void Init()
	{
		tp::global_service<CLogCenter>()->AddListener(this);
	}

	void AddLevel(UINT32 lvl)
	{
		if (m_runtimelvlset.insert(lvl).second)
		{
			m_lvlVersion++;
		}
	}

	void AddTag(const std::wstring& tag)
	{
		for (size_t i = 0, j = 0; ; j++)
		{
			if (tag[j] == L';' || tag[j] == L'\0')
			{
				if (i < j)
				{
					if (m_tagset.insert(tag.substr(i, j-i)).second)
					{
						m_tagVersion++;
					}
				}
				if (tag[j] == L'\0') break;
				i = j+1;
			}
		}
	}

	void Clear()
	{
		m_tagset.clear();
		m_tagVersion++;

		m_runtimelvlset.clear();
		m_lvlVersion++;
	}

	const TagSet& GetTags(int* ver) const
	{
		*ver = m_tagVersion;
		return m_tagset;
	}

	const LevelSet& GetRuntimeLevels(int* ver) const
	{
		*ver = m_lvlVersion;
		return m_runtimelvlset;
	}

	const LevelSet& GetDefaultLevels() const
	{
		return m_defaultlvlset;
	}

	static CStringW GetLevelDesc(UINT level)
	{
		switch (level)
		{
		case 0x05: return IDS(IDS_LL_DIAGNOSE);
		case 0x10: return IDS(IDS_LL_DEBUG);
		case 0x20: return IDS(IDS_LL_INFORMATION);
		case 0x25: return IDS(IDS_LL_EVENT);
		case 0x30: return IDS(IDS_LL_WARNING);
		case 0x40: return IDS(IDS_LL_ERROR);
		default: break;
		}
        CStringW strDesc;
        strDesc.Format(L"0x%x", level);
		return strDesc;
	}

	CLogPropertyDB() : m_tagVersion(1), m_lvlVersion(1)
	{
		m_defaultlvlset.insert(0x05);
		m_defaultlvlset.insert(0x10);
		m_defaultlvlset.insert(0x20);
		m_defaultlvlset.insert(0x25);
		m_defaultlvlset.insert(0x30);
		m_defaultlvlset.insert(0x40);
	}

	~CLogPropertyDB()
	{
		tp::global_service<CLogCenter>()->RemoveListener(this);
	}

	static CLogPropertyDB& Instance()
	{
		return *SERVICE(CLogPropertyDB);
	}

	virtual void OnNewLog(const LogRange& range)
	{
		for (UINT64 i = range.idmin; i < range.idmax; i++)
		{
			const LogInfo* li = SERVICE(CLogCenter)->GetLog(i);
			AddTag(li->item->log_tags);
			AddLevel(li->item->log_class);
		}
	}

	virtual void OnConnect()
	{
		Clear();
	}

	virtual void OnDisconnect()
	{
	}

private:
	TagSet m_tagset;
	int m_tagVersion;

	LevelSet m_defaultlvlset;
	LevelSet m_runtimelvlset;
	int m_lvlVersion;
};

TP_DEFINE_GLOBAL_SERVICE(CLogPropertyDB, L"Log Property Manager");
