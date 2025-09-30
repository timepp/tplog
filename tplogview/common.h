#pragma once

#include <set>
#include <vector>
#include <list>
#include <deque>
#include <tplog_reader.h>

typedef std::set<CStringW> StrSet;
typedef std::list<std::wstring> strlist_t;

struct LogInfo
{
	UINT64 logid;                      // globally unique
	tplog::logitem* item;              // log format received from pipe
	INT64 occupytime;                  // time interval between current log and next log in same thread
};

struct disp_info
{
	COLORREF color;
	COLORREF bkcolor;
	disp_info()
		: color(RGB(0, 0, 0)), bkcolor(RGB(255,255,255))
	{
	}
};

constexpr INT32 M = 1000000; // 1 second = 1,000,000 microseconds

struct accutime
{
	INT64 sec{};
	INT32 usec{};

	accutime(INT64 s = 0, INT32 us = 0): sec(s), usec(us)
	{
	}
	accutime(const accutime& rhs): sec(rhs.sec), usec(rhs.usec)
	{
	}
	accutime& operator=(const accutime& rhs)
	{
		sec = rhs.sec;
		usec = rhs.usec;
		return *this;
	}

	accutime& operator+=(INT64 us)
	{
		us += usec;
		sec += us / M;
		usec = us % M;
		return *this;
	}
	accutime& operator-=(INT64 t)
	{
		INT64 s = t / M;
		INT32 us = static_cast<INT32>(t % M);
		sec -= s;
		if (us > usec)
		{
			sec--;
			usec = M + usec - us;
		}
		else
		{
			usec -= us;
		}
		return *this;
	}
	accutime operator+(INT64 us)
	{
		accutime ret(*this);
		ret += us;
		return ret;
	}
	accutime operator-(INT64 us)
	{
		accutime ret(*this);
		ret -= us;
		return ret;
	}

	INT64 operator-(const accutime& rhs) const
	{
		if (*this < rhs) return -(rhs-*this);
		return (sec - rhs.sec) * M + usec - rhs.usec;
	}

	bool operator<(const accutime& rhs) const
	{
		if (sec < rhs.sec) return true;
		if (sec > rhs.sec) return false;
		return usec < rhs.usec;
	}
	bool operator>(const accutime& rhs) const
	{
		if (sec > rhs.sec) return true;
		if (sec < rhs.sec) return false;
		return usec > rhs.usec;
	}
	bool operator==(const accutime& rhs) const
	{
		return sec == rhs.sec && usec == rhs.usec;
	}

};

struct time_range
{
	accutime t1;
	accutime t2;
};


#define WM_FILTER_CHANGE     WM_USER + 21

