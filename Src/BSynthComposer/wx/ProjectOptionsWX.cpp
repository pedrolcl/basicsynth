//////////////////////////////////////////////////////////////////////
// BasicSynth - wx specific code for project options
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"

void ProjectOptions::Load()
{
	long lval;
	wxString str;

#if UNIX
	wxString locname;
	locname = wxStandardPaths::Get().GetUserConfigDir();
	locname += "/.";
	locname += wxTheApp->GetAppName();
	wxString globname;
	globname = wxStandardPaths::Get().GetDataDir();
	globname += "/.";
	globname += wxTheApp->GetAppName();
	// if the user does not have a config file,
	// copy the global configuration
	if (!wxFile::Exists(locname) && wxFile::Exists(globname))
	{
		wxFile in(globname, wxFile::read);
		wxFile out(locname, wxFile::write);
		char buffer[1024];
		size_t count;
		while ((count = in.Read(buffer, 1024)) > 0)
			out.Write(buffer, count);
	}
#endif
	wxConfigBase *config = wxConfig::Get();
	if (config->Read("Install", &str))
		strncpy(installDir, str, MAX_PATH);
	if (config->Read("HelpFile", &str))
		strncpy(helpFile, str, MAX_PATH);
	if (config->Read("Forms", &str))
		strncpy(formsDir, str, MAX_PATH);
	if (config->Read("HelpFile", &str))
		strncpy(helpFile, str, MAX_PATH);
	if (config->Read("Colors", &str))
		strncpy(colorsFile, str, MAX_PATH);
	if (config->Read("InstrLib", &str))
		strncpy(defLibDir, str, MAX_PATH);
	if (config->Read("Projects", &str))
		strncpy(defPrjDir, str, MAX_PATH);
	if (config->Read("WaveIn", &str))
		strncpy(defWaveIn, str, MAX_PATH);
	if (config->Read("InclNotelist", &lval))
		inclNotelist = (int)lval;
	if (config->Read("InclSequence", &lval))
		inclSequence = lval;
	if (config->Read("InclScripts", &lval))
		inclScripts = lval;
	if (config->Read("InclTextFiles", &lval))
		inclTextFiles = lval;
	if (config->Read("InclLibraries", &lval))
		inclLibraries = lval;
	if (config->Read("InclSoundFonts", &lval))
		inclSoundFonts = lval;
	if (config->Read("Latency", &str))
		playBuf = atof(str);
	if (config->Read("TickRes", &str))
		tickRes = atof(str);
	if (config->Read("InclInstruments", &str))
		inclInstr = xtoi(str);
	if (config->Read("Author", &str))
		strncpy(defAuthor, str, sizeof(defAuthor));
	if (config->Read("Copyright", &str))
		strncpy(defCopyright, str, sizeof(defCopyright));
	if (config->Read("MIDIDeviceName", &str))
		strncpy(midiDeviceName, str, sizeof(midiDeviceName));
	if (config->Read("MIDIDevice", &lval))
		midiDevice = lval;
	if (config->Read("WaveDevice", &str))
		strncpy(waveDevice, str, sizeof(waveDevice));
	if (config->Read("Top", &lval))
		frmTop = lval;
	if (config->Read("Left", &lval))
		frmLeft = lval;
	if (config->Read("Width", &lval))
		frmWidth = lval;
	if (config->Read("Height", &lval))
		frmHeight = lval;
	if (config->Read("Maximize", &lval))
		frmMax = lval;

	if (installDir[0] == '\0')
	{
		str = ::wxPathOnly(wxStandardPaths::Get().GetExecutablePath());
		strncpy(installDir, str, sizeof(installDir));
	}
	str = wxStandardPaths::Get().GetDataDir();
	if (formsDir[0] == '\0')
	{
		strncpy(formsDir, str, sizeof(formsDir)-8);
		strcat(formsDir, "/Forms");
	}

//	str = wxStandardPaths::Get().GetUserDataDir();
//	if (defLibDir[0] == '\0')
//		strncpy(defLibDir, str, MAX_PATH);
//	if (defPrjDir[0] == '\0')
//		strncpy(defPrjDir, str, MAX_PATH);
	if (colorsFile[0] == '\0')
		strncpy(colorsFile, "Colors.xml", MAX_PATH);
	SynthProject::NormalizePath(installDir);
	SynthProject::NormalizePath(defPrjDir);
	SynthProject::NormalizePath(defLibDir);
	SynthProject::NormalizePath(defWaveIn);
	SynthProject::NormalizePath(formsDir);
	SynthProject::NormalizePath(colorsFile);
	SynthProject::NormalizePath(helpFile);
}

void ProjectOptions::Save()
{
	SynthProject::NormalizePath(installDir);
	SynthProject::NormalizePath(defPrjDir);
	SynthProject::NormalizePath(defLibDir);
	SynthProject::NormalizePath(defWaveIn);
	SynthProject::NormalizePath(formsDir);
	SynthProject::NormalizePath(colorsFile);
	//SynthProject::NormalizePath(helpFile);

	wxConfigBase *config = wxConfig::Get();

	config->Write("Projects", wxString(defPrjDir));
	config->Write("Forms", wxString(formsDir));
	config->Write("HelpFile", wxString(helpFile));
	config->Write("Colors", wxString(colorsFile));
	config->Write("WaveIn", wxString(defWaveIn));
	config->Write("Author", wxString(defAuthor));
	config->Write("Copyright", wxString(defCopyright));
	config->Write("InclNotelist", inclNotelist);
	config->Write("InclSequence", inclSequence);
	config->Write("InclScripts", inclScripts);
	config->Write("InclTextFiles", inclTextFiles);
	config->Write("InclLibraries", inclLibraries);
	config->Write("InclSoundFonts", inclSoundFonts);
	char buf[40];
	snprintf(buf, 40, "%x", inclInstr);
	config->Write("InclInstruments", wxString(buf));
	config->Write("Latency", playBuf);
	config->Write("TickRes", tickRes);
	config->Write("MIDIDevice", midiDevice);
	config->Write("MIDIDeviceName", wxString(midiDeviceName));
	config->Write("WaveDevice", wxString(waveDevice));
	config->Write("Top", frmTop);
	config->Write("Left", frmLeft);
	config->Write("Width", frmWidth);
	config->Write("Height", frmHeight);
	config->Write("Maximize", frmMax);
}

