#include "stdafx.h"

// ������ȫ�ֶ���������������е���tplog�Ƿ�����

struct GlobalObj
{
	GlobalObj()
	{
		Log(LL_EVENT, NOTAG, L"ȫ�ֶ�����");
	}
	~GlobalObj()
	{
		Log(LL_EVENT, NOTAG, L"ȫ�ֶ������� [0X%p]", this);
	}
}gobj;

