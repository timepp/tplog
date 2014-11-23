#include "stdafx.h"
#include "helper.h"
#include <time.h>
#include <psapi.h>
#include <atlsync.h>
#include "config.h"
#include "servicehelper.h"
#include <wininet.h>

GlobalData GD;

CStringW helper::LoadStringFromResource(UINT id)
{
	wchar_t buffer[1024];
	AtlLoadString(id, buffer, _countof(buffer));
	return buffer;
}

CStringW helper::LoadStringFromResourceFmt(UINT id, ...)
{
	wchar_t buffer[1024] = L"";
	AtlLoadString(id, buffer, _countof(buffer));

	CStringW ret;
	va_list args;
	va_start(args, id);
	ret.FormatV(buffer, args);
	va_end(args);
	return ret;
}

void helper::SplitString(const CStringW& str, LPCWSTR pszSpliter, std::set<CStringW>& ss)
{
	ss.clear();

	int split_size = static_cast<int>(wcslen(pszSpliter));
	int pos = 0;
	for (;;)
	{
		int pos_end = str.Find(pszSpliter, pos);
		if (pos_end >= 0)
		{
			if (pos_end > pos)
			{
				ss.insert(str.Mid(pos, pos_end - pos));
			}
		}
		else
		{
			if (str.GetLength() > pos)
			{
				ss.insert(str.Mid(pos));
			}
			break;
		}

		pos = pos_end + split_size;
	}
}

CStringW helper::ConcatString(const StrSet& ss, LPCWSTR pszSpliter)
{
	CStringW strRet;
	for (StrSet::const_iterator it = ss.begin(); it != ss.end(); ++it)
	{
		if (!strRet.IsEmpty()) strRet += pszSpliter;
		strRet += *it;
	}

	return strRet;
}

CStringW helper::GetModuleFilePath()
{
	WCHAR szPath[MAX_PATH];
	::GetModuleFileNameW(GD.module, szPath, _countof(szPath));
	return szPath;
}

CStringW helper::GetModuleDir()
{
	return GetDir(GetModuleFilePath());
}

CStringW helper::GetDir(const CStringW& strPath)
{
	return strPath.Left(strPath.ReverseFind(L'\\'));
}

CStringW helper::GetFileName(LPCWSTR pszFilePath)
{
	if (!pszFilePath) return L"";

	const WCHAR *r = max(wcsrchr(pszFilePath, L'\\'), wcsrchr(pszFilePath, L'/'));
	return r? r+1 : L"";
}

CStringW helper::XML_GetAttributeAsString(IXMLDOMNode* pNode, LPCWSTR pszAttr, LPCWSTR pszDefault)
{
	CComPtr<IXMLDOMNamedNodeMap> pMap;
	CComPtr<IXMLDOMNode> pAttr;
	CComVariant val;

	HRESULT hr;
	hr = pNode->get_attributes(&pMap);
	if (SUCCEEDED(hr))
	{
		hr = pMap->getNamedItem(CComBSTR(pszAttr), &pAttr);
		if (SUCCEEDED(hr) && pAttr)
		{
			hr = pAttr->get_nodeValue(&val);
			{
				return val.bstrVal;
			}
		}
	}

	return pszDefault;
}

UINT helper::XML_GetAttributeAsUint(IXMLDOMNode* pNode, LPCWSTR pszAttr, UINT uDefault)
{
	CStringW strVal = XML_GetAttributeAsString(pNode, pszAttr, L"");
	if (!strVal.IsEmpty())
	{
		return wcstoul(strVal, NULL, 10);
	}
	else
	{
		return uDefault;
	}
}

int helper::XML_GetAttributeAsInt(IXMLDOMNode* pNode, LPCWSTR pszAttr, int nDefault)
{
	CStringW strVal = XML_GetAttributeAsString(pNode, pszAttr, L"");
	if (!strVal.IsEmpty())
	{
		return _wtoi(strVal);
	}
	else
	{
		return nDefault;
	}
}

bool helper::XML_GetAttributeAsBool(IXMLDOMNode* pNode, LPCWSTR pszAttr, bool bDefault)
{
	int v = XML_GetAttributeAsInt(pNode, pszAttr, bDefault?1:0);
	return v == 1? true : false;
}

COLORREF helper::XML_GetAttributeAsColor(IXMLDOMNode* pNode, LPCWSTR pszAttr, COLORREF crDefault)
{
	CStringW strVal = XML_GetAttributeAsString(pNode, pszAttr, L"");
	if (!strVal.IsEmpty())
	{
		int r, g, b;
		swscanf_s(strVal, L"%d,%d,%d", &r, &g, &b);
		return RGB(r, g, b);
	}
	else
	{
		return crDefault;
	}
}

CComPtr<IXMLDOMNode> helper::XML_CreateNode(IXMLDOMDocument* pDoc, int type, LPCWSTR pszName)
{
	CComPtr<IXMLDOMNode> pNode;
	pDoc->createNode(CComVariant(type), CComBSTR(pszName), CComBSTR(L""), &pNode);
	return pNode;
}

bool helper::XML_AddAttribute(IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, LPCWSTR pszAttrName, LPCWSTR pszValue)
{
	CComPtr<IXMLDOMNamedNodeMap> pAttrMap;
	CComPtr<IXMLDOMAttribute> pAttr;

	if (SUCCEEDED(pNode->get_attributes(&pAttrMap)) &&
		SUCCEEDED(pDoc->createAttribute(CComBSTR(pszAttrName), &pAttr)) &&
		SUCCEEDED(pAttrMap->setNamedItem(pAttr, NULL )) &&
		SUCCEEDED(pAttr->put_nodeValue(CComVariant(pszValue))))
	{
		return true;
	}

	return false;
}
bool helper::XML_AddAttributeUint(IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, LPCWSTR pszAttrName, UINT nVal)
{
	CStringW strVal;
	strVal.Format(L"%u", nVal);
	return XML_AddAttribute(pDoc, pNode, pszAttrName, strVal);
}
bool helper::XML_AddAttributeInt(IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, LPCWSTR pszAttrName, int nVal)
{
	CStringW strVal;
	strVal.Format(L"%d", nVal);
	return XML_AddAttribute(pDoc, pNode, pszAttrName, strVal);
}
bool helper::XML_AddAttributeBool(IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, LPCWSTR pszAttrName, bool bVal)
{
	return XML_AddAttributeInt(pDoc, pNode, pszAttrName, bVal?1:0);
}
bool helper::XML_AddAttributeColor(IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, LPCWSTR pszAttrName, COLORREF cr)
{
	CStringW strVal;
	strVal.Format(L"%u,%u,%u", GetRValue(cr), GetGValue(cr), GetBValue(cr));
	return XML_AddAttribute(pDoc, pNode, pszAttrName, strVal);
}

size_t helper::str_distance(const std::wstring& source, const std::wstring& target)
{
	//step 1

	size_t n=source.length();
	size_t m=target.length();
	if (m==0) return n;
	if (n==0) return m;
	//Construct a matrix
	typedef std::vector< std::vector<size_t> > Tmatrix;
	Tmatrix matrix(n+1);
	for(size_t i=0; i<=n; i++) matrix[i].resize(m+1);

	//step 2 Initialize

	for(size_t i=1;i<=n;i++) matrix[i][0]=i;
	for(size_t i=1;i<=m;i++) matrix[0][i]=i;

	//step 3
	for(size_t i=1;i<=n;i++)
	{
		const wchar_t si=source[i-1];
		//step 4
		for(size_t j=1;j<=m;j++)
		{
			const wchar_t dj=target[j-1];
			//step 5
			int cost;
			if(si==dj){
				cost=0;
			}
			else{
				cost=1;
			}
			//step 6
			const size_t above=matrix[i-1][j]+1;
			const size_t left=matrix[i][j-1]+1;
			const size_t diag=matrix[i-1][j-1]+cost;
			matrix[i][j]=min(above,min(left,diag));
		}
	}//step7
	return matrix[n][m];
}

__time64_t helper::GetExpireTime()
{
	HRSRC hSrc = ::FindResourceW(GD.module, L"#1", L"PDATA");
	if (hSrc == NULL) return 0;

	HGLOBAL hResData = ::LoadResource(GD.module, hSrc);
	if (hResData == NULL) return 0;

	LPVOID pData = ::LockResource(hResData);
	if (pData == NULL) return 0;

	return *reinterpret_cast<__time64_t*>((char*)pData + 16);
}

HRESULT helper::WriteFileIfNotExist(LPCWSTR path, LPCWSTR rcname, LPCWSTR rctype)
{
	if (_waccess_s(path, 0) != 0)
	{
		helper::MakeRequiredPath(path);
		return helper::WriteFileFromResource(path, rcname, rctype);
	}

	return S_FALSE;
}

HRESULT helper::WriteFileFromResource(LPCWSTR filename, LPCWSTR name, LPCWSTR type)
{
	HRSRC hSrc = ::FindResourceW(GD.module, name, type);
	if (hSrc == NULL) return E_FAIL;

	HGLOBAL hResData = ::LoadResource(GD.module, hSrc);
	if (hResData == NULL) return E_FAIL;

	DWORD dwSize = ::SizeofResource(GD.module, hSrc);
	LPVOID pData = ::LockResource(hResData);
	if (pData == NULL) return E_FAIL;

	HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return E_FAIL;

	DWORD bytesWrite;
	WriteFile(hFile, pData, dwSize, &bytesWrite, NULL);

	CloseHandle(hFile);
	return S_OK;
}

CStringW helper::GetHelpText()
{
	HRSRC hSrc = ::FindResourceW(GD.module, L"#3", L"PDATA");
	if (hSrc == NULL) return L"";

	HGLOBAL hResData = ::LoadResource(GD.module, hSrc);
	if (hResData == NULL) return L"";

	int ressize = static_cast<int>(::SizeofResource(GD.module, hSrc));
	if (ressize < 2) return L"";

	LPVOID pData = ::LockResource(hResData);
	if (pData == NULL) return L"";

	CStringW strHelp;
	strHelp.SetString(reinterpret_cast<wchar_t*>((char*)pData + 2), (ressize - 2) / 2);
	return strHelp;
}

bool helper::NeedNotifyExpire()
{
/*
	if (!GD.isRealtimeLog)
	{
		return false;
	}

	__time64_t expireTime = GetExpireTime();
	__time64_t currentTime = _time64(NULL);
	if (expireTime < currentTime ||
		expireTime - currentTime < 86400 * 3)
	{
		return true;
	}
*/
	return false;
}

CStringW helper::GetErrorDesc(DWORD dwErr)
{
	WCHAR* msg;
	FormatMessageW(
		FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		dwErr,
		0,
		reinterpret_cast<WCHAR*>(&msg),
		0,
		NULL);

	CStringW strDesc = msg;
	::LocalFree(msg);
	return strDesc;
}

void helper::GUIReportError(LPCWSTR pszError)
{
	DWORD dwErr = GetLastError();
	GUIReportError(pszError, HRESULT_FROM_WIN32(dwErr));
}

void helper::GUIReportError(LPCWSTR pszError, HRESULT hr)
{
	CStringW strInfo = pszError;
	strInfo += L"\n";
	strInfo.AppendFormat(L"LASTERROR: %u\n%s", hr, (LPCWSTR)GetErrorDesc((DWORD)hr));
	::MessageBoxW(NULL, strInfo, IDS(IDS_TPLOGVIEW), MB_OK|MB_ICONERROR);
}

void helper::RunOffline(LPCWSTR pszFilePath)
{
	CStringW strPath;
	if (pszFilePath && *pszFilePath) strPath.Format(L"\"%s\"", pszFilePath);
	else strPath = pszFilePath;
	ShellExecute(NULL, L"open", GetModuleFilePath(), strPath, GetModuleDir(), SW_SHOW);
}

UINT64 helper::GetFileVersion(LPCWSTR path)
{
	DWORD dwSize = GetFileVersionInfoSizeW(path, NULL);
	CAutoVectorPtr<char> buf;
	buf.Allocate(dwSize);
	if (!GetFileVersionInfoW(path, 0, dwSize, buf))
	{
		return 0;
	}

	VS_FIXEDFILEINFO* pInfo = NULL;
	unsigned int nInfoLen;
	if (!VerQueryValue(buf, _T( "\\" ), (LPVOID*)&pInfo, &nInfoLen) || !pInfo)
	{
		return 0;
	}

	return (UINT64(pInfo->dwFileVersionMS) << 32) + pInfo->dwFileVersionLS;
}

CStringW helper::GetVersion()
{
	return IntVersionToString(GetFileVersion(GetModuleFilePath()));
}

bool helper::FtpDownloadFile(LPCSTR pszHost, LPCSTR user, LPCSTR pass, LPCSTR remote_path, LPCSTR remote_file, LPCSTR local_file)
{
	CStringW strDir = GetModuleDir();
	CStringW strFtpCmdFile = strDir + L"\\ftp_cmd";
	CStringA buf;
	buf.Format("open %s\r\n%s\r\n%s\r\nbin\r\ncd  %s\r\nget %s %s\r\nqui\r\n",
		pszHost, user, pass, remote_path, remote_file, local_file);
	HANDLE hFile = CreateFile(strFtpCmdFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesWritten = 0;
		WriteFile(hFile, buf, strlen(buf), &dwBytesWritten, NULL);
		CloseHandle(hFile);
	}

	SHELLEXECUTEINFOW ei = {};
	ei.cbSize = sizeof(ei);
	ei.lpVerb = L"open";
	ei.lpFile = L"ftp.exe";
	ei.lpDirectory = strDir;
	ei.lpParameters = L"-s:ftp_cmd";
	ei.nShow = SW_HIDE;
	ei.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShellExecuteExW(&ei);

	WaitForSingleObject(ei.hProcess, 5000);

	return true;
}

int helper::parse_num(const wchar_t * start, int count, int radix)
{
	int num = 0;
	if (radix == 10)
	{
		for (int i = 0; i < count; i++)
		{
			num *= 10;
			num += start[i] - L'0';
		}
	}
	return num;
}

const wchar_t* helper::wcsistr(const wchar_t * str, const wchar_t * substr)
{
	size_t len = wcslen(str);
	size_t sublen = wcslen(substr);
	for (size_t i = 0; i + sublen <= len; i++)
	{
		if (_wcsnicmp(str + i, substr, sublen) == 0) return str + i;
	}
	return NULL;
}

CStringW helper::ExpandPath(LPCWSTR pszPath)
{
#define CM_ENTRY(x) { x, L#x }
	struct csidlmap
	{
		int csidl;
		LPCWSTR str;
	} cm[] =
	{
		CM_ENTRY(CSIDL_APPDATA),
		CM_ENTRY(CSIDL_COMMON_APPDATA),
		CM_ENTRY(CSIDL_DESKTOP),
		CM_ENTRY(CSIDL_INTERNET),
		CM_ENTRY(CSIDL_PROGRAMS),
		CM_ENTRY(CSIDL_CONTROLS),
		CM_ENTRY(CSIDL_PRINTERS),
		CM_ENTRY(CSIDL_PERSONAL),
		CM_ENTRY(CSIDL_FAVORITES),
		CM_ENTRY(CSIDL_STARTUP),
		CM_ENTRY(CSIDL_RECENT),
		CM_ENTRY(CSIDL_SENDTO),
		CM_ENTRY(CSIDL_BITBUCKET),
		CM_ENTRY(CSIDL_STARTMENU),
		CM_ENTRY(CSIDL_MYDOCUMENTS),
		CM_ENTRY(CSIDL_MYMUSIC),
		CM_ENTRY(CSIDL_MYVIDEO),
		CM_ENTRY(CSIDL_DESKTOPDIRECTORY),
		CM_ENTRY(CSIDL_DRIVES),
		CM_ENTRY(CSIDL_NETWORK),
		CM_ENTRY(CSIDL_NETHOOD),
		CM_ENTRY(CSIDL_FONTS),
		CM_ENTRY(CSIDL_TEMPLATES),
		CM_ENTRY(CSIDL_COMMON_STARTMENU),
		CM_ENTRY(CSIDL_COMMON_PROGRAMS),
		CM_ENTRY(CSIDL_COMMON_STARTUP),
		CM_ENTRY(CSIDL_COMMON_DESKTOPDIRECTORY),
		CM_ENTRY(CSIDL_APPDATA),
		CM_ENTRY(CSIDL_PRINTHOOD),
		CM_ENTRY(CSIDL_LOCAL_APPDATA),
		CM_ENTRY(CSIDL_ALTSTARTUP),
		CM_ENTRY(CSIDL_COMMON_ALTSTARTUP),
		CM_ENTRY(CSIDL_COMMON_FAVORITES),
		CM_ENTRY(CSIDL_INTERNET_CACHE),
		CM_ENTRY(CSIDL_COOKIES),
		CM_ENTRY(CSIDL_HISTORY),
		CM_ENTRY(CSIDL_COMMON_APPDATA),
		CM_ENTRY(CSIDL_WINDOWS),
		CM_ENTRY(CSIDL_SYSTEM),
		CM_ENTRY(CSIDL_PROGRAM_FILES),
		CM_ENTRY(CSIDL_MYPICTURES),
		CM_ENTRY(CSIDL_PROFILE),
		CM_ENTRY(CSIDL_SYSTEMX86),
		CM_ENTRY(CSIDL_PROGRAM_FILESX86),
		CM_ENTRY(CSIDL_PROGRAM_FILES_COMMON),
		CM_ENTRY(CSIDL_PROGRAM_FILES_COMMONX86),
		CM_ENTRY(CSIDL_COMMON_TEMPLATES),
		CM_ENTRY(CSIDL_COMMON_DOCUMENTS),
		CM_ENTRY(CSIDL_COMMON_ADMINTOOLS),
		CM_ENTRY(CSIDL_ADMINTOOLS),
		CM_ENTRY(CSIDL_CONNECTIONS),
		CM_ENTRY(CSIDL_COMMON_MUSIC),
		CM_ENTRY(CSIDL_COMMON_PICTURES),
		CM_ENTRY(CSIDL_COMMON_VIDEO),
		CM_ENTRY(CSIDL_RESOURCES),
		CM_ENTRY(CSIDL_RESOURCES_LOCALIZED),
		CM_ENTRY(CSIDL_COMMON_OEM_LINKS),
		CM_ENTRY(CSIDL_CDBURN_AREA),
		CM_ENTRY(CSIDL_COMPUTERSNEARME),
	};

	if (!pszPath) return L"";
	CStringW ret;

	LPCWSTR p = pszPath;
	while (*p)
	{
		if (*p != L'<') ret += *p++;
		else
		{
			LPCWSTR q = wcschr(p, L':');
			if (!q) break;

			LPCWSTR r = wcschr(q, L'>');
			if (!r) break;

			p++;
			CStringW sn(p, q-p);
			if (sn == L"CSIDL")
			{
				CStringW idl = sn + L"_" + CStringW(q+1, r-q-1);
				for (size_t i = 0; i < _countof(cm); i++)
				{
					if (cm[i].str == idl)
					{
						WCHAR path[MAX_PATH];
						::SHGetFolderPathW(NULL, cm[i].csidl, NULL, SHGFP_TYPE_CURRENT, path);
						ret += path;
						break;
					}
				}
			}
			else if (sn == L"REG")
			{
				CStringW path(q+1, r-q-1);
				int pos1 = path.Find(L'\\');
				int pos2 = path.ReverseFind(L'\\');
				if (pos1 < 0 || pos2 < 0) break;

				CStringW root = path.Mid(0, pos1);
				CStringW key = path.Mid(pos1+1, pos2-pos1-1);
				CStringW name = path.Mid(pos2+1);
				HKEY hRoot = (root == L"HKLM")? HKEY_LOCAL_MACHINE: HKEY_CURRENT_USER;
				HKEY hKey;
				if (::RegOpenKeyExW(hRoot, key, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
				{
					DWORD type;
					WCHAR szPath[MAX_PATH];
					DWORD pathlen = sizeof(szPath);
					if (::RegQueryValueExW(hKey, name, NULL, &type, (LPBYTE)szPath, &pathlen) == ERROR_SUCCESS)
					{
						ret += szPath;
					}
					::RegCloseKey(hKey);
				}
			}
			else if (sn == L"VAR")
			{
				CStringW varname(q+1, r-q-1);
				if (varname == L"PRODUCT")
				{
				}
			}

			p = r + 1;
		}
	}

	WCHAR dst[MAX_PATH];
	::ExpandEnvironmentStringsW(ret, dst, _countof(dst));
	return dst;
}

CStringW helper::GetLogLevelDescription(UINT level)
{
	if (level == 0) return IDS(IDS_LOWEST);
	return ServiceHelper::GetLogPropertyDB()->GetLevelDesc(level);
}

void helper::bool_reverse(bool& val)
{
	val = !val;
}

void helper::SplitTag(const wchar_t* tags, str_segment (&segs)[256])
{
	str_segment* p = segs;
	p->start = tags;
	for(;;)
	{
		if (*tags == L';' || *tags == L' ' || *tags == L'+' || !*tags)
		{
			p->end = tags;
			if (p->start != p->end && p < segs + _countof(segs)) p++;
			p->start = tags + 1;
		}
		if (!*tags) break;
		tags++;
	}
	p->start = NULL;
	p->end = NULL;
}

bool helper::TagMatch(const wchar_t* tags, const wchar_t* matcher)
{
	str_segment ss_tag[256];
	str_segment ss_matcher[256];

	if (!matcher || !*matcher) return true;

	SplitTag(tags, ss_tag);
	SplitTag(matcher, ss_matcher);

	bool match_all = wcschr(matcher, L'+') != NULL;

	for (str_segment* p = ss_matcher; p->start; p++)
	{
		bool match_one = false;
		for (str_segment* q = ss_tag; q->start; q++)
		{
			size_t len = static_cast<size_t>(p->end - p->start);
			if (q->start + len == q->end &&
				wcsncmp(p->start, q->start, len) == 0)
			{
				match_one = true;
				break;
			}
		}
		if (match_all && !match_one) return false;
		if (!match_all && match_one) return true;
	}

	if (match_all) return true;
	return false;
}

bool helper::MakeRequiredPath(LPCWSTR pcszPath)
{
	CStringW strPath = pcszPath;

	BOOL bRet = FALSE;
	for (int pos = 0; pos >= 0; pos = strPath.Find(L'\\', pos+1))
	{
		CStringW strDir = strPath.Left(pos);
		if (!strDir.IsEmpty())
		{
			bRet = ::CreateDirectoryW(strDir, NULL);
		}
	}

	return bRet? true: false;
}

bool helper::FileExists(LPCWSTR path)
{
	return _waccess_s(path, 0) == 0;
}

static CStringW GetSpecialDir(int csidl, LPCWSTR subdir)
{
	CStringW strPath;

	WCHAR szPath[MAX_PATH];
	SHGetFolderPathW(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, szPath);
	strPath = szPath;

	strPath += L"\\";
	strPath += subdir;
	::SHCreateDirectory(NULL, strPath);

	return strPath;
}

CStringW helper::GetConfigDir()
{
	return GetModuleDir();
}

bool helper::DownloadUrlToFile(LPCWSTR url, LPCWSTR path)
{
	HINTERNET hnet = ::InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hnet) return false;
	ON_LEAVE_1(::InternetCloseHandle(hnet), HINTERNET, hnet);

	HINTERNET hurl = ::InternetOpenUrlW(hnet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hurl) return false;
	ON_LEAVE_1(::InternetCloseHandle(hurl), HINTERNET, hurl);

	HANDLE hFile = ::CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return false;
	ON_LEAVE_1(::CloseHandle(hFile), HANDLE, hFile);

	CHAR buffer[4096];
	DWORD bytesRead;
	do {
		bytesRead = 0;
		DWORD bytesWrite = 0;
		BOOL bRet = ::InternetReadFile(hurl, (LPVOID)buffer, _countof(buffer), &bytesRead);
		if (!bRet) break;
		::WriteFile(hFile, buffer, bytesRead, &bytesWrite, NULL);
	} while (bytesRead > 0);

	return true;
}

UINT64 helper::StringVersionToInt(CStringW ver)
{
	UINT a,b,c,d;
	swscanf_s(ver, L"%u.%u.%u.%u", &a, &b, &c, &d);
	UINT32 h = (a << 16) + b;
	UINT32 l = (c << 16) + d;
	return ((UINT64)h << 32) + l;
}

CStringW helper::IntVersionToString(UINT64 ver)
{
	UINT32 h = static_cast<UINT32>(ver >> 32);
	UINT32 l = static_cast<UINT32>(ver & 0xFFFFFFFF);
	CStringW ret;
	ret.Format(L"%u.%u.%u.%u", HIWORD(h), LOWORD(h), HIWORD(l), LOWORD(l));
	return ret;
}

INT64 helper::GetElapsedTime(const LogInfo& li1, const LogInfo& li2)
{
	return accutime(li2.item->log_time_sec, li2.item->log_time_msec) - accutime(li1.item->log_time_sec, li1.item->log_time_msec);
}

COLORREF helper::GetGradientColor(COLORREF c1, COLORREF c2, double ratio)
{
	int r1 = GetRValue(c1);
	int b1 = GetBValue(c1);
	int g1 = GetGValue(c1);
	int r2 = GetRValue(c2);
	int b2 = GetBValue(c2);
	int g2 = GetGValue(c2);

	if (ratio > 1) ratio = 1;

	int r = r1 + static_cast<int>((r2 - r1) * ratio);
	int g = g1 + static_cast<int>((g2 - g1) * ratio);
	int b = b1 + static_cast<int>((b2 - b1) * ratio);

	return RGB(r, g, b);
}

CDlgItem::CDlgItem(HWND hDlg, int ctrlid)
{
	m_hWnd = ::GetDlgItem(hDlg, ctrlid);
}

bool CDlgItem::GetCheck()
{
	return SendMessage(BM_GETCHECK, 0, 0L) == BST_CHECKED;
}

void CDlgItem::SetCheck(bool val)
{
	SendMessage(BM_SETCHECK, WPARAM(val? BST_CHECKED : BST_UNCHECKED), 0L);
}

CStringW CDlgItem::GetText()
{
	CStringW text;
	GetWindowText(text);
	return text;
}

void CDlgItem::SetText(LPCWSTR text)
{
	SetWindowText(text);
}

CStringW helper::GetTextFileContent(LPCWSTR path)
{
	CStringW content;
	FILE* fp = NULL;
	_wfopen_s(&fp, path, L"rt,ccs=UNICODE");
	if (fp)
	{
		wchar_t line[4096];
		while (fgetws(line, _countof(line), fp) != NULL)
		{
			content += line;
		}
		fclose(fp);
	}
	return content;
}

HRESULT helper::SaveTextContentToFile(LPCWSTR path, LPCWSTR content)
{
	FILE* fp = NULL;
	_wfopen_s(&fp, path, L"wt,ccs=UTF-8");
	if (fp)
	{
		fwprintf_s(fp, L"%s", content);
		fclose(fp);
		return S_OK;
	}

	return helper::GetLastErrorAsHRESULT();
}

HRESULT helper::GetLastErrorAsHRESULT()
{
	DWORD err = ::GetLastError();
	return HRESULT_FROM_WIN32(err);
}
