/////////////////////////////////////////////////////////////////////////////
// Startup code for BSynthComposer running on MS-Windows. We read the project
// options, create a main frame window, and run the message loop. Various global
// housekeeping chores are done here as well.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>

#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"

SynthAppModule _Module;

const GUID DSDEVID_DefaultPlayback = {0xdef00000, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03};


int Run(LPTSTR cmd = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	theProject = NULL;

	SetCurrentDirectory(prjOptions.defPrjDir);

	MainFrame wndMain;

	RECT rc;
	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);
	if (sw < 1024)
	{
		rc.left = 0;
		rc.right = sw;
	}
	else
	{
		rc.left = (sw - 1024) / 2;
		rc.right = rc.left + 1024;
	}
	if (sh < 768)
	{
		rc.top = 0;
		rc.bottom = sh;
	}
	else
	{
		rc.top = (sh - 768) / 2;
		rc.bottom = rc.top + 768;
	}
	if(wndMain.CreateEx(HWND_DESKTOP, rc) == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	if (cmd && *cmd)
	{
		while (*cmd == ' ')
			cmd++;
		if (*cmd== '"')
		{
			char *quo = strchr(++cmd, '"');
			if (quo)
				*quo = 0;
		}
		if (wndMain.OpenProject(cmd))
			wndMain.AfterOpenProject();
	}

	wndMain.ShowWindow(nCmdShow);
	//wndMain.ShowWindow(SW_MAXIMIZE);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

static BOOL CALLBACK FindWaveDevice(LPGUID lpGUID, 
             LPCTSTR lpszDesc,
             LPCTSTR lpszDrvName, 
             LPVOID lpContext)
{
	if (strcmp(lpszDesc, prjOptions.waveDevice) == 0)
	{
		if (_Module.waveID)
			delete _Module.waveID;
		if (lpGUID)
		{
			GUID *newGUID = new GUID;
			memcpy(newGUID, lpGUID, sizeof(GUID));
			_Module.waveID = newGUID;
		}
		else
			_Module.waveID = NULL;
		return FALSE;
	}
	return TRUE;
}

static int xtoi(const char *p)
{
	int h;
	int x = 0;
	while (*p)
	{
		if (*p >= 0 && *p <= '9')
			h = *p - '0';
		else if (*p >= 'A' && *p <= 'F')
			h = (*p - 'A') + 10;
		else if (*p >= 'a' && *p <= 'f')
			h = (*p - 'a') + 10;
		else
			break;
		x = (x << 4) | h;
		p++;
	}
	return x;
}

void ProjectOptions::Load()
{
	_Module.waveID = NULL;

	bsString bsKey;
	bsKey = "Software\\";
	bsKey += _Module.ProductName;

	CRegKey rk;
	ULONG len = MAX_PATH;
	char curdir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, curdir);

	DWORD dwval;
	char module[MAX_PATH];
	GetModuleFileName(NULL, module, MAX_PATH);
	SynthProject::NormalizePath(module);
	char *base = strrchr(module, '/');
	if (base)
		*base = 0;

	len = MAX_PATH;
	GetUserName(defAuthor, &len);
	SYSTEMTIME systm;
	GetSystemTime(&systm);
	snprintf(defCopyright, MAX_PATH, "Copyright %d", systm.wYear);

	if (rk.Open(HKEY_LOCAL_MACHINE, bsKey, KEY_READ) == ERROR_SUCCESS)
	{
		len = MAX_PATH;
		rk.QueryStringValue("Install", installDir, &len);
		len = MAX_PATH;
		rk.QueryStringValue("Forms", formsDir, &len);
		len = MAX_PATH;
		rk.QueryStringValue("Colors", colorsFile, &len);
		len = MAX_PATH;
		rk.QueryStringValue("Instrlib", defLibDir, &len);
		rk.Close();
	}

	if (rk.Open(HKEY_CURRENT_USER, bsKey, KEY_ALL_ACCESS) == ERROR_SUCCESS)
	{
		len = MAX_PATH;
		rk.QueryStringValue("Projects", defPrjDir, &len);
		len = MAX_PATH;
		rk.QueryStringValue("Forms", formsDir, &len);
		len = MAX_PATH;
		rk.QueryStringValue("Colors", colorsFile, &len);
		len = MAX_PATH;
		rk.QueryStringValue("WaveIn", defWaveIn, &len);
		if (rk.QueryDWORDValue("InclNotelist", dwval) == ERROR_SUCCESS)
			inclNotelist = dwval;
		if (rk.QueryDWORDValue("InclSequence", dwval) == ERROR_SUCCESS)
			inclSequence = dwval;
		if (rk.QueryDWORDValue("InclScripts", dwval) == ERROR_SUCCESS)
			inclScripts = dwval;
		if (rk.QueryDWORDValue("InclTextFiles", dwval) == ERROR_SUCCESS)
			inclTextFiles = dwval;
		if (rk.QueryDWORDValue("InclLibraries", dwval) == ERROR_SUCCESS)
			inclLibraries = dwval;
		if (rk.QueryDWORDValue("InclSoundFonts", dwval) == ERROR_SUCCESS)
			inclSoundFonts = dwval;
		char buf[80];
		len = 80;
		if (rk.QueryStringValue("Latency", buf, &len) == ERROR_SUCCESS)
			playBuf = atof(buf);
		len = 80;
		if (rk.QueryStringValue("InclInstruments", buf, &len) == ERROR_SUCCESS)
			inclInstr = xtoi(buf);
		len = MAX_PATH;
		rk.QueryStringValue("Author", defAuthor, &len);
		len = MAX_PATH;
		rk.QueryStringValue("Copyright", prjOptions.defCopyright, &len);
		len = MAX_PATH;
		rk.QueryStringValue("MIDIDeviceName", prjOptions.midiDeviceName, &len);
		if (rk.QueryDWORDValue("MIDIDevice", dwval) == ERROR_SUCCESS)
			midiDevice = (int) dwval;
		len = MAX_PATH;
		rk.QueryStringValue("WaveDevice", prjOptions.waveDevice, &len);
		rk.Close();
	}
	else
	{
		HRESULT hr;
		hr = SHGetFolderPathAndSubDir(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, _Module.ProductName, defPrjDir);
		if (hr != S_OK)
			hr = SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, _Module.ProductName, defPrjDir);
	}
	if (defPrjDir[0] == 0)
	{
		strncpy(defPrjDir, curdir, MAX_PATH);
	}

	if (installDir[0] == 0)
	{
		strncpy(installDir, module, MAX_PATH);
	}
	if (formsDir[0] == 0)
	{
		strncpy(formsDir, installDir, MAX_PATH-7);
		strcat(formsDir, "\\Forms");
	}
	if (colorsFile[0] == 0)
	{
		strncpy(colorsFile, "Colors.xml", MAX_PATH);
	}
	if (defLibDir[0] == 0)
	{
		strncpy(defLibDir, installDir, MAX_PATH-10);
		strcat(defLibDir, "\\Instrlib");
	}

	DirectSoundEnumerate(FindWaveDevice, NULL);
}

void ProjectOptions::Save()
{
	bsString bsKey;
	bsKey = "Software\\";
	bsKey += _Module.ProductName;
	CRegKey rk;
	if (rk.Open(HKEY_CURRENT_USER, bsKey, KEY_ALL_ACCESS) != ERROR_SUCCESS)
		rk.Create(HKEY_CURRENT_USER, bsKey);
	rk.SetStringValue("Projects", defPrjDir);
	rk.SetStringValue("Forms", formsDir);
	rk.SetStringValue("Colors", colorsFile);
	rk.SetStringValue("WaveIn", defWaveIn);
	rk.SetStringValue("Author", defAuthor);
	rk.SetStringValue("Copyright", defCopyright);
	rk.SetDWORDValue("InclNotelist", inclNotelist);
	rk.SetDWORDValue("InclSequence", inclSequence);
	rk.SetDWORDValue("InclScripts", inclScripts);
	rk.SetDWORDValue("InclTextFiles", inclTextFiles);
	rk.SetDWORDValue("InclLibraries", inclLibraries);
	rk.SetDWORDValue("InclSoundFonts", inclSoundFonts);
	char buf[40];
	snprintf(buf, 40, "%x", inclInstr);
	rk.SetStringValue("InclInstruments", buf);
	snprintf(buf, 40, "%f", playBuf);
	rk.SetStringValue("Latency", buf);
	rk.SetDWORDValue("MIDIDevice", (DWORD)midiDevice);
	rk.SetStringValue("MIDIDeviceName", midiDeviceName);
	rk.SetStringValue("WaveDevice", prjOptions.waveDevice);
	rk.Close();
	DirectSoundEnumerate(FindWaveDevice, NULL);
}

static void bad_parameter(
   const wchar_t * expression,
   const wchar_t * function, 
   const wchar_t * file, 
   unsigned int line,
   uintptr_t pReserved)
{
#ifdef _DEBUG
	OutputDebugStringW(expression);
#endif
}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);
	// this prevents the CRT library from kicking us out if there are bad parameters
	_set_invalid_parameter_handler(bad_parameter);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls
	LoadEditorDLL();

	_Module.mainWnd = 0;
	hRes = _Module.Init(NULL, hInstance);

	knobCache = new WidgetImageCache;
	switchCache = new WidgetImageCache;

	int nRet = Run(lpstrCmdLine, nCmdShow);

	delete switchCache;
	delete knobCache;

	GdiplusShutdown(gdiplusToken);
	_Module.Term();
	::CoUninitialize();

	return nRet;
}
