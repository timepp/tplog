#pragma once

/************************************************************************
update process:
1. check
2. download new version to bdlogview.exe.new
3. rename bdlogview.exe to bdlogview.exe.old
4. rename bdlogview.exe.new to bdlogview.exe
5. take effect when restart
6. on start, if bdlogview.exe.old exists, delte it, show what's new
************************************************************************/

#include <vector>

struct VersionDetail
{
	UINT64 ver;
	CStringW description;
};

typedef std::vector<VersionDetail> Versions;

struct Updater
{
	static void CheckAndUpdate(BOOL bBackend);

	static void NotifyNewVersion();

	static CStringW GetVersionInfoFilePath();

private:
	static Versions ParseVersionInfoFile();
	static CStringW GetOldExePath();
	static unsigned int __stdcall CheckAndUpdateWorker(void* param);
};

