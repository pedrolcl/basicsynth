// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

struct FMInstrParam
{
	int algorithm;
	AmpValue carStart;
	FrqValue carAtk;
	AmpValue carPk;
	FrqValue carDec;
	AmpValue carSus;
	FrqValue carRel;
	AmpValue carEnd;

	FrqValue mod1Mult;
	AmpValue mod1Start;
	FrqValue mod1Atk;
	AmpValue mod1Pk;
	FrqValue mod1Dec;
	AmpValue mod1Sus;
	FrqValue mod1Rel;
	AmpValue mod1End;

	FrqValue mod2Mult;
	AmpValue mod2Start;
	FrqValue mod2Atk;
	AmpValue mod2Pk;
	FrqValue mod2Dec;
	AmpValue mod2Sus;
	FrqValue mod2Rel;
	AmpValue mod2End;
};

class CMainDlg : public CDialogImpl<CMainDlg>, 
	public CUpdateUI<CMainDlg>,
	public CMessageFilter, 
	public CIdleHandler
{
private:
	CTrackBarCtrl carFrq;
	CTrackBarCtrl carAtk;
	CTrackBarCtrl carVol;
	CTrackBarCtrl carDec;
	CTrackBarCtrl carSus;
	CTrackBarCtrl carRel;
	CEdit carFrqEd;
	CEdit carAtkEd;
	CEdit carVolEd;
	CEdit carDecEd;
	CEdit carSusEd;
	CEdit carRelEd;

	CTrackBarCtrl mod1Mul;
	CTrackBarCtrl mod1Ndx;
	CTrackBarCtrl mod1Atk;
	CTrackBarCtrl mod1Pck;
	CTrackBarCtrl mod1Dec;
	CTrackBarCtrl mod1Sus;
	CTrackBarCtrl mod1Rel;
	CTrackBarCtrl mod1End;
	CEdit mod1MulEd;
	CEdit mod1NdxEd;
	CEdit mod1AtkEd;
	CEdit mod1PckEd;
	CEdit mod1DecEd;
	CEdit mod1SusEd;
	CEdit mod1RelEd;
	CEdit mod1EndEd;

	CTrackBarCtrl mod2Mul;
	CTrackBarCtrl mod2Ndx;
	CTrackBarCtrl mod2Atk;
	CTrackBarCtrl mod2Pck;
	CTrackBarCtrl mod2Dec;
	CTrackBarCtrl mod2Sus;
	CTrackBarCtrl mod2Rel;
	CTrackBarCtrl mod2End;
	CEdit mod2MulEd;
	CEdit mod2NdxEd;
	CEdit mod2AtkEd;
	CEdit mod2PckEd;
	CEdit mod2DecEd;
	CEdit mod2SusEd;
	CEdit mod2RelEd;
	CEdit mod2EndEd;

	CEdit durValEd;
	CEdit volValEd;
	FrqValue durTotal;  // duration including relase
	FrqValue durAtkSus; // duration through sustain
	AmpValue volMaster;

	CButton btnPlay;
	CButton btnLoop;
	CButton btnStop;

	int algorithm;
	// Fast oscillator (32-bit phase accumulator):
	//GenWave32 carOsc;
	//GenWave32 mod1Osc;
	//GenWave32 mod2Osc;
	// Interpolating oscillator (slightly less noise):
	GenWaveI carOsc;
	GenWaveI mod1Osc;
	GenWaveI mod2Osc;
	EnvGenADSR carEG;
	EnvGenADSR mod1EG;
	EnvGenADSR mod2EG;

#ifdef USE_DIRECTSOUND
	IDirectSound *dirSndObj;
	IDirectSoundBuffer *dirSndBuf;
#else
	WAVEHDR wh;
	HWAVEOUT woHandle;
#endif
	UINT idTimer;

public:
	CMainDlg();
	~CMainDlg();

	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_CODE_HANDLER(EN_KILLFOCUS, OnKillFocus)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDC_PLAY, OnPlay)
		COMMAND_ID_HANDLER(IDC_LOOP, OnLoop)
		COMMAND_ID_HANDLER(IDC_STOP, OnStop)
		COMMAND_ID_HANDLER(IDC_SAVE, OnSave)
		COMMAND_ID_HANDLER(IDC_STACK, OnStack)
		COMMAND_ID_HANDLER(IDC_STACK2, OnStack2)
		COMMAND_ID_HANDLER(IDC_WYE, OnWye)
		COMMAND_ID_HANDLER(IDC_DELTA, OnDelta)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_HELP2, OnHelp)
		COMMAND_ID_HANDLER(IDC_COPY_CLIP, OnCopyClip)
		COMMAND_ID_HANDLER(IDC_PRE_PIANO, OnPresetPiano)
		COMMAND_ID_HANDLER(IDC_PRE_ORGAN, OnPresetOrgan)
		COMMAND_ID_HANDLER(IDC_PRE_STRINGS, OnPresetStrings)
		COMMAND_ID_HANDLER(IDC_PRE_BRASS, OnPresetBrass)
		COMMAND_ID_HANDLER(IDC_PRE_CLAR, OnPresetClarinet)
		COMMAND_ID_HANDLER(IDC_PRE_FLUTE, OnPresetFlute)
		COMMAND_ID_HANDLER(IDC_PRE_SYNTH, OnPresetSynth)
		COMMAND_ID_HANDLER(IDC_PRE_PLUCK, OnPresetPlucked)
		COMMAND_ID_HANDLER(IDC_PRE_BASS, OnPresetBass)
		COMMAND_ID_HANDLER(IDC_PRE_RESET, OnPresetReset)
		COMMAND_ID_HANDLER(IDC_PRE_BELL1, OnPresetBell1)
		COMMAND_ID_HANDLER(IDC_PRE_BELL2, OnPresetBell2)
		COMMAND_ID_HANDLER(IDC_PRE_BELL3, OnPresetBell3)
		COMMAND_ID_HANDLER(IDC_PRE_WOOD, OnPresetWood)
		MESSAGE_HANDLER(MM_WOM_DONE, OnWaveDone)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnVScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPlay(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLoop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStack(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStack2(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWye(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDelta(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnKillFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopyClip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnPresetPiano(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetStrings(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBrass(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetClarinet(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetOrgan(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetFlute(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetSynth(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetPlucked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBass(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetReset(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBell1(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBell2(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBell3(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetWood(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnWaveDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void CloseDialog(int nVal);
	//void InitValue(CTrackBarCtrl& slid, CEdit& ed, int minval, int maxval, int ticfrq, int def, double val);
	void InitValue(CTrackBarCtrl& slid, CEdit& ed, int minval, int maxval, int ticfrq);
	void TrackValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv = 0);
	void UpdateValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv = 0);
	void SetPreset(FMInstrParam *p);
	float GetSetting(CEdit& ed);
	void PutSetting(CTrackBarCtrl& slid, CEdit& ed, double val, double div, int inv = 0);
	AmpValue CalcPhaseMod(AmpValue amp, FrqValue mult);
	void InitGen();
	AmpValue Generate();
	void NoteOff();
	int PlayFM(int loop);
	void StopPlaying();
};

