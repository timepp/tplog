#include "stdafx.h"
#include "updater.h"
#include "helper.h"
#include "Resource.h"

unsigned int Updater::CheckAndUpdateWorker(void* param)
{
	helper::DownloadUrlToFile(
		L"http://localhost:8180/help/tplogview_ver.txt",
		Updater::GetVersionInfoFilePath()
		);

	Versions v = Updater::ParseVersionInfoFile();
	if (v.size() > 0 && v.front().ver > helper::GetFileVersion(helper::GetModuleFilePath()))
	{
		CStringW strTmpPath = helper::GetModuleFilePath() + L".tmp";
		CStringW strExePath = helper::GetModuleFilePath();
		CStringW strOldPath = Updater::GetOldExePath();

		helper::DownloadUrlToFile(
			L"http://localhost:8180/help/tplogview.exe",
			strTmpPath
			);

		::MoveFileExW(strExePath, strOldPath, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
		::MoveFileExW(strTmpPath, strExePath, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
	}

	return 0;
}

Versions Updater::ParseVersionInfoFile()
{
	Versions v;
	CStringW verfile = GetVersionInfoFilePath();

	FILE* fp = NULL;
	_wfopen_s(&fp, verfile, L"rt,ccs=UNICODE");
	if (!fp) return v;
	ON_LEAVE_1(fclose(fp), FILE*, fp);

	WCHAR line[1024];
	VersionDetail detail = {0, L""};
	while (fgetws(line, _countof(line), fp))
	{
		bool versionline = true;
		for (WCHAR* p = line; *p; p++)
		{
			if (*p == L'\r' || *p == L'\n') { *p = 0; break; }
			if ((*p < L'0' || *p > L'9') && *p != L'.') versionline = false;
		}
		if (!*line) versionline = false;

		if (versionline)
		{
			if (detail.ver) v.push_back(detail);
			detail.ver = helper::StringVersionToInt(line);
			detail.description = L"";
		}
		else
		{
			detail.description += line;
			detail.description += L"\n";
		}
	}

	if (detail.ver) v.push_back(detail);

	return v;
}

CStringW Updater::GetOldExePath()
{
	return helper::GetModuleFilePath() + L".old";
}
CString Updater::GetVersionInfoFilePath()
{
	return helper::GetConfigDir() + L"\\tplogview_version.txt";
}

// \param bBackend: TRUE, silent mode, doesn't notify user even there is update
//                  FALSE, if no update then notify user; if has update then notify user only when update complete
void Updater::CheckAndUpdate(BOOL bBackend)
{
	if (bBackend)
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, CheckAndUpdateWorker, NULL, 0, NULL);
		::CloseHandle(hThread);
	}
	else
	{
		CheckAndUpdateWorker(NULL);
		if (helper::FileExists(GetOldExePath()))
		{
			::MessageBoxW(NULL, IDS(IDS_MSG_UPDATE_COMPLETE), IDS(IDS_UPDATE_MSG), MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			::MessageBoxW(NULL, IDS(IDS_MSG_NO_UPDATE), IDS(IDS_MSG), MB_OK|MB_ICONINFORMATION);
		}
	}
}

void Updater::NotifyNewVersion()
{
	CStringW strPath = GetOldExePath();
	if (helper::FileExists(strPath))
	{
		UINT64 oldver = helper::GetFileVersion(strPath);
		::DeleteFileW(strPath);

		CStringW info = IDS(IDS_MSG_UPDATE_DETAIL);
		Versions v = Updater::ParseVersionInfoFile();
		for (Versions::const_iterator it = v.begin(); it != v.end(); ++it)
		{
			if (it->ver <= oldver) break;
			info += helper::IntVersionToString(it->ver);
			info += L"\n";
			info += it->description;
			info += L"\n";
		}

		::MessageBoxW(NULL, info, IDS(IDS_UPDATE_MSG), MB_OK|MB_ICONINFORMATION);
	}
}
