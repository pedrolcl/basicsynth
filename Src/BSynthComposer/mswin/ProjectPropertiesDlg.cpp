#include "Stdafx.h"
#include "resource.h"
#include "ProjectItemDlg.h"
#include "ProjectPropertiesDlg.h"

static int prjPropIds[] = { -1,
	IDC_PROJECT_NAME, IDC_PROJECT_COMPOSER, IDC_PROJECT_COPYRIGHT, IDC_PROJECT_DESCR, 
	IDC_PROJECT_FILE, IDC_PROJECT_OUTPUT, IDC_PROJECT_LEAD, IDC_PROJECT_TAIL,
	IDC_PROJECT_SAMPLERATE, IDC_PROJECT_SAMPLEFMT, IDC_PROJECT_WTSIZE, IDC_PROJECT_WTUSER,
	IDC_PROJECT_WVPATH };

int ProjectPropertiesDlg::GetFieldID(int id, int& idval)
{
	if (id < (sizeof(prjPropIds)/sizeof(int)))
	{
		idval = prjPropIds[id];
		return 1;
	}
	return 0;
}

int ProjectPropertiesDlg::GetSelection(int id, short& sel)
{
	if (prjPropIds[id] == IDC_PROJECT_SAMPLEFMT)
	{
		CComboBox cbFmt;
		cbFmt = GetDlgItem(IDC_PROJECT_SAMPLEFMT);
		sel = (short) cbFmt.GetCurSel();
		if (sel != CB_ERR)
			return 1;
	}
	sel = 0;
	return 0;
}

void ProjectPropertiesDlg::SetSelection(int id, short sel)
{
	if (prjPropIds[id] == IDC_PROJECT_SAMPLEFMT)
	{
		CComboBox cbFmt;
		cbFmt = GetDlgItem(IDC_PROJECT_SAMPLEFMT);
		cbFmt.SetCurSel((int)sel);
	}
}

LRESULT ProjectPropertiesDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	prjNameWnd = GetDlgItem(IDC_PROJECT_NAME);
	prjFileWnd = GetDlgItem(IDC_PROJECT_FILE);
	wavFileWnd = GetDlgItem(IDC_PROJECT_OUTPUT);
	wavPathWnd = GetDlgItem(IDC_PROJECT_WVPATH);

	CComboBox cbRate;
	cbRate = GetDlgItem(IDC_PROJECT_SAMPLERATE);
	cbRate.InsertString(0, "22050");
	cbRate.InsertString(1, "44100");
	cbRate.InsertString(2, "48000");
	cbRate.InsertString(3, "96000");

	CComboBox cbFmt;
	cbFmt = GetDlgItem(IDC_PROJECT_SAMPLEFMT);
	cbFmt.InsertString(0, "16-bit PCM");
	cbFmt.InsertString(1, "32-bit Float");

	CButton btn;
	btn = GetDlgItem(IDC_PROJECT_PATH_UP);
	RECT rc;
	btn.GetClientRect(&rc);
	int cx = 16;
	if ((rc.right - rc.left) >= 32)
		cx = 32;

	HICON mvUp = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_MVUP), IMAGE_ICON, cx, cx, LR_SHARED);
	HICON mvDn = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_MVDN), IMAGE_ICON, cx, cx, LR_SHARED);
	HICON add = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ADD), IMAGE_ICON, cx, cx, LR_SHARED);
	HICON rem = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_REM), IMAGE_ICON, cx, cx, LR_SHARED);

	btn.SetIcon(mvUp);
	btn = GetDlgItem(IDC_PROJECT_PATH_DN);
	btn.SetIcon(mvDn);
	btn = GetDlgItem(IDC_PROJECT_PATH_ADD);
	btn.SetIcon(add);
	btn = GetDlgItem(IDC_PROJECT_PATH_REM);
	btn.SetIcon(rem);

	lbPath = GetDlgItem(IDC_PROJECT_PATH);

	pi->LoadProperties(static_cast<PropertyBox*>(this));

	// HACK: Fill in path list...
	int ndx = 0;
	PathListItem *pl = theProject->libPath->EnumList(0);
	while (pl)
	{
		lbPath.InsertString(ndx++, pl->path);
		pl = theProject->libPath->EnumList(pl);
	}

	AutoWavefile();
	EnableUpDn();
	EnableOK();

	return 1;
}

LRESULT ProjectPropertiesDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!pi->SaveProperties(static_cast<PropertyBox*>(this)))
		return 0;

	theProject->libPath->RemoveAll();

	char path[MAX_PATH];
	int lbCount = lbPath.GetCount();
	int lbIndex;
	for (lbIndex = 0; lbIndex < lbCount; lbIndex++)
	{
		lbPath.GetText(lbIndex, path);
		theProject->libPath->AddItem(path);
	}

	EndDialog(IDOK);
	return 0;
}

LRESULT ProjectPropertiesDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT ProjectPropertiesDlg::OnNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EnableOK();
	return 0;
}

LRESULT ProjectPropertiesDlg::OnFileNameBlur(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	AutoWavefile();
	EnableOK();
	return 0;
}

void ProjectPropertiesDlg::EnableUpDn()
{
	int count = lbPath.GetCount();
	int mvUp = count > 1;
	int mvDn = count > 1;
	int sel = lbPath.GetCurSel();
	if (sel == LB_ERR)
	{
		mvUp = FALSE;
		mvDn = FALSE;
	}
	else if (sel == 0)
		mvUp = FALSE;
	else if (sel == count-1)
		mvDn = FALSE;
	::EnableWindow(GetDlgItem(IDC_PROJECT_PATH_UP), mvUp);
	::EnableWindow(GetDlgItem(IDC_PROJECT_PATH_DN), mvDn);
}

LRESULT ProjectPropertiesDlg::OnPathSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EnableUpDn();
	return 0;
}

LRESULT ProjectPropertiesDlg::OnPathMvup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = lbPath.GetCurSel();
	if (sel == LB_ERR || sel == 0)
		return 0;
	
	char path[MAX_PATH];
	lbPath.GetText(sel, path);
	lbPath.DeleteString(sel);
	sel--;
	lbPath.InsertString(sel, path);
	lbPath.SetCurSel(sel);

	EnableUpDn();

	return 0;
}

LRESULT ProjectPropertiesDlg::OnPathMvdn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = lbPath.GetCurSel();
	if (sel == LB_ERR && sel < lbPath.GetCount())
		return 0;

	char path[MAX_PATH];
	lbPath.GetText(sel, path);
	lbPath.DeleteString(sel);
	sel++;
	lbPath.InsertString(sel, path);
	lbPath.SetCurSel(sel);

	EnableUpDn();

	return 0;
}

LRESULT ProjectPropertiesDlg::OnPathAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char name[MAX_PATH];
	memset(name, 0, sizeof(name));

	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = 0;
	bi.pszDisplayName = name;
	bi.lpszTitle = "Select the folder for project files";
    bi.ulFlags = BIF_USENEWUI;

	PIDLIST_ABSOLUTE pidl;
	if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		SHGetPathFromIDList(pidl, name);
		int count = lbPath.GetCount();
		lbPath.InsertString(count, name);
		IMalloc *mp;
		SHGetMalloc(&mp);
		mp->Free(pidl);
		mp->Release();
	}
	EnableUpDn();
	return 0;
}

LRESULT ProjectPropertiesDlg::OnPathRem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = lbPath.GetCurSel();
	if (sel != LB_ERR)
		lbPath.DeleteString(sel);

	EnableUpDn();

	return 0;
}

LRESULT ProjectPropertiesDlg::OnBrowseOut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	const char *spc = ProjectItem::GetFileSpec(PRJNODE_WVFILE);
	const char *ext = ProjectItem::GetFileExt(PRJNODE_WVFILE);
	char path[MAX_PATH];
	path[0] = 0;
	prjFileWnd.GetWindowText(path, MAX_PATH);
	if (prjFrame->BrowseFile(0, path, spc, ext))
		wavFileWnd.SetWindowText(SynthProject::SkipProjectDir(path));

	return 0;
}

LRESULT ProjectPropertiesDlg::OnBrowseIn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	const char *spc = ProjectItem::GetFileSpec(PRJNODE_PROJECT);
	const char *ext = ProjectItem::GetFileExt(PRJNODE_PROJECT);
	char path[MAX_PATH];
	path[0] = 0;
	prjFileWnd.GetWindowText(path, MAX_PATH);
	if (prjFrame->BrowseFile(0, path, spc, ext))
		prjFileWnd.SetWindowText(path);

	return 0;
}

LRESULT ProjectPropertiesDlg::OnBrowseWv(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char name[MAX_PATH];
	memset(name, 0, sizeof(name));
	wavPathWnd.GetWindowText(name, MAX_PATH);

	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = 0;
	bi.pszDisplayName = name;
	bi.lpszTitle = "Select the folder for WAV files";
    bi.ulFlags = BIF_USENEWUI;

	PIDLIST_ABSOLUTE pidl;
	if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		SHGetPathFromIDList(pidl, name);
		wavPathWnd.SetWindowText(name);
		IMalloc *mp;
		SHGetMalloc(&mp);
		mp->Free(pidl);
		mp->Release();
	}
	EnableUpDn();
	return 0;
}
void ProjectPropertiesDlg::EnableOK()
{
	int len1 = prjNameWnd.GetWindowTextLength();
	int len2 = prjFileWnd.GetWindowTextLength();
	::EnableWindow(GetDlgItem(IDOK), len1 > 0 && len2 > 0);
}

void ProjectPropertiesDlg::AutoWavefile()
{
	int len2 = wavFileWnd.GetWindowTextLength();
	if (len2 > 0)
		return;

	int len1 = prjFileWnd.GetWindowTextLength();
	if (len1 > 0)
	{
		char buf[MAX_PATH];
		prjFileWnd.GetWindowText(buf, MAX_PATH);
		char *name = SynthProject::SkipProjectDir(buf);
		len1 = (int) strlen(name);
		char *dot = strrchr(name, '.');
		if (dot == NULL)
			dot = &name[len1];
		strcpy(dot, ".wav");
		wavFileWnd.SetWindowText(name);
	}
}
