#include "stdafx.h"
#include "helper.h"
#include "filter.h"
#include "concretefilter.h"
#include "logpropdb.h"


#define SAVE_UNSTABLE_FILTER

namespace
{
	template <typename T> int value_compare(const T& v1, const T& v2)
	{
		if (v1 < v2) return -1;
		if (v2 < v1) return 1;
		return 0;
	}

	int value_compare(bool v1, bool v2)
	{
		if (v1 == v2) return 0;
		if (!v1) return -1;
		return 1;
	}
	int value_compare(const std::wstring& v1, const std::wstring& v2)
	{
		return v1.compare(v2);
	}

	template <typename A, typename B>
	int value_compare(const A& a1, const A& a2, const B& b1, const B& b2)
	{
		int a = value_compare(a1, a2); if (a) return a;
		int b = value_compare(b1, b2); if (b) return b;
		return 0;
	}

	template <typename A, typename B, typename C>
	int value_compare(const A& a1, const A& a2, const B& b1, const B& b2, const C& c1, const C& c2)
	{
		int a = value_compare(a1, a2); if (a) return a;
		int b = value_compare(b1, b2); if (b) return b;
		int c = value_compare(c1, c2); if (c) return c;
		return 0;
	}
}


std::wstring logclass_filter::name() const
{
	return (LPCWSTR)IDS(IDS_FILTER_RULE_LOG_LEVEL);
}
std::wstring logclass_filter::desc() const
{
	if (m_class_low == 0 && m_class_high == 0)
	{
        return IDS2(IDS_ALL_LOG_LEVEL);
	}
	else if (m_class_low == m_class_high)
	{
		return std::wstring(IDS2(IDS_LOG_LEVEL)) + L" = " + quote(helper::GetLogLevelDescription(m_class_high));
	}
	else if (m_class_low == 0)
	{
		return std::wstring(IDS2(IDS_LOG_LEVEL)) + L" <= " + quote(helper::GetLogLevelDescription(m_class_high));
	}
	else if (m_class_high == 0)
	{
		return std::wstring(IDS(IDS_LOG_LEVEL)) + L" >= " + quote(helper::GetLogLevelDescription(m_class_low));
	}
	else
	{
		return
			std::wstring(IDS2(IDS_LOG_LEVEL)) +
            IDSFMT2(IDS_BETWEEN_s_AND_s, quote(helper::GetLogLevelDescription(m_class_low)).c_str(), quote(helper::GetLogLevelDescription(m_class_high)).c_str());
	}
}
component* logclass_filter::clone(bool) const
{
	return new logclass_filter(m_class_low, m_class_high);
}
bool logclass_filter::meet(const LogInfo& log) const
{
	return log.item->log_class >= m_class_low && (m_class_high == 0 || log.item->log_class <= m_class_high);
}

bool logclass_filter::load(component_creator* /*cc*/, serializer* s)
{
	swscanf_s(s->get_property(L"class_low").c_str(), L"%u", &m_class_low);
	swscanf_s(s->get_property(L"class_high").c_str(), L"%u", &m_class_high);
	return true;
}
bool logclass_filter::save(component_creator* /*cc*/, serializer* s) const
{
	wchar_t buf[32];
	serializer* mys = s->add_child(classname());
	swprintf_s(buf, L"%u", m_class_low);
	mys->set_property(L"class_low", buf);

	swprintf_s(buf, L"%u", m_class_high);
	mys->set_property(L"class_high", buf);

	return true;
}

component* logclass_filter::simplify() const
{
	return clone();
}

int logclass_filter::compare(const leaf* l) const
{
	const logclass_filter* rhs = dynamic_cast<const logclass_filter*>(l);
	return value_compare(m_class_low, rhs->m_class_low, m_class_high, rhs->m_class_high);
}

logclass_filter::logclass_filter(UINT low, UINT high) : m_class_high(high), m_class_low(low)
{
}

std::wstring logcontent_filter::name() const
{
	return IDS2(IDS_FILTER_RULE_LOG_CONTENT);
}
std::wstring logcontent_filter::desc() const
{
	std::wstring str;
	if (m_use_regex)
	{
		str = IDSFMT(IDS_LOG_CONTENT_MATCH_REGEX_s, m_matcher.c_str());
	}
	else
	{
		str = IDSFMT(IDS_LOG_CONTENT_CONTAIN_s, m_matcher.c_str());
	}

	if (m_ignore_case) str += L"  (" + IDS(IDS_IGNORE_CASE) + L")";
	return str;
}
component* logcontent_filter::clone(bool) const
{
	return new logcontent_filter(m_matcher, m_ignore_case, m_use_regex);
}
bool logcontent_filter::meet(const LogInfo& log) const
{
	if (m_use_regex)
	{
		CAtlREMatchContext<CAtlRECharTraits> context;
		return m_regexp->Match(log.item->log_content.c_str(), &context)? true:false;
	}
	if (m_ignore_case)
	{
		return helper::wcsistr(log.item->log_content.c_str(), m_matcher.c_str()) != NULL;
	}
	else
	{
		return wcsstr(log.item->log_content.c_str(), m_matcher.c_str()) != NULL;
	}
}

bool logcontent_filter::load(component_creator* /*cc*/, serializer* s)
{
	m_matcher = s->get_property(L"matcher");
	int tmp = 0;
	swscanf_s(s->get_property(L"ignorecase").c_str(), L"%d", &tmp);
	m_ignore_case = (tmp == 1);
	tmp = 0;
	swscanf_s(s->get_property(L"useregex").c_str(), L"%d", &tmp);
	m_use_regex = (tmp == 1);

	if (m_use_regex)
	{
		m_regexp->Parse(m_matcher.c_str(), m_ignore_case);
	}

	return true;
}
bool logcontent_filter::save(component_creator* /*cc*/, serializer* s) const
{
	serializer* mys = s->add_child(classname());

	mys->set_property(L"matcher", m_matcher.c_str());
	mys->set_property(L"ignorecase", formatstr(L"%d", m_ignore_case? 1: 0));
	mys->set_property(L"useregex", formatstr(L"%d", m_use_regex? 1: 0));

	return true;
}

component* logcontent_filter::simplify() const
{
	return clone();
}

int logcontent_filter::compare(const leaf* f) const
{
	const logcontent_filter* rhs = dynamic_cast<const logcontent_filter*>(f);

	return value_compare(m_ignore_case, rhs->m_ignore_case, m_matcher, rhs->m_matcher, m_use_regex, rhs->m_use_regex);
}

bool logcontent_filter::setfilter(const std::wstring& matcher, bool ignore_case, bool use_regex)
{
	m_matcher = matcher;
	m_ignore_case = ignore_case;
	m_use_regex = use_regex;
	if (m_use_regex)
	{
		ATL::REParseError ret = m_regexp->Parse(matcher.c_str(), m_ignore_case);
		return (ret == REPARSE_ERROR_OK);
	}

	return true;
}

logcontent_filter::logcontent_filter(const std::wstring& matcher, bool ignore_case, bool use_regex)
: m_matcher(matcher), m_ignore_case(ignore_case), m_use_regex(use_regex)
{
	if (m_use_regex)
	{
		m_regexp = new ATL::CAtlRegExp<>();
		m_regexp->Parse(matcher.c_str(), m_ignore_case);
	}
}

logcontent_filter::~logcontent_filter()
{
	if (m_regexp)
	{
		delete m_regexp;
		m_regexp = nullptr;
	}
}


logtag_filter::logtag_filter(const std::wstring& tag)
{
	setfilter(tag);
}
std::wstring logtag_filter::name() const
{
	return IDS2(IDS_FILTER_RULE_LOG_TAG);
}
std::wstring logtag_filter::desc() const
{
	return (LPCWSTR)IDSFMT(IDS_LOG_TAG_MATCH_s, m_tag.c_str());
}
component* logtag_filter::clone(bool) const
{
	return new logtag_filter(m_tag);
}
bool logtag_filter::meet(const LogInfo &log) const
{
	return helper::TagMatch(log.item->log_tags.c_str(), m_tag.c_str());
}

bool logtag_filter::load(component_creator *, serializer *s)
{
	setfilter(s->get_property(L"tag"));
	return true;
}

bool logtag_filter::save(component_creator *, serializer *s) const
{
	serializer* mys = s->add_child(classname());
	mys->set_property(L"tag", m_tag);
	return true;
}

component* logtag_filter::simplify() const
{
	return clone();
}

void logtag_filter::setfilter(const std::wstring& tag)
{
	m_tag = tag;
	m_tag_matcher = m_tag + L";";
}

int logtag_filter::compare(const leaf* l) const
{
	const logtag_filter* rhs = dynamic_cast<const logtag_filter*>(l);
	return value_compare(m_tag, rhs->m_tag);
}


logpid_filter::logpid_filter(unsigned int pid)
{
	setfilter(pid);
}
std::wstring logpid_filter::name() const
{
	return IDS2(IDS_FILTER_RULE_PROCESS_ID);
}
std::wstring logpid_filter::desc() const
{
	return IDSFMT2(IDS_PROCESS_ID_IS_u, m_pid);
}
component* logpid_filter::clone(bool) const
{
	return new logpid_filter(m_pid);
}
bool logpid_filter::meet(const LogInfo &log) const
{
	return log.item->log_pid == m_pid;
}

bool logpid_filter::load(component_creator *, serializer *s)
{
	swscanf_s(s->get_property(L"pid").c_str(), L"%u", &m_pid);
	return true;
}

bool logpid_filter::save(component_creator *, serializer *s) const
{
#ifdef SAVE_UNSTABLE_FILTER
	serializer* mys = s->add_child(classname());
	mys->set_property(L"pid", (const wchar_t*)tp::cz(L"%u", m_pid));
	return true;
#else
	return false;
#endif
}

component* logpid_filter::simplify() const
{
	return clone();
}

void logpid_filter::setfilter(unsigned int pid)
{
	m_pid = pid;
}

int logpid_filter::compare(const leaf* l) const
{
	const logpid_filter* rhs = dynamic_cast<const logpid_filter*>(l);
	return value_compare(m_pid, rhs->m_pid);
}


logtid_filter::logtid_filter(unsigned int tid)
{
	setfilter(tid);
}
std::wstring logtid_filter::name() const
{
	return IDS2(IDS_FILTER_RULE_THREAD_ID);
}
std::wstring logtid_filter::desc() const
{
	return IDSFMT2(IDS_THREAD_ID_IS_u, m_tid);
}
component* logtid_filter::clone(bool) const
{
	return new logtid_filter(m_tid);
}
bool logtid_filter::meet(const LogInfo &log) const
{
	return log.item->log_tid == m_tid;
}

bool logtid_filter::load(component_creator *, serializer *s)
{
	swscanf_s(s->get_property(L"tid").c_str(), L"%u", &m_tid);
	return true;
}

bool logtid_filter::save(component_creator *, serializer *s) const
{
#ifdef SAVE_UNSTABLE_FILTER
	serializer* mys = s->add_child(classname());
	mys->set_property(L"tid", (const wchar_t*)tp::cz(L"%u", m_tid));
	return true;
#else
	return false;
#endif
}

component* logtid_filter::simplify() const
{
	return clone();
}

void logtid_filter::setfilter(unsigned int tid)
{
	m_tid = tid;
}

int logtid_filter::compare(const leaf* l) const
{
	const logtid_filter* rhs = dynamic_cast<const logtid_filter*>(l);
	return value_compare(m_tid, rhs->m_tid);
}


logprocessname_filter::logprocessname_filter(const std::wstring& process_name)
{
	setfilter(process_name);
}
std::wstring logprocessname_filter::name() const
{
	return IDS2(IDS_FILTER_RULE_PROCESSNAME);
}
std::wstring logprocessname_filter::desc() const
{
	return IDSFMT2(IDS_PROCESS_IS_s, m_process_name.c_str());
}
component* logprocessname_filter::clone(bool) const
{
	return new logprocessname_filter(m_process_name);
}
bool logprocessname_filter::meet(const LogInfo &log) const
{
	return _wcsicmp(log.item->log_process_name.c_str(), m_process_name.c_str()) == 0;
}

bool logprocessname_filter::load(component_creator *, serializer *s)
{
	setfilter(s->get_property(L"process_name"));
	return true;
}

bool logprocessname_filter::save(component_creator *, serializer *s) const
{
	serializer* mys = s->add_child(classname());
	mys->set_property(L"process_name", m_process_name);
	return true;
}

component* logprocessname_filter::simplify() const
{
	return clone();
}

void logprocessname_filter::setfilter(const std::wstring& process_name)
{
	m_process_name = process_name;
}

int logprocessname_filter::compare(const leaf* l) const
{
	const logprocessname_filter* rhs = dynamic_cast<const logprocessname_filter*>(l);
	return value_compare(m_process_name, rhs->m_process_name);
}
