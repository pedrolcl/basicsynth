#include "stdafx.h"
#include "resource.h"
#include "OptionsDlg.h"

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

	PIDLIST_ABSOLUTE pidl;
	if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		SHGetPathFromIDList(pidl, name);
		SetDlgItemText(id, name);
		IMalloc *mp;
		SHGetMalloc(&mp);
		mp->Free(pidl);
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
	Browse(IDC_DEF_COLORS, "Colors File");
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

	EndDialog(IDOK);
	return 0;
}

LRESULT OptionsDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 0;
}
