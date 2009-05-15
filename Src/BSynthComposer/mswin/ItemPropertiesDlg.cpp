#include "Stdafx.h"
#include "resource.h"
#include "ProjectItemDlg.h"
#include "ItemPropertiesDlg.h"

static int itmPropIds[] = {
	-1,
	IDC_ITEM_NAME,
	IDC_ITEM_DESCR,
	IDC_FILE_NAME,
	IDC_FILE_INCLUDE,
	IDC_INST_NUM,
	IDC_INST_TYPE,
	IDC_INST_LIST,
	IDC_WVF_ID,
//	IDC_WVF_LPST,
//	IDC_WVF_LPEND,
	IDC_RENUMBER
};

int ItemPropertiesBase::GetFieldID(int id, int& idval)
{
	if (id < (sizeof(itmPropIds)/sizeof(int)))
	{
		idval = itmPropIds[id];
		return 1;
	}
	return 0;
}

LRESULT ItemPropertiesBase::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	InitSpecific();

	pi->LoadProperties(static_cast<PropertyBox*>(this));

	EnableOK();

	return 1;
}

LRESULT ItemPropertiesBase::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (pi->SaveProperties(static_cast<PropertyBox*>(this)))
		EndDialog(1);
	return 0;
}

LRESULT ItemPropertiesBase::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(0);
	return 0;
}

LRESULT ItemPropertiesBase::OnNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (needName)
		EnableOK();
	return 0;
}

LRESULT ItemPropertiesBase::OnFileNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (needFile)
		EnableOK();
	return 0;
}

LRESULT ItemPropertiesBase::OnNumChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (needInum)
		EnableOK();
	return 0;
}

LRESULT ItemPropertiesBase::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	const char *spc = ProjectItem::GetFileSpec(pi->GetType());
	const char *ext = ProjectItem::GetFileExt(pi->GetType());
	char path[MAX_PATH];
	path[0] = '\0';
	GetDlgItemText(IDC_FILE_NAME, path, MAX_PATH);

	if (prjFrame->BrowseFile(0, path, spc, ext))
	{
		SetDlgItemText(IDC_FILE_NAME, SynthProject::SkipProjectDir(path));
		if (needFile)
			EnableOK();
	}

	return 0;
}

void ItemPropertiesBase::EnableOK()
{
	HWND w;
	int enable = 1;
	if (needName)
	{
		if ((w = GetDlgItem(IDC_ITEM_NAME)) == 0 || ::GetWindowTextLength(w) < 1)
			enable = 0;
	}
	if (needFile)
	{
		if ((w = GetDlgItem(IDC_FILE_NAME)) == 0 || ::GetWindowTextLength(w) < 1)
			enable = 0;
	}
	if (needInum)
	{
		if ((w = GetDlgItem(IDC_INST_NUM)) == 0 || ::GetWindowTextLength(w) < 1)
			enable = 0;
	}
	::EnableWindow(GetDlgItem(IDOK), enable);
}

void InstrPropertiesDlg::InitSpecific()
{
	CListBox typeList = GetDlgItem(IDC_INST_TYPE);

	InstrMapEntry *im = 0;
	while ((im = theProject->mgr.EnumType(im)) != 0)
	{
		int ndx = typeList.AddString(im->GetType());
		typeList.SetItemDataPtr(ndx, im);
	}
}

////////////////////////////////////////////////////////////////////////////

int InstrSelectDlg::GetFieldID(int id, int& idval)
{
	if (id == PROP_ILST || id == IDC_INST_TYPE)
	{
		idval = IDC_INST_TYPE;
		return 1;
	}
	if (id == PROP_INUM || id == IDC_INST_NUM)
	{
		idval = IDC_INST_NUM;
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////

