//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#ifndef GENERATE_DLG_H
#define GENERATE_DLG_H

#pragma once

class GenerateDlg : public CDialogImpl<GenerateDlg>, public GenerateWindow
{
private:
	CEdit ed;
	CEdit tm;
	CEdit lpk;
	CEdit rpk;
	bsString lastMsg;
	long lastTime;
	int canceled;
	int genAuto;

	AmpValue lftPeak;
	AmpValue rgtPeak;
	AmpValue lftMax;
	AmpValue rgtMax;
	CRITICAL_SECTION guard;
	HANDLE genThreadH;
	DWORD  genThreadID;

	void EnableOK(int e, int c);
	long GetTimeValue(int id);
	void FormatTime(HWND w, long secs);
	void FormatPeak();
	void SetButtonImage(int ctrl, int imgid);

public:
	static long playFrom;
	static long playTo;
	static int  playLive;

	GenerateDlg(int a = 0, int pl = -1)
	{
		genAuto = a;
		genThreadID = 0;
		genThreadH = 0;
		lastTime = 0;
		canceled = 0;
		if (pl >= 0)
			playLive = pl;
		InitializeCriticalSection(&guard);
	}

	~GenerateDlg()
	{
		prjGenerate = 0;
		DeleteCriticalSection(&guard);
	}

	static DWORD WINAPI GenerateDlg::GenerateProc(LPVOID param);

	virtual void AddMessage(const char *s);
	virtual void UpdateTime(long tm);
	virtual void UpdatePeak(AmpValue lft, AmpValue rgt);
	virtual void Finished();
	virtual int WasCanceled();

	enum { IDD = IDD_GENERATE };

	BEGIN_MSG_MAP(GenerateDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_GENMSG, OnUpdateMsg)
		MESSAGE_HANDLER(WM_GENEND, OnGenFinished)
		MESSAGE_HANDLER(WM_GENUPDTM, OnUpdateTime)
		MESSAGE_HANDLER(WM_GENUPDPK, OnUpdatePeak)
		COMMAND_ID_HANDLER(IDC_START, OnStart)
		COMMAND_ID_HANDLER(IDC_STOP, OnStop)
		COMMAND_ID_HANDLER(IDC_PAUSE, OnPause)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnStart(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPause(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUpdateTime(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnUpdatePeak(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnUpdateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnGenFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
};

#endif
