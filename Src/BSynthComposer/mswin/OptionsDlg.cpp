//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "OptionsDlg.h"

static BOOL CALLBACK DSDevEnum(LPGUID lpGUID, 
             LPCTSTR lpszDesc,
             LPCTSTR lpszDrvName, 
             LPVOID lpContext)
{
	CComboBox *cb = (CComboBox *) lpContext;
	cb->AddString(lpszDesc);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
OptionsDlg::OptionsDlg()
{
}

LRESULT OptionsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	CheckDlgButton(IDC_INCL_NOTELIST, prjOptions.inclNotelist);
	CheckDlgButton(IDC_INCL_SCRIPTS, prjOptions.inclScripts);
	CheckDlgButton(IDC_INCL_SEQUENCE, prjOptions.inclSequence);
	CheckDlgButton(IDC_INCL_TEXTFILES, prjOptions.inclTextFiles);
	CheckDlgButton(IDC_INCL_LIBRARIES, prjOptions.inclLibraries);
	CheckDlgButton(IDC_INCL_SOUNDFONTS, prjOptions.inclSoundFonts);
	SetDlgItemText(IDC_DEF_PROJECTS, prjOptions.defPrjDir);
	SetDlgItemText(IDC_DEF_WAVEIN, prjOptions.defWaveIn);
	SetDlgItemText(IDC_DEF_FORMS, prjOptions.formsDir);
	SetDlgItemText(IDC_DEF_COLORS, prjOptions.colorsFile);
	SetDlgItemText(IDC_DEF_LIBRARIES, prjOptions.defLibDir);
	SetDlgItemText(IDC_DEF_AUTHOR, prjOptions.defAuthor);
	SetDlgItemText(IDC_DEF_COPYRIGHT, prjOptions.defCopyright);

	char buf[40];
	snprintf(buf, 40, "%f", prjOptions.playBuf);
	SetDlgItemText(IDC_LATENCY, buf);

	midiDev = GetDlgItem(IDC_MIDI_IN);
	waveDev = GetDlgItem(IDC_WAVE_OUT);

	int midiDevNo = 0;
	UINT ndev = midiInGetNumDevs(); 
	for (UINT n = 0; n < ndev; n++)
	{
		MIDIINCAPS caps;
		memset(&caps, 0, sizeof(caps));
		midiInGetDevCaps(n, &caps, sizeof(caps));
		if (strcmp(caps.szPname, prjOptions.midiDeviceName) == 0)
			midiDevNo = n;
		midiDev.AddString(caps.szPname);
	}
	midiDev.SetCurSel(midiDevNo);

    DirectSoundEnumerate(DSDevEnum, &waveDev);
	if (prjOptions.waveDevice[0])
		waveDev.SelectString(-1, prjOptions.waveDevice);
	else
		waveDev.SetCurSel(0);

	return 1;
}

void OptionsDlg::Browse(int id, char *caption)
{
	char name[MAX_PATH];
	GetDlgItemText(id, name, MAX_PATH);

	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = 0;
	bi.pszDisplayName = name;
	bi.lpszTitle = caption;
    bi.ulFlags = BIF_USENEWUI;

//	PIDLIST_ABSOLUTE pidl;
	LPCITEMIDLIST pidl;
	if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		SHGetPathFromIDList(pidl, name);
		SetDlgItemText(id, name);
		IMalloc *mp;
		SHGetMalloc(&mp);
		mp->Free((void*)pidl);
		mp->Release();
	}
}

LRESULT OptionsDlg::OnBrowseProjects(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Browse(IDC_DEF_PROJECTS, "Default Project Folder");
	return 0;
}

LRESULT OptionsDlg::OnBrowseForms(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Browse(IDC_DEF_PROJECTS, "Forms Folder");
	return 0;
}

LRESULT OptionsDlg::OnBrowseColors(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char name[MAX_PATH];
	GetDlgItemText(IDC_DEF_COLORS, name, MAX_PATH);
	if (prjFrame->BrowseFile(1, name, "XML Files|*.xml|", "xml"))
		SetDlgItemText(IDC_DEF_COLORS, name);
	return 0;
}

LRESULT OptionsDlg::OnBrowseLibraries(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Browse(IDC_DEF_PROJECTS, "Library Folder");
	return 0;
}

LRESULT OptionsDlg::OnBrowseWaveIn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Browse(IDC_DEF_WAVEIN, "Wave Files Folder");
	return 0;
}

LRESULT OptionsDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	prjOptions.inclNotelist = IsDlgButtonChecked(IDC_INCL_NOTELIST);
	prjOptions.inclScripts = IsDlgButtonChecked(IDC_INCL_SCRIPTS);
	prjOptions.inclSequence = IsDlgButtonChecked(IDC_INCL_SEQUENCE);
	prjOptions.inclTextFiles = IsDlgButtonChecked(IDC_INCL_TEXTFILES);
	prjOptions.inclLibraries = IsDlgButtonChecked(IDC_INCL_LIBRARIES);
	prjOptions.inclSoundFonts = IsDlgButtonChecked(IDC_INCL_SOUNDFONTS);
	GetDlgItemText(IDC_DEF_PROJECTS, prjOptions.defPrjDir, MAX_PATH);
	GetDlgItemText(IDC_DEF_WAVEIN, prjOptions.defWaveIn, MAX_PATH);
	GetDlgItemText(IDC_DEF_FORMS, prjOptions.formsDir, MAX_PATH);
	GetDlgItemText(IDC_DEF_COLORS, prjOptions.colorsFile, MAX_PATH);
	GetDlgItemText(IDC_DEF_LIBRARIES, prjOptions.defLibDir, MAX_PATH);
	GetDlgItemText(IDC_DEF_AUTHOR, prjOptions.defAuthor, MAX_PATH);
	GetDlgItemText(IDC_DEF_COPYRIGHT, prjOptions.defCopyright, MAX_PATH);

	char buf[40];
	GetDlgItemText(IDC_LATENCY, buf, 40);
	prjOptions.playBuf = atof(buf);
	if (prjOptions.playBuf < 0.02)
		prjOptions.playBuf = 0.02;

	prjOptions.midiDevice = SendDlgItemMessage(IDC_MIDI_IN, CB_GETCURSEL);
	if (prjOptions.midiDevice == CB_ERR)
		memset(prjOptions.midiDeviceName, 0, MAX_PATH);
	else
		GetDlgItemText(IDC_MIDI_IN, prjOptions.midiDeviceName, MAX_PATH);

	waveDev.GetWindowText(prjOptions.waveDevice, MAX_PATH);

	prjOptions.Save();

	EndDialog(IDOK);
	return 0;
}

LRESULT OptionsDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 0;
}
