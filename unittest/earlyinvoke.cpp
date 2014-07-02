#include "stdafx.h"

struct GlobalObj
{
	GlobalObj()
	{
		Log(LL_EVENT, NOTAG, L"Global object construct");
	}
	~GlobalObj()
	{
		Log(LL_EVENT, NOTAG, L"Global object destruct [0X%p]", this);
	}
}gobj;

