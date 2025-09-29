#pragma once

#include "filter.h"

#pragma warning(push, 1)
#include <atlserver/atlrx.h>
#pragma warning(pop)

class logclass_filter: public filter, public leaf
{
	DEFINE_CLASS_NAME(logclass_filter);

public:
	virtual std::wstring name() const;
	virtual std::wstring desc() const;
	virtual component* clone(bool /*deep*/ = true) const;
	virtual bool meet(const LogInfo& log) const;

	virtual bool load(component_creator* /*cc*/, serializer* s);
	virtual bool save(component_creator* /*cc*/, serializer* s) const;

	virtual component* simplify() const;
	virtual int compare(const leaf* f) const;

	logclass_filter(UINT low, UINT high);

public:
	UINT m_class_low;
	UINT m_class_high;
};

class logcontent_filter : public filter, public leaf
{
	DEFINE_CLASS_NAME(logcontent_filter);

public:
	virtual std::wstring name() const;
	virtual std::wstring desc() const;
	virtual component* clone(bool /*deep*/ = true) const;
	virtual bool meet(const LogInfo& log) const;

	virtual bool load(component_creator* /*cc*/, serializer* s);
	virtual bool save(component_creator* /*cc*/, serializer* s) const;

	virtual component* simplify() const;
	virtual int compare(const leaf* f) const;

	bool setfilter(const std::wstring& matcher, bool ignore_case, bool use_regex);
	
	logcontent_filter(const std::wstring& matcher, bool ignore_case, bool use_regex);
	~logcontent_filter();

public:
	std::wstring m_matcher;
	bool m_ignore_case;
	bool m_use_regex;
	ATL::CAtlRegExp<> *m_regexp = nullptr;
};

class logtag_filter: public filter, public leaf
{
	DEFINE_CLASS_NAME(logtag_filter);

public:
	virtual std::wstring name() const;
	virtual std::wstring desc() const;
	virtual component* clone(bool /*deep*/ = true) const;
	virtual bool meet(const LogInfo& log) const;

	virtual bool load(component_creator* /*cc*/, serializer* s);
	virtual bool save(component_creator* /*cc*/, serializer* s) const;

	virtual component* simplify() const;
	virtual int compare(const leaf* f) const;

	logtag_filter(const std::wstring& tag);
	void setfilter(const std::wstring& tag);

public:
	std::wstring m_tag;
private:
	std::wstring m_tag_matcher;
};

class logprocessname_filter: public filter, public leaf
{
	DEFINE_CLASS_NAME(logprocessname_filter);

public:
	virtual std::wstring name() const;
	virtual std::wstring desc() const;
	virtual component* clone(bool /*deep*/ = true) const;
	virtual bool meet(const LogInfo& log) const;

	virtual bool load(component_creator* /*cc*/, serializer* s);
	virtual bool save(component_creator* /*cc*/, serializer* s) const;

	virtual component* simplify() const;
	virtual int compare(const leaf* f) const;

	logprocessname_filter(const std::wstring& process_name);
	void setfilter(const std::wstring& process_name);

public:
	std::wstring m_process_name;
};

class logpid_filter: public filter, public leaf
{
	DEFINE_CLASS_NAME(logpid_filter);

public:
	virtual std::wstring name() const;
	virtual std::wstring desc() const;
	virtual component* clone(bool /*deep*/ = true) const;
	virtual bool meet(const LogInfo& log) const;

	virtual bool load(component_creator* /*cc*/, serializer* s);
	virtual bool save(component_creator* /*cc*/, serializer* s) const;

	virtual component* simplify() const;
	virtual int compare(const leaf* f) const;

	logpid_filter(unsigned int pid);
	void setfilter(unsigned int pid);

public:
	unsigned int m_pid;
};

class logtid_filter: public filter, public leaf
{
	DEFINE_CLASS_NAME(logtid_filter);

public:
	virtual std::wstring name() const;
	virtual std::wstring desc() const;
	virtual component* clone(bool /*deep*/ = true) const;
	virtual bool meet(const LogInfo& log) const;

	virtual bool load(component_creator* /*cc*/, serializer* s);
	virtual bool save(component_creator* /*cc*/, serializer* s) const;

	virtual component* simplify() const;
	virtual int compare(const leaf* f) const;

	logtid_filter(unsigned int tid);
	void setfilter(unsigned int tid);

public:
	unsigned int m_tid;
};
