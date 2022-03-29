#include "stdafx.h"
#include <Windows.h>
#include "util.h"
#include "atlconv.h"
#include <ShlObj_core.h>



std::wstring  GetCurrentFileName()
{
	TCHAR strExeFullDir[MAX_PATH];
	GetModuleFileName(NULL, strExeFullDir, MAX_PATH);
	std::wstring fullPath = std::wstring(strExeFullDir);
	size_t start = fullPath.find_last_of(L"\\");
	size_t end = fullPath.find_last_of(L"\.");
	std::wstring filename = fullPath.substr(start + 1, end - start - 1);
	return filename;

}

std::wstring  GetCurrentPath()
{
	TCHAR strExeFullDir[MAX_PATH];
	GetModuleFileName(NULL, strExeFullDir, MAX_PATH);
	std::wstring fullPath = std::wstring(strExeFullDir);
	size_t end = fullPath.find_last_of(L"\\");
	std::wstring path = fullPath.substr(0, end);
	return path;

}

HRESULT CreateShortCut(LPCWSTR lpszPathObj, WCHAR* lpszPathLink, LPCWSTR lpszDesc)
{
	HRESULT hres;
	IShellLink* psl;

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		psl->SetPath(lpszPathObj);
		psl->SetDescription(lpszDesc);

		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

		if (SUCCEEDED(hres))
		{
			hres = ppf->Save(lpszPathLink, TRUE);

			ppf->Release();
		}
		psl->Release();
	}
	return hres;
}

std::wstring CATOW(const char* lpcszString)
{
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);//获取字符串长度


	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));


	::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);//转换
	std::wstring wString = (wchar_t*)pUnicode;
	delete[] pUnicode;
	return wString;
}

bool CreatShortCutToDesktop()
{

	WCHAR fullPath[MAX_PATH];
	GetModuleFileName(NULL, fullPath, MAX_PATH);

	WCHAR desktopPathW[MAX_PATH] = { 0 };
	SHGetSpecialFolderPath(NULL, desktopPathW, CSIDL_DESKTOP, 0);


	std::wstring desktopPath = std::wstring(desktopPathW);
	std::wstring name = CATOW("\\熄屏助手.lnk");
	desktopPath.append(name);

	HRESULT a = CreateShortCut(fullPath, (WCHAR*)desktopPath.c_str(), L"");
	return true;
}

void SetStartupWithWindows()
{
	HKEY hKey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) ///打开启动项       
	{
		TCHAR strExeFullDir[MAX_PATH];
		GetModuleFileName(NULL, strExeFullDir, MAX_PATH);
		std::wstring filename = GetCurrentFileName();

		TCHAR strDir[MAX_PATH] = {};
		DWORD nLength = MAX_PATH;
		long result = RegGetValue(hKey, nullptr, filename.c_str(), RRF_RT_REG_SZ, 0, strDir, &nLength);

		if (result != ERROR_SUCCESS || _tcscmp(strExeFullDir, strDir) != 0)
		{
			RegSetValueEx(hKey, filename.c_str(), 0, REG_SZ, (LPBYTE)strExeFullDir, (lstrlen(strExeFullDir) + 1) * sizeof(TCHAR));
			RegCloseKey(hKey);
		}
	}
}

void CancleStartupWithWindows()
{
	HKEY hKey;
	std::wstring filename = GetCurrentFileName();

	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		RegDeleteValue(hKey, filename.c_str());
		RegCloseKey(hKey);
	}
}

bool isWin7()
{
	return WINVER == 0x0601;
}

bool IsGreaterThanOrEqualToWin8()
{
	return WINVER >= 0x0602;
}

bool SaveIntData(std::wstring name, int value)
{
	std::wstring path = GetCurrentPath() + L"\\config.ini";
	std::wstring sectionName = L"config";
	std::wstring val = nbase::StringPrintf(L"%d", value);
	bool ret = WritePrivateProfileString(sectionName.c_str(), name.c_str(), val.c_str(), path.c_str());
	return ret;
}

int GetIntData(std::wstring name, int defaultValue)
{
	std::wstring path = GetCurrentPath() + L"\\config.ini";
	std::wstring sectionName = L"config";
	int ret = defaultValue;
	try
	{
		ret = GetPrivateProfileInt(sectionName.c_str(), name.c_str(), defaultValue, path.c_str());
	}
	catch (const std::exception&)
	{

	}
	return ret;
}
