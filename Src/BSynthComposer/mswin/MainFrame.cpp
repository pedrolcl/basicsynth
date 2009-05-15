/////////////////////////////////////////////////////////////////////////////
// BSynthComposer - frame window implementation for MS-Windows
//
// Much of the GUI implementation is here. This class manages the main frame
// layout, tree view, creation of dialogs and editors, and menu/toolbar commands.
// Properties are handled by separate dialogs. The specific dialog is determined
// from the project item type.
//
// Most methods are short, primarily passing work off to the current project
// item and/or editor.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <Htmlhelp.h>

#include "AboutDlg.h"
#include "QueryValueDlg.h"
#include "MainFrm.h"
#include "FormEditor.h"
#include "TextEditor.h"
#include "OptionsDlg.h"
#include "ProjectItemDlg.h"
#include "ProjectPropertiesDlg.h"
#include "ItemPropertiesDlg.h"
#include "GenerateDlg.h"
#include "MixerSetupDlg.h"

static char mruRegKey[] = "Software\\BasicSynth";

static TBBUTTON mainButtons[] = {
	{ 0, ID_PROJECT_NEW, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 1, ID_PROJECT_OPEN, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 13, ID_PROJECT_SAVE, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 4, 0, 0, BTNS_SEP },
	{ 3, ID_EDIT_CUT, 0, BTNS_BUTTON },
	{ 4, ID_EDIT_COPY, 0, BTNS_BUTTON },
	{ 5, ID_EDIT_PASTE, 0, BTNS_BUTTON },
	{ 6, ID_EDIT_UNDO, 0, BTNS_BUTTON },
	{ 7, ID_EDIT_REDO, 0, BTNS_BUTTON },
	{ 8, ID_EDIT_FIND, 0, BTNS_BUTTON },
	{ 9, ID_EDIT_FINDNEXT, 0, BTNS_BUTTON },
	{ 4, 0, 0, BTNS_SEP },
	{ 10, ID_PROJECT_GENERATE, 0, BTNS_BUTTON },
	{ 11, ID_PROJECT_PLAY, 0, BTNS_BUTTON },
	{ 4, 0, 0, BTNS_SEP },
	{ 12, ID_ITEM_PROPERTIES, 0, BTNS_BUTTON },
	{ 2, ID_ITEM_SAVE, 0, BTNS_BUTTON },
	{ 15, ID_ITEM_EDIT, 0, BTNS_BUTTON },
	{ 14, ID_ITEM_NEW, 0, BTNS_BUTTON },
	{ 16, ID_ITEM_ADD, 0, BTNS_BUTTON },
};

BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<MainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return tabView.PreTranslateMessage(pMsg);
}

BOOL MainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}

LRESULT MainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_Module.mainWnd = m_hWnd;
	LoadString(_Module.GetResourceInstance(), IDS_PRODUCT, _Module.ProductName, 80);
	prjOptions.Load();


	RECT rcMain;
	GetClientRect(&rcMain);

	// create command bar window
	HWND hWndCmdBar = cmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	HMENU mainMnu = ::GetMenu(m_hWnd);
	HMENU fileMnu = ::GetSubMenu(mainMnu, 0);
	HMENU mruMnu = ::GetSubMenu(fileMnu, 8);
	mruList.SetMaxEntries(6);
	mruList.SetMenuHandle(mruMnu);
	mruList.SetMaxItemLength(40);
	mruList.ReadFromRegistry(mruRegKey);

	// attach menu
	cmdBar.AttachMenu(GetMenu());
	cmdBar.SetImageMaskColor(RGB(255,0,255));
	cmdBar.SetImageSize(16, 16);
	cmdBar.AddBitmap(IDB_NEWFOLDER, ID_PROJECT_NEW);
	cmdBar.AddBitmap(IDB_OPENFOLDER, ID_PROJECT_OPEN);
	cmdBar.AddBitmap(IDB_SAVEALL, ID_PROJECT_SAVE);
	cmdBar.AddBitmap(IDB_SAVE, ID_ITEM_SAVE);

	cmdBar.AddBitmap(IDB_UNDO, ID_EDIT_UNDO);
	cmdBar.AddBitmap(IDB_REDOX, ID_EDIT_REDO);
	cmdBar.AddBitmap(IDB_CUT, ID_EDIT_CUT);
	cmdBar.AddBitmap(IDB_COPY, ID_EDIT_COPY);
	cmdBar.AddBitmap(IDB_PASTE, ID_EDIT_PASTE);
	cmdBar.AddBitmap(IDB_FIND, ID_EDIT_FIND);
	cmdBar.AddBitmap(IDB_FINDNEXT, ID_EDIT_FINDNEXT);

	cmdBar.AddBitmap(IDB_PROPERTIES, ID_PROJECT_PROPERTIES);
	cmdBar.AddBitmap(IDB_AUDIO, ID_PROJECT_GENERATE);
	cmdBar.AddBitmap(IDB_PLAY, ID_PROJECT_PLAY);
// remove old menu
	SetMenu(NULL);

	bVisible = EDITOR_PANE | TOOLBAR_PANE | STATUS_PANE;

	DWORD tbWinStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	DWORD tbStyle = CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT;
	HWND hWndToolBar = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, tbWinStyle | tbStyle,
		0, 0, 100, 100, m_hWnd, (HMENU)LongToHandle(ATL_IDW_TOOLBAR),
		_Module.GetModuleInstance(), NULL);
	::SendMessage(hWndToolBar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0L);
	COLORREF crMask = CLR_DEFAULT;
	HIMAGELIST hImageList = ImageList_LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINTB), 
		16, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
	::SendMessage(hWndToolBar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
	::SendMessage(hWndToolBar, TB_ADDBUTTONS, 20, (LPARAM) mainButtons);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();

	RECT rcKbd;
	rcKbd.left = 0;
	rcKbd.top = 0;
	rcKbd.right = rcMain.right - 50;
	rcKbd.bottom = 250;
	kbdWnd.Create(m_hWnd, rcKbd, "", WS_CHILD|WS_BORDER|WS_CLIPCHILDREN, 0, 9);
	kbdWnd.GetClientRect(&rcKbd);

	RECT rcSplit;
	rcSplit.left = 0;
	rcSplit.top = 0;
	rcSplit.right = rcMain.right;
	rcSplit.bottom = rcMain.bottom - rcKbd.bottom;
	m_hWndClient = splitTop.Create(m_hWnd, rcSplit, "client", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER, 0);
	splitTop.SetSplitterExtendedStyle(0, SPLIT_PROPORTIONAL);
	prjList.Create(splitTop, rcDefault, "list", WS_CHILD | WS_BORDER | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, ID_PROJECT_LIST);
	tabView.Create(splitTop, rcDefault, "tabber", WS_CHILD | WS_BORDER | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE, ID_TAB_WND);
	splitTop.SetSplitterPanes(prjList, tabView, false);
	splitTop.SetSplitterPos(200, 1); //Pct(15);

	UIAddToolBar(hWndToolBar);
	
	UpdateEditUI(-1);
	UpdateItemUI(0);
	UpdateProjectUI();

	UISetCheck(ID_VIEW_PROJECT, 0);
	UISetCheck(ID_VIEW_KEYBOARD, 0);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	EnablePanes();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = cmdBar.GetMenu();
	tabView.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));
	//tabView.SetTitleBarWindow(m_hWnd);

	return 0;
}

//////////////// Internal functions ///////////////////////////////

void MainFrame::EnablePanes()
{
	if (bVisible & PROJECT_PANE)
		splitTop.SetSinglePaneMode(SPLIT_PANE_NONE);
	else
		splitTop.SetSinglePaneMode(SPLIT_PANE_RIGHT);

	kbdWnd.ShowWindow(bVisible & KEYBOARD_PANE ? SW_SHOW : SW_HIDE);
	UISetCheck(ID_VIEW_PROJECT, bVisible & PROJECT_PANE ? 1 : 0);
	UISetCheck(ID_VIEW_KEYBOARD, bVisible & KEYBOARD_PANE ? 1 : 0);
}

void MainFrame::UpdateLayout(BOOL bResizeBars)
{
	RECT rect = { 0 };
	GetClientRect(&rect);

	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	if (bVisible & KEYBOARD_PANE)
	{
		RECT kbrect = { 0 };
		kbdWnd.GetWindowRect(&kbrect);
		int kbh = kbrect.bottom - kbrect.top;
		int kbtop = rect.bottom - kbh;
		if (kbtop > 0)
		{
			kbdWnd.SetWindowPos(NULL, rect.left, kbtop, rect.right - rect.left, kbh, SWP_NOZORDER|SWP_NOACTIVATE/*|SWP_NOSIZE*/);
			rect.bottom -= kbh;
		}
		else
		{
			bVisible &= ~KEYBOARD_PANE;
			kbdWnd.ShowWindow(SW_HIDE);
			UISetCheck(ID_VIEW_KEYBOARD, 0);
		}
	}

	// resize client window
	if(m_hWndClient != NULL)
		::SetWindowPos(m_hWndClient, NULL, rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			SWP_NOZORDER | SWP_NOACTIVATE);
}

void MainFrame::SaveBackup()
{
	if (theProject)
	{
		bsString path;
		theProject->GetProjectPath(path);
		if (path.Length() > 0 && GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
		{
			bsString bak;
			bak = path;
			bak += ".bak";
			CopyFile(path, bak, 0);
		}
	}
}

void MainFrame::SaveTemp(int sv)
{
	char tmpdir[MAX_PATH];
	char tmppath[MAX_PATH];
	tmpdir[0] = 0;
	SHGetFolderPath(m_hWnd, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, tmpdir);
	snprintf(tmppath, MAX_PATH, "%s\\BasicSynth\\%d.bsprj", tmpdir, GetCurrentProcessId());
	if (sv)
	{
		if (theProject)
			theProject->SaveProject(tmppath);
	}
	else
	{
		DeleteFile(tmppath);
	}
}

void MainFrame::AfterOpenProject()
{
	if (errWnd)
		errWnd->Clear();

	if (!theProject)
		return; // huh?

	kbdWnd.Load();

	bsString path;
	theProject->GetProjectDir(path);
	if (path.Length() > 0)
		SetCurrentDirectory(path);
	prjList.Expand((HTREEITEM)theProject->GetPSData(), TVE_EXPAND|TVE_EXPANDPARTIAL);
	bVisible |= PROJECT_PANE|KEYBOARD_PANE;
	EnablePanes();
	UpdateLayout();
	UpdateProjectUI();
	UpdateEditUI(-1);
	UpdateItemUI(theProject);
	bsString title;
	title = _Module.ProductName;
	title += " - ";
	title += theProject->GetName();
	SetWindowText(title);
}

void MainFrame::UpdateEditUI(int pg)
{
	unsigned long flags = 0;
	if (pg >= 0)
	{
		EditorView *vw = (EditorView *)tabView.GetPageData(pg);
		if (vw)
			flags = vw->EditState();
	}

	UIEnable(ID_EDIT_COPY, flags & VW_ENABLE_COPY);
	UIEnable(ID_EDIT_CUT, flags & VW_ENABLE_CUT);
	UIEnable(ID_EDIT_PASTE, flags & VW_ENABLE_PASTE);
	UIEnable(ID_EDIT_UNDO, flags & VW_ENABLE_UNDO);
	UIEnable(ID_EDIT_REDO, flags & VW_ENABLE_REDO);
	UIEnable(ID_EDIT_GOTOLINE, flags & VW_ENABLE_GOTO);
	UIEnable(ID_EDIT_FIND, flags & VW_ENABLE_FIND);
	UIEnable(ID_EDIT_FINDNEXT, flags & VW_ENABLE_FIND);
	UIEnable(ID_EDIT_SELECTALL, flags & VW_ENABLE_SELALL);
	UIEnable(ID_ITEM_SAVE, flags & VW_ENABLE_FILE);
	UIEnable(ID_ITEM_CLOSE, flags & VW_ENABLE_FILE);
}

void MainFrame::UpdateItemUI(ProjectItem *pi)
{
	int file = 0;
	int enable = 0;
	if (pi)
	{
		enable = pi->ItemActions();
		file = pi->GetEditor() != 0;
	}
	UIEnable(ID_ITEM_ADD, enable & ITM_ENABLE_ADD);
	UIEnable(ID_ITEM_NEW, enable & ITM_ENABLE_NEW);
	UIEnable(ID_ITEM_EDIT, enable & ITM_ENABLE_EDIT);
	UIEnable(ID_ITEM_COPY, enable & ITM_ENABLE_COPY);
	UIEnable(ID_ITEM_REMOVE, enable & ITM_ENABLE_REM);
	UIEnable(ID_ITEM_PROPERTIES, enable & ITM_ENABLE_PROPS);
	UIEnable(ID_ITEM_CLOSE, enable & ITM_ENABLE_CLOSE && file);
	UIEnable(ID_ITEM_SAVE, enable & ITM_ENABLE_SAVE && file);
}

void MainFrame::UpdateProjectUI()
{
	int enable = theProject != 0;
	UIEnable(ID_PROJECT_SAVE, enable);
	UIEnable(ID_PROJECT_PROPERTIES, enable);
	UIEnable(ID_PROJECT_GENERATE, enable);
	UIEnable(ID_PROJECT_PLAY, enable);
	UIEnable(ID_VIEW_PROJECT, enable);
	UIEnable(ID_VIEW_KEYBOARD, enable);
	UIEnable(ID_ITEM_ERRORS, enable);
}

//////////////// Callbacks ///////////////////////////////
LRESULT MainFrame::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
/*	if (bVisible & KEYBOARD_PANE)
	{
		// all of the area of the main window is covered by child
		// windows except for the small area to the right of the
		// keyboard window. That's the only thing we ever need to
		// paint, and we paint it with the dialog background color.
		HBRUSH br = GetSysColorBrush(COLOR_BTNFACE);
		if (IsRectEmpty(&ps.rcPaint))
		{
			RECT rc;
			GetClientRect(&rc);
			FillRect(dc, &rc, br);
		}
		else
			FillRect(dc, &ps.rcPaint, br);
	}*/
	EndPaint(&ps);
	return 0;
}

LRESULT MainFrame::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT MainFrame::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT MainFrame::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam != WA_INACTIVE)
	{
	}
	return 0;
}

LRESULT MainFrame::OnSuspend(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	StopPlayer();
	return 0;
}

LRESULT MainFrame::OnTerminate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	StopPlayer();
	//SaveTemp(1);
	return 0;
}

LRESULT MainFrame::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT MainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if (CloseProject(1))
	{
		mruList.WriteToRegistry(mruRegKey);
		prjOptions.Save();
		DestroyWindow();
	}
	return 0;
}

LRESULT MainFrame::OnOpenProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (OpenProject(0))
	{
		bsString path;
		theProject->GetProjectPath(path);
		mruList.AddToList(path);
		AfterOpenProject();
	}

	return 0;
}

LRESULT MainFrame::OnSaveProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!theProject)
		return 0;
	SaveBackup();
	SaveAllEditors(0);
	SaveProject(0);
	return 0;
}

LRESULT MainFrame::OnSaveProjectAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!theProject)
		return 0;
	SaveProject(1);
	return 0;
}

LRESULT MainFrame::OnRecentProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char fname[MAX_PATH];
	if (mruList.GetFromList(wID, fname, MAX_PATH))
	{
		SynthProject::NormalizePath(fname);
		if (OpenProject(fname))
		{
			AfterOpenProject();
			mruList.MoveToTop(wID);
		}
	}
	return 0;
}

LRESULT MainFrame::OnNewProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (NewProject())
	{
		bsString path;
		theProject->GetProjectPath(path);
		mruList.AddToList(path);
		AfterOpenProject();
	}

	return 0;
}

LRESULT MainFrame::OnEditItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditItem();
	return 0;
}

LRESULT MainFrame::OnSaveItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SaveItem();
	return 0;
}

LRESULT MainFrame::OnCloseItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CloseItem();
	return 0;
}

LRESULT MainFrame::OnRemoveItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	RemoveItem();
	return 0;
}

LRESULT MainFrame::OnCopyItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CopyItem();
	return 0;
}

LRESULT MainFrame::OnItemProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ItemProperties();
	return 0;
}

LRESULT MainFrame::OnNewItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	NewItem();
	return 0;
}

LRESULT MainFrame::OnAddItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	AddItem();
	return 0;
}

LRESULT MainFrame::OnItemErrors(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!errWnd)
	{
		errWnd = new ScoreErrorsDlg;
		errWnd->Create(m_hWnd);
		errWnd->SetWindowPos(NULL, 100, 200, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		errWnd->ShowWindow(SW_SHOW);
		UISetCheck(ID_ITEM_ERRORS, 1);
	}
	else
	{
		errWnd->DestroyWindow();
		delete errWnd;
		errWnd = 0;
		UISetCheck(ID_ITEM_ERRORS, 0);
	}
	return 0;
}

LRESULT MainFrame::OnProjectGenerate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	StopPlayer();
	GenerateDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT MainFrame::OnProjectPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	StartPlayer();
	return 0;
}

LRESULT MainFrame::OnProjectOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OptionsDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::OnEditUndo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditUndo();
	return 0;
}

LRESULT MainFrame::OnEditRedo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditRedo();
	return 0;
}

LRESULT MainFrame::OnEditCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditCopy();
	return 0;
}

LRESULT MainFrame::OnEditCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditCut();
	return 0;
}

LRESULT MainFrame::OnEditPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditPaste();
	return 0;
}

LRESULT MainFrame::OnEditSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditSelectAll();
	return 0;
}

LRESULT MainFrame::OnEditFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditFind();
	return 0;
}

LRESULT MainFrame::OnEditFindNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditFindNext();
	return 0;
}

LRESULT MainFrame::OnEditGotoLine(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditGoto();
	return 0;
}

LRESULT MainFrame::OnEditUpdateUI(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	return 0;
}

LRESULT MainFrame::OnViewProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bVisible ^= PROJECT_PANE;
	if (bVisible & PROJECT_PANE)
	{
		UISetCheck(ID_VIEW_PROJECT, 1);
		splitTop.SetSinglePaneMode(SPLIT_PANE_NONE);
	}
	else
	{
		UISetCheck(ID_VIEW_PROJECT, 0);
		splitTop.SetSinglePaneMode(SPLIT_PANE_RIGHT);
	}
	return 0;
}

LRESULT MainFrame::OnViewKeyboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bVisible ^= KEYBOARD_PANE;
	UISetCheck(ID_VIEW_KEYBOARD, bVisible & KEYBOARD_PANE ? 1 : 0);
	kbdWnd.ShowWindow(bVisible & KEYBOARD_PANE ? SW_SHOW : SW_HIDE);
	UpdateLayout();
	return 0;
}

LRESULT MainFrame::OnViewToolBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bVisible ^= TOOLBAR_PANE;
	int show = bVisible & TOOLBAR_PANE ? 1 : 0;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, show);
	UISetCheck(ID_VIEW_TOOLBAR, show);
	UpdateLayout();
	return 0;
}

LRESULT MainFrame::OnViewStatusBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	BOOL show = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, show ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, show);
	UpdateLayout();
	kbdWnd.InvalidateRect(0,1);
	return 0;
}

LRESULT MainFrame::OnAppAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CAboutDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::OnHelpContents(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bsString path;
	path = prjOptions.installDir;
	path += "\\BSynthHelp.chm";
	HWND h = HtmlHelp(m_hWnd, path, HH_DISPLAY_TOC, NULL);
	if (h == 0)
		Alert("Cannot locate help file.", "Ooops");
	return 0;
}

LRESULT MainFrame::OnWindowClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CloseFile();
/*	int pg = tabView.GetActivePage();
	if (pg >= 0)
	{
		EditorView *vw = (EditorView *)tabView.GetPageData(pg);
		ProjectItem *itm = vw->GetItem();
		itm->CloseItem();
	}*/
	return 0;
}

LRESULT MainFrame::OnWindowSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SaveFile();
	return 0;
}

LRESULT MainFrame::OnWindowCloseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UpdateEditUI(-1);
	//if (SaveAllEditors(1))
	CloseAllEditors();
	return 0;
}

LRESULT MainFrame::OnNextPane(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int pg = tabView.GetActivePage();
	if (++pg >= tabView.GetPageCount())
		pg = 0;
	tabView.SetActivePage(pg);
	return 0;
}

LRESULT MainFrame::OnPrevPane(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int pg = tabView.GetActivePage();
	if (--pg < 0)
		pg = tabView.GetPageCount() - 1;
	tabView.SetActivePage(pg);
	return 0;
}

LRESULT MainFrame::OnWindowActivate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int pg = wID - ID_WINDOW_TABFIRST;
	tabView.SetActivePage(pg);
	UpdateEditUI(pg);
	return 0;
}

LRESULT MainFrame::OnPageActivated(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	int pg = pnmh->idFrom;
	UpdateEditUI(pg);
	if (pg >= 0)
	{
		EditorView *vw = (EditorView *)tabView.GetPageData(pg);
		if (vw)
		{
			ProjectItem *itm = vw->GetItem();
			if (itm)
			{
				prjList.SelectItem((HTREEITEM)itm->GetPSData());
				if (itm->GetType() == PRJNODE_INSTR)
				{
					InstrItem *ii = (InstrItem *)itm;
					kbdWnd.SelectInstrument(ii->GetConfig());
				}
			}
		}
	}
	return 0;
}

LRESULT MainFrame::OnTabContextMenu(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	TBVCONTEXTMENUINFO *cmi = (TBVCONTEXTMENUINFO*)pnmh;
	EditorView *ed = (EditorView *) tabView.GetPageData(cmi->hdr.idFrom);
	if (ed)
	{
		UpdateItemUI(ed->GetItem());
		CMenu menu;
		menu.LoadMenu(IDR_POPUP);
		HMENU pop = menu.GetSubMenu(0);
		if (pop)
			::TrackPopupMenu(pop, TPM_RIGHTBUTTON|TPM_VERTICAL, cmi->pt.x, cmi->pt.y, 0, m_hWnd, NULL);
	}
	return 0;
}

ProjectItem *MainFrame::GetClickedItem()
{
	TVHITTESTINFO ht;
	GetCursorPos(&ht.pt);
	prjList.ScreenToClient(&ht.pt);
	ht.flags = 0;
	ht.hItem = 0;
	prjList.HitTest(&ht);
	if (ht.flags & TVHT_ONITEM)
		return (ProjectItem*)prjList.GetItemData(ht.hItem);
	return 0;
}

LRESULT MainFrame::OnPrjRClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	int subMnu = -1;
	ProjectItem *itm = GetClickedItem();
	if (itm)
	{
		UpdateItemUI(itm);
		prjList.SelectItem((HTREEITEM)itm->GetPSData());
		switch (itm->GetType())
		{
		case PRJNODE_PROJECT:
			subMnu = 1;
			break;
		case PRJNODE_WAVEOUT:
			subMnu = 3;
			break;
		case PRJNODE_SYNTH:
			subMnu = 2;
			break;
		case PRJNODE_NOTELIST:
		case PRJNODE_SEQLIST:
		case PRJNODE_TEXTLIST:
		case PJRNODE_SCRIPTLIST:
		case PRJNODE_LIBLIST:
		case PRJNODE_WVFLIST:
			subMnu = 4;
			break;
		default:
			subMnu = 5;
			break;
		}
	}
	else
		subMnu = 0;
	if (subMnu >= 0)
	{
		CMenu menu;
		menu.LoadMenu(IDR_POPUP);
		HMENU pop = menu.GetSubMenu(subMnu);
		if (pop)
		{
			POINT scrn;
			GetCursorPos(&scrn);
			::TrackPopupMenu(pop, TPM_RIGHTBUTTON|TPM_VERTICAL, scrn.x, scrn.y, 0, m_hWnd, NULL);
		}
	}
	return 0;
}

LRESULT MainFrame::OnPrjSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMTREEVIEW *nmtv = (NMTREEVIEW*)pnmh;
	UpdateItemUI((ProjectItem*)nmtv->itemNew.lParam);
	return 0;
}

LRESULT MainFrame::OnPrjDblclick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
// If you call OpenEditor directly, you won't get the 
// focus moved to the new editor (???). The tree view
// takes it back.... (arrrggghhhh...)
//	OpenEditor((ProjectItem *)GetClickedItem());
	PostMessage(WM_COMMAND, ID_ITEM_EDIT, 0);
	return 1;
}

LRESULT MainFrame::OnPrjDelItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMTREEVIEW *nmtv = (NMTREEVIEW*)pnmh;
	ProjectItem *pi = (ProjectItem *)nmtv->itemOld.lParam;
	if (pi)
		delete pi;
	return 1;
}

void MainFrame::AddNode(ProjectItem *itm, ProjectItem *sib)
{
	if (itm == 0)
		return;

	ProjectItem *parent = itm->GetParent();

	TVINSERTSTRUCT tvi;
	memset(&tvi, 0, sizeof(tvi));
	if (sib)
		tvi.hInsertAfter = (HTREEITEM) sib->GetPSData();
	else
		tvi.hInsertAfter = TVI_LAST;
	if (parent)
		tvi.hParent = (HTREEITEM) parent->GetPSData();
	else
		tvi.hParent = TVI_ROOT;
	tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
	if (!itm->IsLeaf())
	{
		tvi.item.mask |= TVIF_CHILDREN; 
		tvi.item.cChildren = 1;
	}
	else
		tvi.item.cChildren = 0;
	tvi.item.pszText = (LPSTR) itm->GetName();
	tvi.item.lParam = (LPARAM) itm;
	itm->SetPSData((void*) prjList.InsertItem(&tvi));
}

void MainFrame::SelectNode(ProjectItem *pi)
{
	if (pi)
		prjList.SelectItem((HTREEITEM)pi->GetPSData());
}

void MainFrame::RemoveNode(ProjectItem *itm)
{
	if (itm == 0 || itm->GetPSData() == 0)
		return;
	if (itm->GetEditor())
		CloseEditor(itm);

	HTREEITEM ht = (HTREEITEM) itm->GetPSData();
	itm->SetPSData(0);
	prjList.SetItemData(ht, 0);
	prjList.DeleteItem(ht);
	if (itm->GetType() == PRJNODE_NOTELIST)
	{
		if (errWnd)
			errWnd->RemoveItem(itm);
	}
}

void MainFrame::UpdateNode(ProjectItem *itm)
{
	if (itm)
		prjList.SetItemText((HTREEITEM) itm->GetPSData(), itm->GetName());
}

void MainFrame::MoveNode(ProjectItem *itm, ProjectItem *prev)
{
	if (!itm || !prev)
		return;

	RemoveNode(itm);
	AddNode(itm, prev);
}

ProjectItem *MainFrame::GetSelectedNode()
{
	HTREEITEM itm = prjList.GetSelectedItem();
	if (itm != NULL)
		return (ProjectItem *)prjList.GetItemData(itm);
	return 0;
}

ProjectItem *MainFrame::FirstChild(ProjectItem *itm)
{
	if (itm == 0 || itm->GetPSData() == 0)
		return 0;

	HTREEITEM child = prjList.GetChildItem((HTREEITEM) itm->GetPSData());
	if (child)
		return (ProjectItem *) prjList.GetItemData(child);
	return 0;
}

ProjectItem *MainFrame::NextSibling(ProjectItem *itm)
{
	if (itm == 0 || itm->GetPSData() == 0)
		return 0;

	HTREEITEM child = prjList.GetNextSiblingItem((HTREEITEM) itm->GetPSData());
	if (child)
		return (ProjectItem *) prjList.GetItemData(child);
	return 0;
}

void MainFrame::RemoveAll()
{
	prjList.DeleteAllItems();
	theProject = 0;
}

int MainFrame::CloseAllEditors()
{
	tabView.ShowWindow(SW_HIDE);
	int ndx = 0;
	while (ndx < tabView.GetPageCount())
	{
		EditorView *vw = (EditorView*)tabView.GetPageData(0);
		ProjectItem *pi = vw->GetItem();
		if (!pi->CloseItem())
			ndx++;
	}
	tabView.ShowWindow(SW_SHOW);
	return 1;
}

int MainFrame::Verify(const char *msg, const char *title)
{
	switch (MessageBox(msg, title, MB_YESNOCANCEL|MB_ICONQUESTION))
	{
	case IDYES:
		return 1;
	case IDNO:
		return 0;
	}
	return -1;
}

PropertyBox *MainFrame::CreatePropertyBox(ProjectItem *pi, int type)
{
	PropertyBox *pb = 0;
	if (type == 0)
		type = pi->GetType();
	switch (type)
	{
	case PRJNODE_PROJECT:
	case PRJNODE_SYNTH:
	case PRJNODE_WAVEOUT:
		{
			ProjectPropertiesDlg *p = new ProjectPropertiesDlg;
			p->SetItem(pi);
			pb = static_cast<PropertyBox*>(p);
		}
		break;
	case PRJNODE_MIXER:
		{
			MixerSetupDlg *m = new MixerSetupDlg;
			m->SetItem(pi);
			pb = static_cast<PropertyBox*>(m);
		}
		break;
	case PRJNODE_REVERB:
	case PRJNODE_FLANGER:
	case PRJNODE_ECHO:
		{
			EffectsSetupDlg *e = new EffectsSetupDlg;
			e->SetItem(pi);
			pb = static_cast<PropertyBox*>(e);
		}
		break;
	case PRJNODE_NOTEFILE:
	case PRJNODE_SEQFILE:
	case PRJNODE_TEXTFILE:
	case PRJNODE_SCRIPT:
		{
			FilePropertiesDlg *f = new FilePropertiesDlg;
			f->SetItem(pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_LIBLIST:
		break;
	case PRJNODE_LIB:
		{
			LibPropertiesDlg *f = new LibPropertiesDlg;
			f->SetItem(pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_SELINSTR:
		{
			InstrSelectDlg *i = new InstrSelectDlg;
			i->SetItem(pi);
			pb = static_cast<PropertyBox*>(i);
		}
		break;
	case PRJNODE_INSTRLIST:
		break;
	case PRJNODE_LIBINSTR:
	case PRJNODE_INSTR:
		{
			InstrPropertiesDlg *i = new InstrPropertiesDlg;
			i->SetItem(pi);
			pb = static_cast<PropertyBox*>(i);
		}
		break;
	case PRJNODE_WVTABLE:
		{
			NamePropertiesDlg *n = new NamePropertiesDlg;
			n->SetItem(pi);
			pb = static_cast<PropertyBox*>(n);
		}
		break;
	case PRJNODE_WVFILE:
		{
			WavefilePropertiesDlg *w = new WavefilePropertiesDlg;
			w->SetItem(pi);
			pb = static_cast<PropertyBox*>(w);
		}
		break;
	}
	return pb;
}

FormEditor *MainFrame::CreateFormEditor(ProjectItem *pi)
{
	ScrollForm *scrl = new ScrollForm(0);
	scrl->Create(tabView, rcDefault, "scroll", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL, 0, ID_SCROLL_WND, 0);
	FormEditorWin *form = new FormEditorWin;
	form->SetItem(pi);
	Size sz(0,0);
	form->GetSize(sz);
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = sz.Width+1;
	rc.bottom = sz.Height+1;
	form->Create(scrl->m_hWnd, rc, "FormEdit", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, ID_FORM_WND);
	form->SetScrollWin(scrl);
	form->SetPSData((void*)form->m_hWnd);
	scrl->SetForm(form);
	tabView.AddPage(scrl->m_hWnd, pi->GetName(), -1, static_cast<EditorView*>(form));
	::SetFocus(form->m_hWnd);
	return form;
}

TextEditor *MainFrame::CreateTextEditor(ProjectItem *pi)
{
	if (!pi)
		return 0;
	TextEditorWin *ed = new TextEditorWin;
	if (ed)
	{
		ed->SetItem(pi);
		ed->Create(tabView);
		ed->SetPSData((void*)ed->m_hWnd);
		tabView.AddPage(ed->m_hWnd, pi->GetName(), -1, static_cast<EditorView*>(ed));
		::SetFocus(ed->m_hWnd);
		return ed;
	}
	return 0;
}

// NOTE: for now, we only create editors as tabs.
// If that changes, then search other info to find
// the editor and show/close it.

int MainFrame::CloseEditor(ProjectItem *itm)
{
	if (!itm)
		return 0;

	EditorView *vw = itm->GetEditor();
	int pg;
	for (pg = 0; pg < tabView.GetPageCount(); pg++)
	{
		if ((EditorView*)tabView.GetPageData(pg) == vw)
		{
			tabView.RemovePage(pg);
			break;
		}
	}
	itm->SetEditor(0);
	delete vw;

	return 1;
}

EditorView *MainFrame::GetActiveEditor()
{
	int pg = tabView.GetActivePage();
	if (pg >= 0)
		return (EditorView*) tabView.GetPageData(pg);
	return 0;
}

int MainFrame::OpenEditor(ProjectItem *itm)
{
	if (!itm)
		return 0;
	EditorView *vw = itm->GetEditor();
	if (vw)
	{
		if (vw == GetActiveEditor())
		{
			::SetFocus((HWND)vw->GetPSData());
			return 1;
		}
		int pgcnt = tabView.GetPageCount();
		for (int pg = 0; pg < pgcnt; pg++)
		{
			if ((EditorView*)tabView.GetPageData(pg) == vw)
			{
				tabView.SetActivePage(pg);
				return 1;
			}
		}
		itm->SetEditor(0);
	}
	if (itm->EditItem())
		return 1;

	if (itm->ItemProperties())
		UpdateNode(itm);

	return 0;
}

int MainFrame::SaveAllEditors(int query)
{
	for (int pg = 0; pg < tabView.GetPageCount(); pg++)
	{
		EditorView *vw = (EditorView *) tabView.GetPageData(pg);
		if (vw->IsChanged())
		{
			if (query)
			{
				bsString msg;
				msg = tabView.GetPageTitle(pg);
				msg += " has changed. Save?";
				int res = Verify(msg, "Wait...");
				if (res == 0)
					continue;
				if (res == -1)
					return 0;
			}
			ProjectItem *itm = vw->GetItem();
			if (itm)
				itm->SaveItem();
		}
	}
	return 1;
}

void MainFrame::EditStateChanged()
{
	int pg = tabView.GetActivePage();
	UpdateEditUI(pg);
}

int MainFrame::BrowseFile(int open, char *file, const char *spec, const char *ext)
{
	char spcbuf[MAX_PATH];
	strncpy(spcbuf, spec, MAX_PATH);
	if (strstr(spcbuf, "*.*") == NULL)
		strcat(spcbuf, "All Files|*.*|");
	char *pipe = spcbuf;
	while ((pipe = strchr(pipe, '|')) != 0)
		*pipe++ = '\0';

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.Flags = OFN_NOCHANGEDIR;
	ofn.hwndOwner = GetActiveWindow(); //m_hWnd;
	ofn.lpstrFilter = spcbuf;
	ofn.lpstrDefExt = ext; 
	ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	int result;
	if (open)
	{
		result = GetOpenFileName(&ofn);
	}
	else
	{
		// the term 'brain damage' comes to mind here...
		pipe = file;
		while ((pipe = strchr(pipe, '/')) != 0)
			*pipe++ = '\\';
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		result = GetSaveFileName(&ofn);
	}
	if (result)
		SynthProject::NormalizePath(file);
	return result;
}

int MainFrame::BrowseFile(int open, bsString& file, const char *spec, const char *ext)
{
	char fnbuf[MAX_PATH];
	strncpy(fnbuf, file, MAX_PATH);
	int result = BrowseFile(open, fnbuf, spec, ext);
	if (result)
		file = fnbuf;
	return result;
}

int MainFrame::QueryValue(const char *prompt, char *value, int len)
{
	int pg = tabView.GetActivePage();
	QueryValueDlg dlg(prompt, value, len);
	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		if (pg >= 0)
			::SetFocus(tabView.GetPageHWND(pg));
		return 1;
	}
	return 0;
}

int MainFrame::Alert(const char *msg, const char *title)
{
	MessageBox(msg, title ? title : "Wait...", MB_OK|MB_ICONHAND);
	return 0;
}

void MainFrame::InitPlayer()
{
	kbdWnd.InitInstrList();
	kbdWnd.UpdateChannels();
}

int MainFrame::StopPlayer()
{
	if (kbdWnd.IsWindow())
		return kbdWnd.Stop();
	return 0;
}

void MainFrame::StartPlayer()
{
	if (kbdWnd.IsWindow())
		kbdWnd.Start();
}

void MainFrame::ClearPlayer()
{
	if (kbdWnd.IsWindow())
		kbdWnd.Clear();
}

void MainFrame::InstrAdded(InstrConfig *inst)
{
	kbdWnd.AddInstrument(inst);
}

void MainFrame::InstrRemoved(InstrConfig *inst)
{
	kbdWnd.RemoveInstrument(inst);
}

void MainFrame::InstrChanged(InstrConfig *inst)
{
	kbdWnd.UpdateInstrument(inst);
}

void MainFrame::MixerChanged()
{
	kbdWnd.UpdateChannels();
}

void MainFrame::GenerateStarted()
{
	if (errWnd)
		errWnd->Clear();
}

void MainFrame::GenerateFinished()
{
	if (errWnd)
		errWnd->Refresh();
}

void MainFrame::Generate(int autoStart, int todisk)
{
	StopPlayer();
	GenerateDlg dlg(autoStart, todisk);
	dlg.DoModal(m_hWnd);
}
