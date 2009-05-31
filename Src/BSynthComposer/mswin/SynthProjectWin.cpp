//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

// This object is set when the keyboard player is active.
// it is accessible from multiple threads.
static Player *thePlayer;

int SynthProject::Generate(int todisk, long from, long to)
{
	mixInfo->InitMixer();
	if (todisk)
		return GenerateToFile(from, to);

	WaveOutDirect wvd;
	mgr.Init(&mix, &wvd);

	nlConverter cvt;
	cvt.SetInstrManager(&mgr);
	cvt.SetSequencer(&seq);
	cvt.SetSampleRate(synthParams.sampleRate);

	if (GenerateSequence(cvt))
		return -1;

	long nbuf = 5;
	if (wvd.Setup(_Module.mainWnd, prjOptions.playBuf, nbuf))
		return -1;

	if (prjGenerate)
		prjGenerate->AddMessage("Start sequencer...");
	// This line generates the output...
	seq.Sequence(mgr, from*synthParams.isampleRate, to*synthParams.isampleRate);

	AmpValue lv, rv;
	long pad = (long) (synthParams.sampleRate * prjOptions.playBuf) * nbuf;
	while (pad-- > 0)
	{
		mix.Out(&lv, &rv);
		wvd.Output2(lv, rv);
	}
	wvd.Shutdown();

	// re-initialize in case there were dynamic mixer control changes
	mixInfo->InitMixer();

	return 0;
}

static int kbdRunning;
static HANDLE genThreadH = INVALID_HANDLE_VALUE;
static DWORD  genThreadID;

static DWORD WINAPI PlayerProc(LPVOID param)
{
	return theProject->Play();
}

int SynthProject::Play()
{
	WaveOutDirect wvd;
	mix.Reset();
	mgr.Init(&mix, &wvd);
	if (wvd.Setup(_Module.mainWnd, 0.02, 4))
		return 0;
	ATLTRACE("Starting live playback...\n");
	thePlayer = new Player;
	thePlayer->Play(mgr);
	ATLTRACE("Stopping live playback...");
	wvd.Stop();
	delete thePlayer;
	thePlayer = 0;
	ATLTRACE("Done\n");
	return 1;
}

int SynthProject::Start()
{
	if (!kbdRunning)
	{
		genThreadH = CreateThread(NULL, 0, PlayerProc, NULL, CREATE_SUSPENDED, &genThreadID);
		if (genThreadH != INVALID_HANDLE_VALUE)
		{
			kbdRunning = 1;
			ResumeThread(genThreadH);
		}
	}
	return kbdRunning;
}

int SynthProject::Stop()
{
	int wasRunning = kbdRunning;
	if (genThreadH != INVALID_HANDLE_VALUE)
	{
		try
		{
			if (thePlayer)
				thePlayer->Halt();
			WaitForSingleObject(genThreadH, 10000);
		}
		catch (...)
		{
		}
		genThreadH = INVALID_HANDLE_VALUE;
	}
	kbdRunning = 0;
	return wasRunning;
}

int SynthProject::PlayEvent(SeqEvent *evt)
{
	if (thePlayer)
	{
		thePlayer->AddEvent(evt);
		return 1;
	}
	delete evt;
	return 0;
}

int SynthProject::IsPlaying()
{
	if (thePlayer)
		return thePlayer->IsPlaying();
	return 0;
}

