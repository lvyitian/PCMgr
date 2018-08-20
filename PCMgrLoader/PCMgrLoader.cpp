#include "stdafx.h"
#include "PCMgrLoader.h"

const wchar_t* mainDllName = 0;
WCHAR iniPath[MAX_PATH];

void show_err(const wchar_t* err)
{
	MessageBox(NULL, (LPWSTR)err, PCMGRTITLE, MB_ICONERROR);
}

typedef long(NTAPI* fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
typedef void* (__cdecl *fnmemset)(void*  _Dst, int _Val, size_t _Size);
typedef DWORD(*_MAppMainGetExitCode)();
typedef void(*_MAppMainRun)();

fnRtlGetVersion RtlGetVersion;
fnmemset _memset;
_MAppMainGetExitCode MAppMainGetExitCode;
_MAppMainRun MAppMainRun;

HMODULE hMain;

bool IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
	RTL_OSVERSIONINFOEXW verInfo;
	verInfo.dwOSVersionInfoSize = sizeof(verInfo);
	_memset(&verInfo, 0, sizeof(verInfo));

	if (RtlGetVersion != 0 && RtlGetVersion((PRTL_OSVERSIONINFOW)&verInfo) == 0)
	{
		if (verInfo.dwMajorVersion > wMajorVersion)
			return true;
		else if (verInfo.dwMajorVersion < wMajorVersion)
			return false;
		if (verInfo.dwMinorVersion > wMinorVersion)
			return true;
		else if (verInfo.dwMinorVersion < wMinorVersion)
			return false;
		if (verInfo.wServicePackMajor >= wServicePackMajor)
			return true;
	}

	return false;
}

bool load_test_system_support()
{
	if (!IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0))
	{
		show_err(L"Application not support your Windows, Running this program requires Windows 7 at least.");
		return false;
	}	
	return true;
}

int main()
{
	RtlGetVersion = (fnRtlGetVersion)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
	_memset = (fnmemset)GetProcAddress(LoadLibrary(L"msvcrt.dll"), "memset");

	if (!load_test_system_support())return -1;

#ifdef _AMD64_
	mainDllName = L"PCMgr64.dll";
#else
	mainDllName = L"PCMgr32.dll";
#endif
	hMain = LoadLibrary(mainDllName);
	if (!hMain)
	{
		WCHAR errStr[64];
		wsprintf(errStr, L"Can not load %s !\n", mainDllName);
		show_err(errStr);
		return -1;
	}

	MAppMainGetExitCode = (_MAppMainGetExitCode)GetProcAddress(hMain, "MAppMainGetExitCode");
	MAppMainRun = (_MAppMainRun)GetProcAddress(hMain, "MAppMainRun");

	MAppMainRun();

	return MAppMainGetExitCode();
}
int main_entry() {
	int rs = main();
	ExitProcess(rs);
	return rs;
}







