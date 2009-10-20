//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "Stdafx.h"
#include "resource.h"
#include "GenerateDlg.h"

long GenerateDlg::playFrom;
long GenerateDlg::playTo;
int  GenerateDlg::playLive;

DWORD WINAPI GenerateDlg::GenerateProc(LPVOID param)
{
	int err;
	try
	{
		err = theProject->Generate(!playLive, playFrom, playTo);
	}
	catch(...)
	{
		err = -1;
	}
	prjGenerate->Finished();
	ExitThread((DWORD)err);
	return 0;
}

LRESULT GenerateDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	EnableOK(1, 0);
	SetButtonImage(IDC_START, IDI_GENERATE);
	SetButtonImage(IDC_STOP, IDI_STOP);
	SetButtonImage(IDC_PAUSE, IDI_PAUSE);
	SetButtonImage(IDC_PLAY_LIVE, IDI_SPEAKER);
	SetButtonImage(IDC_DISK, IDI_AUDIOCD);

	ed = GetDlgItem(IDC_TEXT);
	tm = GetDlgItem(IDC_TIME);
	tm.SetWindowTextA("00:00");
	lpk = GetDlgItem(IDC_LEFT_PEAK);
	rpk = GetDlgItem(IDC_RIGHT_PEAK);
	int id = IDC_PLAY_ALL;
	if (playFrom > 0 || playTo > 0)
		id = IDC_PLAY_SOME;
	CheckRadioButton(IDC_PLAY_ALL, IDC_PLAY_SOME, id);
	FormatTime(GetDlgItem(IDC_PLAY_FROM), playFrom);
	FormatTime(GetDlgItem(IDC_PLAY_TO), playTo);
	CheckRadioButton(IDC_PLAY_LIVE, IDC_DISK, playLive ? IDC_PLAY_LIVE : IDC_DISK);
	if (genAuto)
		PostMessage(WM_COMMAND, IDC_START, 0);
	return TRUE;
}

LRESULT GenerateDlg::OnStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	prjFrame->GenerateStarted();
	EnableOK(0, 0);
	ed.AppendText("---------- Start ----------\r\n");
	canceled = 0;

	if (!genAuto)
	{
		playLive = IsDlgButtonChecked(IDC_PLAY_LIVE);
		if (IsDlgButtonChecked(IDC_PLAY_SOME))
		{
			playFrom = GetTimeValue(IDC_PLAY_FROM);
			playTo  = GetTimeValue(IDC_PLAY_TO);
		}
		else
		{
			playFrom = 0;
			playTo = 0;
		}
	}
	lastMsg = "";
	lastTime = playFrom;
	FormatTime(tm, lastTime);
	lftPeak = 0;
	rgtPeak = 0;
	lftMax = 0;
	rgtMax = 0;
	FormatPeak();
	prjGenerate = static_cast<GenerateWindow*>(this);

	// start thread
	genThreadH = CreateThread(NULL, 0, GenerateProc, NULL, CREATE_SUSPENDED, &genThreadID);
	if (genThreadH == INVALID_HANDLE_VALUE)
		ed.AppendText("Cannot create playback thread.\r\n");
	else
	{
		if (!SetThreadPriority(genThreadH, THREAD_PRIORITY_HIGHEST))
			OutputDebugString("Can't up priority\r\n");
		ResumeThread(genThreadH);
		CheckDlgButton(IDC_PAUSE, BST_UNCHECKED);
		EnableOK(0, 1);
	}
	return 0;
}

LRESULT GenerateDlg::OnStop(WORD cd, WORD wID, HWND hwnd, BOOL& bHandled)
{
	EnterCriticalSection(&guard);
	lastMsg += "*Cancel*\r\n";
	ed.AppendText(lastMsg);
	lastMsg = "";
	canceled = 1;
	try
	{
		if (theProject->cvtActive)
			theProject->cvtActive->Cancel();
	}
	catch(...)
	{
	}
	LeaveCriticalSection(&guard);
	return 0;
}

LRESULT GenerateDlg::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IsDlgButtonChecked(IDC_PAUSE))
	{
		if (!theProject->Pause())
			CheckDlgButton(IDC_PAUSE, 0);
	}
	else
		theProject->Resume();
	return 0;
}

LRESULT GenerateDlg::OnGenFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	DWORD code = 0;
	if (genThreadH != INVALID_HANDLE_VALUE)
	{
		try 
		{
			WaitForSingleObject(genThreadH, 10000);
		} 
		catch (...) 
		{
		}
		GetExitCodeThread(genThreadH, &code);
		CloseHandle(genThreadH);
		genThreadH = INVALID_HANDLE_VALUE;
	}
	prjGenerate = 0;
	char pk[1024];
	snprintf(pk, 1024, "Peak: [%.3f, %.3f]\r\n-------- Finished ---------\r\n", lftMax, rgtMax);
	ed.AppendText(pk);
	EnableOK(1, 0);
	prjFrame->GenerateFinished();
	if (code == 0 && genAuto)
		EndDialog(IDOK);
	return 0;
}

LRESULT GenerateDlg::OnUpdateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	EnterCriticalSection(&guard);
	if (lastMsg.Length() != 0)
	{
		ed.AppendText(lastMsg);
		lastMsg = "";
	}
	LeaveCriticalSection(&guard);
	return 0;
}

LRESULT GenerateDlg::OnUpdateTime(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	lParam += playFrom;
	if (lParam > lastTime)
	{
		lastTime = lParam;
		FormatTime(tm, lastTime);
	}
	return 0;
}

LRESULT GenerateDlg::OnUpdatePeak(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FormatPeak();
	return 0;
}

LRESULT GenerateDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(IDOK);
	return 0;
}

LRESULT GenerateDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(IDCANCEL);
	return 0;
}

void GenerateDlg::FormatTime(HWND w, long secs)
{
	char txt[80];
	snprintf(txt, 80, "%02d:%02d", secs / 60, secs % 60);
	::SendMessage(w, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)txt);
}

void GenerateDlg::FormatPeak()
{
	char pkText[256];
	snprintf(pkText, 256, "%.3f", lftPeak);
	lpk.SetWindowText(pkText);
	snprintf(pkText, 256, "%.3f", rgtPeak);
	rpk.SetWindowText(pkText);
	if (lftPeak > 1.0 || rgtPeak > 1.0)
	{
		snprintf(pkText, 256, "Out of range (%.3f, %.3f) at %02d:%02d\r\n", 
			lftPeak, rgtPeak, lastTime / 60, lastTime % 60);
		ed.AppendText(pkText);
	}
	if (lftPeak > lftMax)
		lftMax = lftPeak;
	if (rgtPeak > rgtMax)
		rgtMax = rgtPeak;
}

void GenerateDlg::EnableOK(int e, int c)
{
	::EnableWindow(GetDlgItem(IDOK), e);
	::EnableWindow(GetDlgItem(IDC_STOP), c);
	::EnableWindow(GetDlgItem(IDC_START), !c);
	::EnableWindow(GetDlgItem(IDC_PAUSE), c);
}

long GenerateDlg::GetTimeValue(int id)
{
	char buf[80];
	GetDlgItemText(id, buf, 80);
	long sec = 0;
	char *col = strchr(buf, ':');
	if (col == NULL)
	{
		sec = atol(buf);
	}
	else
	{
		*col++ = 0;
		sec = (atol(buf) * 60) + atol(col);
	}
	return sec;
}

void GenerateDlg::AddMessage(const char *s)
{
	EnterCriticalSection(&guard);
	lastMsg += s;
	lastMsg += "\r\n";
	LeaveCriticalSection(&guard);
	PostMessage(WM_GENMSG, 0, 0);
}

void GenerateDlg::UpdateTime(long tmval)
{
	PostMessage(WM_GENUPDTM, 0, tmval);
}

void GenerateDlg::Finished()
{
	PostMessage(WM_GENEND, 0, 0);
}

int GenerateDlg::WasCanceled()
{
	return canceled;
}

void GenerateDlg::UpdatePeak(AmpValue lft, AmpValue rgt)
{
	lftPeak = lft;
	rgtPeak = rgt;
	PostMessage(WM_GENUPDPK, 0, 0);
}

void GenerateDlg::SetButtonImage(int ctrl, int imgid)
{
	HICON ico = (HICON) LoadImage(_Module.GetResourceInstance(), (LPCSTR) imgid, IMAGE_ICON, 32, 32, 0);
	CButton btn = GetDlgItem(ctrl);
	btn.SetIcon(ico);
}
