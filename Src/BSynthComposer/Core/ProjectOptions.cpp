//////////////////////////////////////////////////////////////////////
// BasicSynth - Base class for project items.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

ProjectOptions::ProjectOptions()
{
	strcpy(programName, "BasicSynth");
	memset(installDir, 0, MAX_PATH);
	memset(formsDir, 0, MAX_PATH);
	strcpy(colorsFile, "Colors.xml");
	memset(defAuthor, 0, MAX_PATH);
	memset(defCopyright, 0, MAX_PATH);
	memset(defPrjDir, 0, MAX_PATH);
	memset(defLibDir, 0, MAX_PATH);
	memset(defWaveIn, 0, MAX_PATH);
	memset(defWaveOut, 0, MAX_PATH);
	inclNotelist = 1;
	inclSequence = 0;
	inclScripts = 0;
	inclTextFiles = 0;
	inclLibraries = 0;
	inclSoundFonts = 1;
	inclMIDI = 1;
	inclInstr = -1;
	midiDevice = -1;
	memset(midiDeviceName, 0, MAX_PATH);
	memset(waveDevice, 0, MAX_PATH);
	playBuf = 0.1;
	frmLeft = 0;
	frmTop = 0;
	frmWidth = 1024;
	frmHeight = 768;
	frmMax = 0;
#if defined(_WIN32) && _WIN32
	dsoundHWND = 0;
	waveID = 0;
	HMODULE h = LoadLibrary("dsound.dll");
	if (h)
		pDirectSoundEnumerate = (tDirectSoundEnumerate)GetProcAddress(h, "DirectSoundEnumerateA");
	if (pDirectSoundEnumerate == NULL)
		MessageBox(HWND_DESKTOP, "Cannot locate dsound.dll.", "Sorry...", MB_OK);
#endif
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

#if defined(_WIN32) && _WIN32
#include <Mmsystem.h>
#include <Mmreg.h>

static BOOL CALLBACK FindWaveDevice(LPGUID lpGUID,
             LPCTSTR lpszDesc,
             LPCTSTR lpszDrvName,
             LPVOID lpContext)
{
	if (strcmp(lpszDesc, prjOptions.waveDevice) == 0)
	{
		if (prjOptions.waveID)
			delete prjOptions.waveID;
		if (lpGUID)
		{
			GUID *newGUID = new GUID;
			memcpy(newGUID, lpGUID, sizeof(GUID));
			prjOptions.waveID = newGUID;
		}
		else
			prjOptions.waveID = NULL;
		return FALSE;
	}
	return TRUE;
}
#endif

#if defined(WIN32_REGISTRY) && WIN32_REGISTRY
#include <ShlObj.h>
//#include <dsound.h>

class RegKey
{
public:
	HKEY hk;
	RegKey()
	{
		hk = 0;
	}

	int Open(HKEY parent, const char *key)
	{
		return ::RegOpenKeyEx(parent, key, 0, KEY_ALL_ACCESS, &hk) == ERROR_SUCCESS;
	}

	int Create(HKEY parent, const char *key)
	{
		DWORD dw = 0;
		return ::RegCreateKeyExA(parent, key, 0, 0, 0, KEY_ALL_ACCESS, 0, &hk, &dw);
	}

	int QueryValue(const char *key, char *value, DWORD len = MAX_PATH)
	{
		DWORD type = 0;
		return ::RegQueryValueEx(hk, key, NULL, &type, (LPBYTE)value, &len) == ERROR_SUCCESS;
	}

	int QueryValue(const char *key, int& value)
	{
		DWORD dwval = 0;
		DWORD type;
		DWORD len = 4;
		if (::RegQueryValueEx(hk, key, NULL, &type, (LPBYTE)&dwval, &len) == ERROR_SUCCESS
		 && type == REG_DWORD)
		{
			value = (int) dwval;
			return 1;
		}
		return 0;
	}

	int QueryValue(const char *key, float& value)
	{
		char buf[40];
		if (QueryValue(key, buf, 40))
		{
			value = atof(buf);
			return 1;
		}
		return 0;
	}

	int SetValue(const char *key, char *value)
	{
		return ::RegSetValueEx(hk, key, NULL, REG_SZ, (LPBYTE)value, (DWORD)strlen(value));
	}

	int SetValue(const char *key, int value)
	{
		return ::RegSetValueEx(hk, key, NULL, REG_DWORD, (LPBYTE)&value, 4);
	}

	int SetValue(const char *key, float value)
	{
		char buf[40];
		snprintf(buf, 40, "%f", value);
		return SetValue(key, buf);
	}

	void Close()
	{
		::RegCloseKey(hk);
		hk = 0;
	}
};

void ProjectOptions::Load()
{
	bsString bsKey;
	bsKey = "Software\\";
	bsKey += programName;

	RegKey rk;
	ULONG len = MAX_PATH;
	char buf[MAX_PATH];

	char curdir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, curdir);

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

	if (rk.Open(HKEY_LOCAL_MACHINE, bsKey))
	{
		rk.QueryValue("Install", installDir);
		rk.QueryValue("Forms", formsDir);
		rk.QueryValue("Colors", colorsFile);
		rk.QueryValue("Instrlib", defLibDir);
		rk.Close();
	}

	if (rk.Open(HKEY_CURRENT_USER, bsKey))
	{
		rk.QueryValue("Projects", defPrjDir);
		rk.QueryValue("Forms", formsDir);
		rk.QueryValue("Colors", colorsFile);
		rk.QueryValue("WaveIn", defWaveIn);
		rk.QueryValue("InclNotelist", inclNotelist);
		rk.QueryValue("InclSequence", inclSequence);
		rk.QueryValue("InclScripts", inclScripts);
		rk.QueryValue("InclTextFiles", inclTextFiles);
		rk.QueryValue("InclLibraries", inclLibraries);
		rk.QueryValue("InclSoundFonts", inclSoundFonts);
		rk.QueryValue("Latency", playBuf);
		if (rk.QueryValue("InclInstruments", buf))
			inclInstr = xtoi(buf);
		rk.QueryValue("Author", defAuthor);
		rk.QueryValue("Copyright", defCopyright);
		rk.QueryValue("MIDIDeviceName", midiDeviceName);
		rk.QueryValue("MIDIDevice", midiDevice);
		rk.QueryValue("WaveDevice", waveDevice);
		rk.QueryValue("Top", frmTop);
		rk.QueryValue("Left", frmLeft);
		rk.QueryValue("Width", frmWidth);
		rk.QueryValue("Height", frmHeight);
		rk.QueryValue("Maximize", frmMax);
		rk.Close();
	}
#ifdef SHGFP_TYPE_CURRENT
	else
	{
		HRESULT hr;
		hr = SHGetFolderPathAndSubDir(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, programName, defPrjDir);
		if (hr != S_OK)
			hr = SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, programName, defPrjDir);
	}
#endif
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
	SynthProject::NormalizePath(installDir);
	SynthProject::NormalizePath(defPrjDir);
	SynthProject::NormalizePath(defLibDir);
	SynthProject::NormalizePath(defWaveIn);
	SynthProject::NormalizePath(formsDir);
	if (pDirectSoundEnumerate)
		pDirectSoundEnumerate(FindWaveDevice, NULL);
}

void ProjectOptions::Save()
{
	bsString bsKey;
	bsKey = "Software\\";
	bsKey += programName;
	RegKey rk;
	if (!rk.Open(HKEY_CURRENT_USER, bsKey))
		rk.Create(HKEY_CURRENT_USER, bsKey);
	rk.SetValue("Projects", defPrjDir);
	rk.SetValue("Forms", formsDir);
	rk.SetValue("Colors", colorsFile);
	rk.SetValue("WaveIn", defWaveIn);
	rk.SetValue("Author", defAuthor);
	rk.SetValue("Copyright", defCopyright);
	rk.SetValue("InclNotelist", inclNotelist);
	rk.SetValue("InclSequence", inclSequence);
	rk.SetValue("InclScripts", inclScripts);
	rk.SetValue("InclTextFiles", inclTextFiles);
	rk.SetValue("InclLibraries", inclLibraries);
	rk.SetValue("InclSoundFonts", inclSoundFonts);
	char buf[40];
	snprintf(buf, 40, "%x", inclInstr);
	rk.SetValue("InclInstruments", buf);
	rk.SetValue("Latency", playBuf);
	rk.SetValue("MIDIDevice", midiDevice);
	rk.SetValue("MIDIDeviceName", midiDeviceName);
	rk.SetValue("WaveDevice", waveDevice);
	rk.SetValue("Top", frmTop);
	rk.SetValue("Left", frmLeft);
	rk.SetValue("Width", frmWidth);
	rk.SetValue("Height", frmHeight);
	rk.SetValue("Maximize", frmMax);
	rk.Close();
	if (pDirectSoundEnumerate)
		pDirectSoundEnumerate(FindWaveDevice, NULL);
}

#else

static int GetMIDIDevice(const char *name)
{
	if (*name == 0)
		return -1;
#ifdef _WIN32
	UINT ndev = midiInGetNumDevs();
	for (UINT n = 0; n < ndev; n++)
	{
		MIDIINCAPS caps;
		memset(&caps, 0, sizeof(caps));
		midiInGetDevCaps(n, &caps, sizeof(caps));
		if (strcmp(caps.szPname, name) == 0)
			return n;
	}
#endif
	return 0;
}

void ProjectOptions::Load()
{
	char *hm = getenv("HOME");
	if (hm == 0)
		hm = getenv("HOMEPATH"); // Windows name for HOME
	if (hm == 0)
		hm = ".";

	strcpy(installDir, hm);
	strcat(installDir, "/basicsynth");

	strcpy(waveDevice, "default");

	bsString cfgFile;
	cfgFile = hm;
	cfgFile += "/.bsynth";
	FILE *fp = fopen(cfgFile, "r");
	if (fp)
	{
		char lnbuf[1024];
		while (fgets(lnbuf, 1024, fp))
		{
			char *eq = strchr(lnbuf, '\n');
			if (eq)
				*eq = 0;
			if ((eq = strchr(lnbuf, '=')) != NULL)
			{
				*eq++ = 0;
				if (strcmp(lnbuf, "Install") == 0)
					strcpy(installDir, eq);
				else if (strcmp(lnbuf, "Forms") == 0)
					strcpy(formsDir, eq);
				else if (strcmp(lnbuf, "Colors") == 0)
					strcpy(colorsFile, eq);
				else if (strcmp(lnbuf, "Instrlib") == 0)
					strcpy(defLibDir, eq);
				else if (strcmp(lnbuf, "Projects") == 0)
					strcpy(defPrjDir, eq);
				else if (strcmp(lnbuf, "WaveIn") == 0)
					strcpy(defWaveIn, eq);
				else if (strcmp(lnbuf, "Author") == 0)
					strcpy(defAuthor, eq);
				else if (strcmp(lnbuf, "Copyright") == 0)
					strcpy(defCopyright, eq);
				else if (strcmp(lnbuf, "InclNotelist") == 0)
					inclNotelist = atoi(eq);
				else if (strcmp(lnbuf, "InclSequence") == 0)
					inclSequence = atoi(eq);
				else if (strcmp(lnbuf, "InclScripts") == 0)
					inclScripts = atoi(eq);
				else if (strcmp(lnbuf, "InclTextFiles") == 0)
					inclTextFiles = atoi(eq);
				else if (strcmp(lnbuf, "InclLibraries") == 0)
					inclLibraries = atoi(eq);
				else if (strcmp(lnbuf, "InclInstruments") == 0)
					inclInstr = xtoi(eq);
				else if (strcmp(lnbuf, "InclSoundFonts") == 0)
					inclSoundFonts = atoi(eq);
				else if (strcmp(lnbuf, "Latency") == 0)
					playBuf = atof(eq);
				else if (strcmp(lnbuf, "MIDIDeviceName") == 0)
				{
					strcpy(midiDeviceName, eq);
					midiDevice = GetMIDIDevice(midiDeviceName);
				}
				else if (strcmp(lnbuf, "MIDIDevice") == 0)
					midiDevice = atoi(eq);
				else if (strcmp(lnbuf, "WaveDevice") == 0)
					strcpy(waveDevice, eq);
				else if (strcmp(lnbuf, "Top") == 0)
					frmTop = atoi(eq);
				else if (strcmp(lnbuf, "Left") == 0)
					frmLeft = atoi(eq);
				else if (strcmp(lnbuf, "Width") == 0)
					frmWidth = atoi(eq);
				else if (strcmp(lnbuf, "Height") == 0)
					frmHeight = atoi(eq);
				else if (strcmp(lnbuf, "Maximize") == 0)
					frmMax = atoi(eq);
			}
		}
		fclose(fp);
	}
	else
	{
#if _WIN32
		strcpy(installDir, "c:/Program Files/");
#else
		strcpy(installDir, "/usr/etc/");
#endif
		strcat(installDir, "basicsynth");

		strcpy(defAuthor, "Me");
		strcpy(defCopyright, "2010");
		strcpy(defLibDir, installDir);
		strcat(defLibDir, "/libs");
		strcpy(defWaveIn, installDir);
		strcat(defWaveIn, "/wavefiles");
		strcpy(formsDir, installDir);
		strcat(formsDir, "/forms");
		return;
	}
	SynthProject::NormalizePath(installDir);
	SynthProject::NormalizePath(defPrjDir);
	SynthProject::NormalizePath(defLibDir);
	SynthProject::NormalizePath(defWaveIn);
	SynthProject::NormalizePath(formsDir);
#if _WIN32
	if (pDirectSoundEnumerate)
		pDirectSoundEnumerate(FindWaveDevice, NULL);
#endif
}

void ProjectOptions::Save()
{
	char *hm = getenv("HOME");
	if (hm == 0)
	{
		hm = getenv("HOMEPATH"); // Windows name for HOME
		if (hm == 0)
			hm = ".";
	}
	bsString cfgFile;
	cfgFile = hm;
	cfgFile += "/.bsynth";
	FILE *fp = fopen(cfgFile, "w");
	if (fp)
	{
		fprintf(fp, "Install=%s\n", installDir);
		fprintf(fp, "Forms=%s\n", formsDir);
		fprintf(fp, "Colors=%s\n", colorsFile);
		fprintf(fp, "Instrlib=%s\n", defLibDir);
		fprintf(fp, "Projects=%s\n", defPrjDir);
		fprintf(fp, "WaveIn=%s\n", defWaveIn);
		fprintf(fp, "Author=%s\n", defAuthor);
		fprintf(fp, "Copyright=%s\n", defCopyright);
		fprintf(fp, "InclNotelist=%d\n", inclNotelist);
		fprintf(fp, "InclSequence=%d\n", inclSequence);
		fprintf(fp, "InclScripts=%d\n", inclScripts);
		fprintf(fp, "InclTextFiles=%d\n", inclTextFiles);
		fprintf(fp, "InclLibraries=%d\n", inclLibraries);
		fprintf(fp, "InclInstruments=%x\n", inclInstr);
		fprintf(fp, "InclSoundFonts=%d", inclSoundFonts);
		fprintf(fp, "Latency=%f\n", playBuf);
		fprintf(fp, "MIDIDevice=%d\n", midiDevice);
		fprintf(fp, "MIDIDeviceName=%s\n", midiDeviceName);
		fprintf(fp, "WaveDevice=%s\n", waveDevice);
		fprintf(fp, "Top=%d\n", frmTop);
		fprintf(fp, "Left=%d\n", frmLeft);
		fprintf(fp, "Width=%d\n", frmWidth);
		fprintf(fp, "Height=%d\n", frmHeight);
		fprintf(fp, "Maximize=%d\n", frmMax);
		fclose(fp);
	}
#if _WIN32
	if (pDirectSoundEnumerate)
		pDirectSoundEnumerate(FindWaveDevice, NULL);
#endif
}

#endif
