#include "stdafx.h"
#include <tplib/include/unittest.h>
#include <tplib/include/util_win.h>

LARGE_INTEGER CProfiler::m_freq = {};
BOOL CProfiler::m_avaliable = ::QueryPerformanceFrequency(&CProfiler::m_freq);

// ²âÊÔtplogµÄÐÔÄÜ

static int PerformanceTest(size_t times)
{
	CProfiler pf(true);
	for (size_t i = 0; i < times; i++)
	{
		Log(LL_DEBUG, TAG(L"aa;bb"), L"aaa", 1);
		Log(LL_DEBUG, NOTAG, L"simple log text with code analysis warning %d", L"hhh");
		Log(LL_DEBUG, TAG(L"kernel;perf;logic"), L"the logic control flow: %s%d", L"aaa", 333);
	}
	return pf.GetElapsedMicroSeconds();
}

static void RunPerformanceTest(const wchar_t* scene, double expect_us)
{
	size_t times = 100000;
	int total_us = 0;
	total_us += PerformanceTest(times);
	int total_count = times * 3;
	double avg_us = static_cast<double>(total_us) / total_count;
	wchar_t desc[1024];
	_snwprintf_s(desc, _TRUNCATE, L"%s, everage time per log < %.2f us", scene, expect_us);
	tp::unittest::expect(avg_us < expect_us, desc, L"actual : %.4f us", avg_us);
}

TPUT_DEFINE_BLOCK(L"performance", L"perf")
{
	std::vector<tp::oswin::file> files;
	tp::oswin::list_dir(files, L".", L"test_perf_*.log");
	for (size_t i = 0; i < files.size(); i++)
	{
		::DeleteFileW(files[i].rela_path.c_str());
	}

	ILogController* ctrl = GetLogController();
	ctrl->UnInit();
	ctrl->Init(NULL);
	ctrl->RemoveOutputDevice(NULL);

	ctrl->AddOutputDevice(L"pipe", LODT_PIPE, L"");
	ctrl->AddOutputDevice(L"file", LODT_FILE, L"");
	ctrl->AddOutputDevice(L"sm", LODT_SHARED_MEMORY, L"enable:true");

	ctrl->ChangeOutputDeviceConfig(L"sm", L"filter:level>100");
	RunPerformanceTest(L"share memory log, all filtered", 0.1);

	ctrl->ChangeOutputDeviceConfig(L"sm", L"filter:");
	RunPerformanceTest(L"share memory log", 1);

	ctrl->ChangeOutputDeviceConfig(L"pipe", L"enable:true");
	RunPerformanceTest(L"share memory + unconnected pipe", 1);

	ctrl->ChangeOutputDeviceConfig(L"pipe", L"enable:false");

	ctrl->ChangeOutputDeviceConfig(L"file", L"enable:true path:test_perf_sm_fsb.log");
	RunPerformanceTest(L"share memory + sync file cached", 2);

	ctrl->ChangeOutputDeviceConfig(L"sm", L"enable:false");

	ctrl->ChangeOutputDeviceConfig(L"file", L"async:false path:test_perf_fsb.log");
	RunPerformanceTest(L"sync file cached", 2);

	ctrl->ChangeOutputDeviceConfig(L"file", L"async:true path:test_perf_fab.log");
	RunPerformanceTest(L"async file cached", 2);

	ctrl->ChangeOutputDeviceConfig(L"file", L"buffer_size:0 async:false path:test_perf_fs.log");
	RunPerformanceTest(L"sync file", 10);

	ctrl->ChangeOutputDeviceConfig(L"file", L"buffer_size:0 async:true path:test_perf_fa.log");
	RunPerformanceTest(L"async file", 10);

	ctrl->ChangeOutputDeviceConfig(L"file", L"async:false buffer_size:1000000 share_read:false path:test_perf_fsbe.log");
	RunPerformanceTest(L"sync file cached exclusive", 3);
}
