#include "stdafx.h"
#include "testlog.h"
#include <atlstr.h>
#include <vector>
#include <iostream>
#include <tplib/include/auto_release.h>
#include <tplib/include/unittest.h>
#include <tplib/include/unittest_output.h>
#include <map>
#include <string>
#include <tplog_impl.h>

CAppModule _Module;

#define TPUT_MODNAME Main
#define LOGTEST_REG_PATH L"SOFTWARE\\tplog\\test"

class CTestOutputDeviceBase : public ILogOutputDevice
{
public:
	CTestOutputDeviceBase() : m_refCounter(0) { }
	~CTestOutputDeviceBase() {}

	virtual HRESULT Open(ILogOption* /*opt*/)             { return S_OK; }
	virtual HRESULT Close()                               { return S_OK; }
	virtual HRESULT Write(const LogItem* /*item*/)        { return S_OK; }
	virtual HRESULT Flush()                               { return S_OK; }
	virtual HRESULT OnConfigChange(ILogOption* /*opt*/)   { return S_OK; }

	virtual void IncreaseRefCounter()
	{
		++m_refCounter;
	}
	virtual void DecreaseRefCounter()
	{
		--m_refCounter;
		if (m_refCounter <= 0)
		{
			delete this;
		}
	}
private:
	int m_refCounter;
};

class CLODCallCounter: public CTestOutputDeviceBase
{
public:
	CLODCallCounter()
	{
		ResetCount();
	}

	void ResetCount()
	{
		size_t off = offsetof(CLODCallCounter, m_count_Open);
		memset((char*)this + off, 0, sizeof(*this)-off);
	}

	bool CompareCount(int cntOpen, int cntClose = -1, int cntWrite = -1, int cntOnConfigChange = -1)
	{
		if (cntOpen != -1 && cntOpen != m_count_Open) return false;
		if (cntClose != -1 && cntClose != m_count_Close) return false;
		if (cntWrite != -1 && cntWrite != m_count_Write) return false;
		if (cntOnConfigChange != -1 && cntOnConfigChange != m_count_OnConfigChange) return false;
		return true;
	}

	virtual HRESULT Open(ILogOption* /*opt*/)             { m_count_Open++; return S_OK; }
	virtual HRESULT Close()                               { m_count_Close++; return S_OK; }
	virtual HRESULT Write(const LogItem* /*item*/)        { m_count_Write++; return S_OK; }
	virtual HRESULT Flush()                               { return S_OK; }
	virtual HRESULT OnConfigChange(ILogOption* /*opt*/)   { m_count_OnConfigChange++; return S_OK; }

private:
	int m_count_Open;
	int m_count_Close;
	int m_count_Write;
	int m_count_OnConfigChange;
};



static void PrintSystemInfo()
{
	// TODO
}

#define TAG_FT TAG(L"function_test")

int CalcF(int n)
{
	LOG_FUNCTION(n, 1, 2, L"x++");
	if (n <= 1) return 1;
	return CalcF(n-2) + CalcF(n-4);
}

void TEST_LogFunction()
{
#undef LOGTAG_FILE
#define LOGTAG_FILE L"abc"
	ILogController* ctrl = GetLogController();
	ctrl->AddOutputDevice(L"pipe", LODT_PIPE, L"enable:1");
	ON_LEAVE_1(ctrl->RemoveOutputDevice(L"pipe"), ILogController*, ctrl);

	struct inner
	{
		static int Calc(int n)
		{
			LOG_FUNCTION(n);
			if (n <= 1) return 1;
			return Calc(n-1) + Calc(n-2);
		}
	};

	int n = inner::Calc(4);
	Log(LL_DEBUG, TAG_FT, L"f(%d) = %d", 10, n);
}

void TEST_OutputDevice_File()
{
	ILogController* ctrl = GetLogController();
	ctrl->RemoveOutputDevice(NULL);

	ctrl->AddOutputDevice(L"file", LODT_FILE, L"enable:1 path:'%appdata%\\${PID}${}${$}_${T}_a${DATE}_b${TIME}_u${no}${u${.tplog'");
	Log(LL_EVENT, NOTAG, L"��־����");

	ctrl->RemoveOutputDevice(NULL);

	// TODO ������־·�������ڵ�ʱ���ļ���־�豸�Ƿ���Զ�����
}

HRESULT TEST_TP_CHECK()
{
	TP_CHECK(GetCurrentThreadId() == 0, IGNORE_FAIL);
	TP_CHECK(GetCurrentThreadId() == 10, RETURN_HR_ON_FAIL);
	TP_CHECK_HR(GetCurrentThreadId() == 10, RETURN_HR_ON_FAIL);

//	TP_VERIFY(6 - 6 == 1);
//	TP_ENSURE_RETURN(3-3!=0, );

//	BOOL ret = ::WriteFile(NULL, NULL, 0, 0, NULL);
//	TP_ENSURE_WINAPI_RETURN_HR(ret);

//	HRESULT hr = E_FAIL;
//	TP_ENSURE_SUCCEEDED_RETURN_HR(hr);

	return S_OK;
}

void TEST_ManyManyLog()
{
	ILogController* ctrl = GetLogController();
	ctrl->AddOutputDevice(L"pipe", LODT_PIPE, L"enable:true");
	CProfiler pf;
	pf.Start();
	for (size_t i = 0; i < 10000; i++)
	{
		Log(LL_DEBUG, NOTAG, L"aaaaassssssssssssssssssssssssssaaaaaa");
	}
	int x = pf.GetElapsedMicroSeconds();
	CStringW strResult;
	strResult.Format(L"%d", x);
	::MessageBox(NULL, strResult, strResult, MB_OK);
}

void TEST_DeadLoop()
{
	ILogController* ctrl = GetLogController();
	ctrl->AddOutputDevice(L"pipe2", LODT_PIPE, L"enable:true");

	const wchar_t * longstr = L"aaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssssaaaaassssssssssssssssssssssssssaaaaaaaaaaasssssssssssssssssssssssss";

	for (;;)
	{
		Log(LL_DEBUG, NOTAG, L"aaaaassssssssssssssssssssssssssaaaaaa%s", longstr);
		Log(LL_DEBUG, NOTAG, L"aaaaassssssssssssssssssssssssssaaaaaa");
		Log(LL_DEBUG, NOTAG, L"aaaaassssssssssssssssssssssssssaaaaaa%s", longstr);
		::Sleep(1);
	}
}

void TEST_Basic()
{
	ILogController* ctrl = GetLogController();
	ctrl->AddOutputDevice(L"pipe", LODT_PIPE, L"enable:true");

#undef LOGTAG_MODULE
#define LOGTAG_MODULE L"mod1"
	Log(LL_EVENT, TAG(L"tag1"), L"aaa");

#undef LOGTAG_MODULE
#define LOGTAG_MODULE L""
#undef LOGTAG_FILE
#define LOGTAG_FILE L"file2"
	Log(LL_EVENT, TAG(L"tag2"), L"bbb");

	ctrl->RemoveOutputDevice(L"pipe");
}

tplog::fmter& operator<<(tplog::fmter& fmt, const RECT& /*rc*/)
{
	return fmt;
}

void TEST_Misc()
{
	ILogController* ctrl = GetLogController();
	ctrl->AddOutputDevice(L"dbgout", LODT_DEBUGOUTPUT, L"enable:1");

	for (size_t i = 0; i < 10; i++)
	{
		Sleep(100);
		Log(LL_EVENT, TAG_DEFAULT, L"test %u", i);
	}

	ctrl->RemoveOutputDevice(L"dbgout");
}

TPUT_DEFINE_BLOCK(L"Init", L"")
{
	ILogController* ctrl = GetLogController();

	struct inner
	{
		static bool CheckInvokeResults()
		{
			ILogController* ctrl = GetLogController();
			HRESULT ret[100];
			size_t i = 0;
			ret[i++] = ctrl->UnInit();
			ret[i++] = ctrl->Log(LL_EVENT, L"", L"test");
			ret[i++] = ctrl->AddOutputDevice(L"aaa", LODT_FILE, L"");
			ret[i++] = ctrl->AddCustomOutputDevice(L"aaa", NULL, L"");
			ret[i++] = ctrl->ChangeOutputDeviceConfig(L"aaa", L"");
			ret[i++] = ctrl->IncreaseCallDepth();
			ret[i++] = ctrl->DecreaseCallDepth();
			ret[i++] = ctrl->RemoveOutputDevice(L"aaa");
			for (size_t j = 0; j < i; j++)
			{
				if (ret[j] != TPLOG_E_NOT_INITED) return false;
			}
			return true;
		}
	};

	ctrl->UnInit();
	TPUT_EXPECT(inner::CheckInvokeResults(), L"δ��ʼ��ʱ���ýӿڣ����ش���(֮ǰδ��ʼ����)");
	tp::unittest::expect(SUCCEEDED(ctrl->Init(NULL)), L"������ʼ���ɹ�");
	tp::unittest::expect(SUCCEEDED(ctrl->UnInit()), L"����ע���ɹ�");
	tp::unittest::expect(inner::CheckInvokeResults(), L"δ��ʼ��ʱ���ýӿڣ����ش���(��ʼ������ע��)");

	ctrl->UnInit();
	ctrl->Init(L".");
	TPUT_EXPECT(ctrl->Init(L".") == TPLOG_E_ALREADY_INITED, L"�ظ���ʼ��ʱ���ض�Ӧ�Ĵ�����");

	HRESULT hr = S_OK;
	for (size_t i = 0; i < 100; i++)
	{
		hr = ctrl->UnInit();
		if (FAILED(hr)) break;
		hr = ctrl->Init(L".");
		if (FAILED(hr)) break;
	}
	TPUT_EXPECT(SUCCEEDED(hr), L"������ʼ��/ע��ʱ������");
}

TPUT_DEFINE_BLOCK(L"LogOption", L"")
{
	ILogController* ctrl = GetLogController();
	ctrl->UnInit();
	ctrl->Init(L".");

	typedef std::map<const wchar_t*, const wchar_t*> strmap_t;
	struct TestLOD1 : public CTestOutputDeviceBase
	{
		const strmap_t& m_expectOptions;
		BOOL m_expect;
		TestLOD1(const strmap_t& m) : m_expectOptions(m), m_expect(false) {}
		virtual HRESULT Open(ILogOption* opt)
		{
			m_expect = TRUE;
			for (strmap_t::const_iterator it = m_expectOptions.begin(); it != m_expectOptions.end(); ++it)
			{
				const wchar_t* optval = opt->GetOption(it->first, NULL);
				if (it->second == NULL && optval == NULL) continue;
				if (it->second == NULL || optval == NULL || wcscmp(it->second, optval) != 0)
				{
					m_expect = FALSE;
					break;
				}
			}
			return S_OK;
		}


		BOOL AllOptionIsAsExpect() const
		{
			return m_expect;
		}
	};

	strmap_t opts;
	opts[L"k1"] = L"normal";
	opts[L"k2"] = L"";
	opts[L"k3"] = L"bl ank";
	opts[L"k4"] = L"bl\tank";
	opts[L"k5"] = L"normal";
	opts[L"k6"] = L"a:b";
	opts[L"k7"] = L"a:c=b";
	opts[L"k8"] = L"~!@#$%^&*()_+{}|:\"?><,./;[]\\=-`\n\r\t";
	const wchar_t* optstr = 
		L"enable:true k1:normal k2: k3:'bl ank' k4:\"bl\tank\" k5:'normal' k6:a:b k7:'a:c=b' "
		L"k8:'~!@#$%^&*()_+{}|:\"?><,./;[]\\=-`\n\r\t'";
	TestLOD1* lod1 = new TestLOD1(opts);
	ctrl->AddCustomOutputDevice(L"test", lod1, optstr);
	TPUT_EXPECT(lod1->AllOptionIsAsExpect(), L"��־�豸����ȷ�յ�����");

	ctrl->RemoveOutputDevice(L"test");
}

TPUT_DEFINE_BLOCK(L"LogOption.Filter", L"")
{
	struct testcase
	{
		const wchar_t* filterstr;
		int level;
		const wchar_t* tag;
		BOOL match;
	};
	
	struct inner
	{
		static void RunTest(const testcase* cases, size_t len, const wchar_t* desc)
		{
			ILogController* ctrl = GetLogController();
			CLODCallCounter* lod = new CLODCallCounter;
			ctrl->RemoveOutputDevice(L"test");
			ctrl->AddCustomOutputDevice(L"test", lod, L"enable:1");

			size_t i = 0;
			for (; i < len; i++)
			{
				lod->ResetCount();
				const testcase& t = cases[i];
				ctrl->ChangeOutputDeviceConfig(L"test", t.filterstr);
				if (ctrl->NeedLog((LogLevel)t.level, t.tag) != t.match) break;
				Log((LogLevel)t.level, LogTag(t.tag), L"%s", L"content");
				if (!lod->CompareCount(-1, -1, (t.match? 1 : 0), -1)) break;
			}

			TPUT_EXPECT(i == len, desc);

			ctrl->RemoveOutputDevice(L"test");
		}
	};

	ILogController* ctrl = GetLogController();
	ctrl->UnInit();
	ctrl->Init(L".");
	
	{
		testcase cases[] = 
		{
			L"filter:", 0, L"", TRUE,
			L"filter:", 0, L"tag", TRUE,
			L"filter:", 1, L"", TRUE,
			L"filter:", LL_EVENT, L"tag", TRUE
		};
		inner::RunTest(cases, _countof(cases), L"�չ��˹���(ƥ��ȫ��)");
	}

	{
		testcase cases[] = 
		{
			L"filter:level=25", 0,  L"", FALSE,
			L"filter:level=25", 25, L"", TRUE,
			L"filter:level=25", 10, L"", FALSE,
			L"filter:level=0",  25, L"", FALSE,
			L"filter:level>25", 0,  L"", FALSE,
			L"filter:level>25", 25, L"", FALSE,
			L"filter:level>25", 48, L"", TRUE,
			L"filter:level>0",  25, L"", TRUE,
			L"filter:level<25", 0,  L"", TRUE,
			L"filter:level<25", 25, L"", FALSE,
			L"filter:level<25", 10, L"", TRUE,
			L"filter:level<0",  0, L"", FALSE,
		};
		inner::RunTest(cases, _countof(cases), L"��־������˹���");
	}

	{
		testcase cases[] = 
		{
			L"filter:tag=perf", 16,  L"perf", TRUE,
			L"filter:tag=perf", 16,  L"peef", FALSE,
			L"filter:tag=perf", 16,  L"perf;xyz", TRUE,
			L"filter:tag=perf", 16,  L"xyz;perf", TRUE,
			L"filter:tag=perf", 16,  L"abc;perf;xyz", TRUE,
			L"filter:tag=perf", 16,  L"abc;prf;xyz", FALSE,
			L"filter:tag=perf", 16,  L"perfa", FALSE,
		};
		inner::RunTest(cases, _countof(cases), L"��־��ǩ���˹���");
	}

	{
		testcase cases[] = 
		{
			L"filter:'tag=perf && level>16'", 17,  L"perf", TRUE,
			L"filter:'tag=perf && level>16'", 17,  L"peef", FALSE,
			L"filter:'tag=perf && level>16'", 15,  L"perf", FALSE,
		};
		inner::RunTest(cases, _countof(cases), L"���Ϲ��˹�����");
	}

	{
		testcase cases[] = 
		{
			L"filter:tag=perf||level>16", 16,  L"perf", TRUE,
			L"filter:tag=perf||level>16", 17,  L"peef", TRUE,
			L"filter:tag=perf||level>16", 15,  L"peef", FALSE,
		};
		inner::RunTest(cases, _countof(cases), L"���Ϲ��˹��򣺻�");
	}

	{
		testcase cases[] = 
		{
			L"filter:!(tag=perf)", 16,  L"peef", TRUE,
			L"filter:!(tag=perf)", 16,  L"perf", FALSE,
			L"filter:!(!(tag=perf))", 16,  L"peef", FALSE,
			L"filter:!(!(tag=perf))", 16,  L"perf", TRUE,
		};
		inner::RunTest(cases, _countof(cases), L"���Ϲ��˹��򣺷�");
	}

	{
		testcase cases[] = 
		{
			L"filter:'level>16 && level<32 || tag=perf'", 16,  L"perf", TRUE,
			L"filter:'tag=perf || level<32 && level>16'", 16,  L"perf", FALSE,
			L"filter:'tag=perf || level<32 && !(level>16)'", 16,  L"perf", TRUE,
		};
		inner::RunTest(cases, _countof(cases), L"���ӹ��˹������");
	}

	{
		testcase cases[] = 
		{
			L"filter:tag", 16,  L"perf", TRUE,
			L"filter:=", 16,  L"pef", TRUE,
			L"filter:&&&&&&&&&&&&&&&&&&&&&&", 16,  L"perf;xyz", TRUE,
			L"filter:||||||||||||||||||||||", 16,  L"xyz;perf", TRUE,
			L"filter:!!!!!!!!!!!!!!!!!!!!!!", 16,  L"abc;perf;xyz", TRUE,
			L"filter:&}}&&}*&||&&||&&!&&|||", 16,  L"abc;prf;xyz", TRUE,
			L"filter:======================", 16,  L"perfa", TRUE,
			L"filter:()(((())))))))()())(()", 16,  L"perfa", TRUE,
		};
		inner::RunTest(cases, _countof(cases), L"����Ĺ��˴�");
	}
}

TPUT_DEFINE_BLOCK(L"LogOption.Enable", L"")
{
	ILogController* ctrl = GetLogController();
	ctrl->Init(NULL);

	CLODCallCounter* lod = new CLODCallCounter;
	lod->IncreaseRefCounter();
	
	bool ret = true;
	lod->ResetCount();
	ctrl->AddCustomOutputDevice(L"test", lod, L"enable:true");
	ret = ret && lod->CompareCount(1, 0, 0, 0);
	Log(LL_DEBUG, TAG_DEFAULT, L"%s", L"");
	ret = ret && lod->CompareCount(1, 0, 1, 0);
	TPUT_EXPECT(ret, L"�����÷�ʽ����豸�ܵ���Open�����豸���յ���־");

	ret = true;
	lod->ResetCount();
	ctrl->ChangeOutputDeviceConfig(L"test", L"enable:false");
	ret = ret && lod->CompareCount(0, 1, 0, 0);
	Log(LL_DEBUG, TAG_DEFAULT, L"%s", L"");
	ret = ret && lod->CompareCount(0, 1, 0, 0);
	TPUT_EXPECT(ret, L"������־�豸Ϊ����ʱ�ܵ���Close�����豸���ٽ�����־");
	ctrl->RemoveOutputDevice(L"test");

	ret = true;
	lod->ResetCount();
	ctrl->AddCustomOutputDevice(L"test", lod, L"enable:false");
	ret = ret && lod->CompareCount(0, 0, 0, 0);
	Log(LL_DEBUG, TAG_DEFAULT, L"%s", L"");
	ret = ret && lod->CompareCount(0, 0, 0, 0);
	TPUT_EXPECT(ret, L"�Խ��÷�ʽ����豸������Open���Ҳ�������־");

	ret = true;
	lod->ResetCount();
	ctrl->ChangeOutputDeviceConfig(L"test", L"enable:true");
	ret = ret && lod->CompareCount(1, 0, 0, 0);
	Log(LL_DEBUG, TAG_DEFAULT, L"%s", L"");
	ret = ret && lod->CompareCount(1, 0, 1, 0);
	TPUT_EXPECT(ret, L"������־�豸Ϊ����ʱ�ܵ���Open�ͽ�����־");
}

TPUT_DEFINE_BLOCK(L"#a3", L"")
{
	ILogController* ctrl = GetLogController();
	HRESULT hr;
	ctrl->Init(L".");
	TPUT_EXPECT(ctrl->RemoveOutputDevice(L"aaa") == TPLOG_E_NOT_INITED, 0);
}

TPUT_DEFINE_BLOCK(L"#��ʼ����־ϵͳ", L"")
{
	ILogController* ctrl = GetLogController();
	ctrl->Init(L".");
}

TPUT_DEFINE_BLOCK(L"#OutputDevice.Pipe", L"")
{
	ILogController* ctrl = GetLogController();
	TPUT_EXPECT(ctrl->AddOutputDevice(L"pipe", LODT_PIPE, L"enable:1") == S_OK, L"��ӹܵ���־�豸");

	for (size_t i = 0; i < 100; i++)
	{
		Log(LL_EVENT, TAG_DEFAULT, L"loop var: %d", i);
	}
}

TPUT_DEFINE_BLOCK(L"#Pipe.Interact", L"")
{
	ILogController* ctrl = GetLogController();

	ctrl->Init(LOGTEST_REG_PATH);
	ctrl->AddOutputDevice(L"pipe", LODT_PIPE, L"enable:1");
	ctrl->AddOutputDevice(L"file", LODT_FILE, L"enalbe:0");

	ctrl->MonitorConfigChange();

	for (;;)
	{
		::Sleep(1000);
		Log(LL_EVENT, TAG_DEFAULT, L"loop");
	}
}

TPUT_DEFINE_BLOCK(L"#Performance", L"")
{
	ILogController* ctrl = GetLogController();
	ctrl->Init(LOGTEST_REG_PATH);
	ctrl->AddOutputDevice(L"sm", LODT_SHARED_MEMORY, L"enable:1");

	for (size_t i = 0; i < 1000000; i++)
	{
		Log(LL_EVENT, TAG_DEFAULT, L"loop #%d", i);
	}
}

TPUT_DEFINE_BLOCK(L"LOD.DebugOutput", L"")
{
	ILogController* ctrl = GetLogController();
	ctrl->UnInit();
	ctrl->Init(LOGTEST_REG_PATH);
	ctrl->AddOutputDevice(L"dbgout", LODT_DEBUGOUTPUT, L"enable:true");

	Log(LL_EVENT, TAG_DEFAULT, L"%s", L"Hello, Debug Output.");
}

TPUT_DEFINE_BLOCK(L"LOD.Pipe", L"")
{
	ILogController* ctrl = GetLogController();
	ctrl->UnInit();
	ctrl->Init(LOGTEST_REG_PATH);
	ctrl->AddOutputDevice(L"pipe", LODT_PIPE, L"enable:true");

	Log(LL_EVENT, TAG_DEFAULT, L"%s", L"Hello, Pipe.");
}

TPUT_DEFINE_BLOCK(L"LOD.File", L"")
{
	ILogController* ctrl = GetLogController();
	ctrl->UnInit();
	ctrl->Init(LOGTEST_REG_PATH);
	ctrl->AddOutputDevice(L"file", LODT_FILE, L"enable:true path:'d:\\${T}.log'");

	for (size_t i = 0; i < 10; i++)
	{
		Log(LL_EVENT, TAG_DEFAULT, L"%s", L"Hello, File.");
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR lpstrCmdLine, int nCmdShow)
{
    tp::unittest& ut = tp::unittest::instance();
    tp::ListTestOutput lto(L"TPLOG TEST");
    ut.set_test_output(&lto);
    ut.run_test();
    lto.WaitUIExit();

	return 0;
}
