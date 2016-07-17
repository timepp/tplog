#include "stdafx.h"

#include <tplog_singleton.h>

BOOL APIENTRY DllMain( HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		GetLogController();
	}
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		GetLogController()->UnInit();
	}

	return TRUE;

}

// TPLOG_MINIMAL_SIZE should be defined together with the following configurations
// * Set entry point to DllMain 

#ifdef TPLOG_MINIMAL_SIZE

#define WIDE_STR_(x) L##x
#define WIDE_STR(x) WIDE_STR_(x)

// Use symview to check if "should not link" appears in binary's symbol table. 
// If so, we have linked the wrong implementation, we must check it and remove it.
#define SHOULD_NOT_LINK(reason)  LOG(L"should not link(" reason L"): " WIDE_STR(__FUNCTION__))

// Default implementation of net/delete is too complicated.
// tplog uses little allocation work, so HeapAlloc/HeapFree is enough.
void * operator new(unsigned int size)
{
	return ::HeapAlloc(::GetProcessHeap(), 0, size);
}
void operator delete(void* p)
{
	::HeapFree(::GetProcessHeap(), 0, p);
}

int _purecall()
{
	return 0;
}

// Default 'wcschr' occupies 170 more bytes, we hand craft a simpler version.
const wchar_t* wcschr(const wchar_t* src, wchar_t ch)
{
	while (*src)
	{
		if (*src == ch) return src;
		src++;
	}
	return NULL;
}

int wcsncmp(const wchar_t*, const wchar_t*, size_t)
{
    return 0;
}

#ifndef _DEBUG
int _itow_s(int /*val*/, wchar_t* /*buf*/, size_t /*buflen*/, int /*redix*/)
{
	SHOULD_NOT_LINK(L"Large default implementation");
	return 0;
}

int wcsncpy_s(wchar_t* /*dest*/, size_t /*destlen*/, const wchar_t* /*src*/, size_t /*copylen*/)
{
	SHOULD_NOT_LINK(L"Large default implementation");
	return 0;
}

int wcsncat_s(wchar_t* /*dest*/, size_t /*destlen*/, const wchar_t* /*src*/, size_t /*copylen*/)
{
	SHOULD_NOT_LINK(L"Large default implementation");
	return 0;
}

int _wtoi(const wchar_t* /*str*/)
{
	SHOULD_NOT_LINK(L"Large default implementation");
	return 0;
}

int atexit ( void ( * /*func*/ ) (void) )
{
	SHOULD_NOT_LINK(L"we do not support atexit");
	return 0;
}

#endif

#endif