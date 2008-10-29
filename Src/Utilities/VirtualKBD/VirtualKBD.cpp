/////////////////////////////////////////////////////////////////////////////
// BasicSynth - Virtual Keyboard instrument player, program entry
//
// The virutal keyboard provides an interactive on-screen keyboard you play
// with the mouse. The synthesis instrument is loaded from either a BSynth
// project file or an instrument library file.
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

#include "aboutdlg.h"
#include "Project.h"
#include "KbdWindow.h"

CAppModule _Module;
InstrManager instrMgr;
Mixer theMixer;
WaveOutDirect waveOut;
Player thePlayer;

#include "MainDlg.h"

void DestroyTemplate(Opaque tp)
{
	Instrument *ip = (Instrument *)tp;
	delete ip;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);

	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	instrMgr.AddType("Tone", ToneInstr::ToneFactory, ToneInstr::ToneEventFactory);
	instrMgr.AddType("ToneFM", ToneFM::ToneFMFactory, ToneFM::ToneFMEventFactory);
	instrMgr.AddType("AddSynth", AddSynth::AddSynthFactory, AddSynth::AddSynthEventFactory);
	instrMgr.AddType("SubSynth", SubSynth::SubSynthFactory, SubSynth::SubSynthEventFactory);
	instrMgr.AddType("FMSynth", FMSynth::FMSynthFactory, FMSynth::FMSynthEventFactory);
	instrMgr.AddType("MatrixSynth", MatrixSynth::MatrixSynthFactory, MatrixSynth::MatrixSynthEventFactory);
	instrMgr.AddType("WFSynth", WFSynth::WFSynthFactory, WFSynth::WFSynthEventFactory);
	InstrMapEntry *im = 0;
	while ((im = instrMgr.EnumType(im)) != 0)
		im->dumpTmplt = DestroyTemplate;

	theMixer.MasterVolume(1.0, 1.0);
	theMixer.SetChannels(1);
	theMixer.ChannelOn(0, 1);
	theMixer.ChannelVolume(0, 0.7);
	theMixer.ChannelPan(0, panOff, 0);
	instrMgr.Init(&theMixer, &waveOut);

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	GdiplusShutdown(gdiplusToken);
	::CoUninitialize();

	return nRet;
}
