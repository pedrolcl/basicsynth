//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

/////////////////////////////////////////////////////////////////////////////
// Platform-specific project functions
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <dsound.h>
#include <WaveOutDirect.h>
static WaveOutDirect *wop = 0;
static HANDLE genThreadH = INVALID_HANDLE_VALUE;
static DWORD  genThreadID;
#define ShortWait() Sleep(0)
#endif
#ifdef UNIX
#include <WaveOutALSA.h>
#include <pthread.h>
static WaveOutALSA *wop = 0;
static pthread_t genThreadID;
#define ShortWait() usleep(1000)
#endif

int SynthProject::Generate(int todisk, long from, long to)
{
	SeqState oldState = seq.GetState();
	if (oldState != seqOff)
		Stop();

	mixInfo->InitMixer();
	if (todisk)
		return GenerateToFile(from, to);
	long nbuf = 4;
	if (prjOptions.playBuf < 0.01)
		prjOptions.playBuf = 0.01;
#ifdef _WIN32
	WaveOutDirect wvd;
	if (wvd.Setup(prjOptions.dsoundHWND, prjOptions.playBuf, nbuf, prjOptions.waveID))
		return -1;
#endif
#ifdef UNIX
	WaveOutALSA wvd;
	if (wvd.Setup(prjOptions.waveDevice, prjOptions.playBuf, nbuf))
		return -1;
#endif
	mgr.Init(&mix, &wvd);

	nlConverter cvt;
	cvt.SetInstrManager(&mgr);
	cvt.SetSequencer(&seq);
	cvt.SetSampleRate(synthParams.sampleRate);

	if (GenerateSequence(cvt))
		return -1;

	if (prjGenerate)
		prjGenerate->AddMessage("Start sequencer...");
	seq.SetCB(SeqCallback, synthParams.isampleRate, (Opaque)this);

	// For now, if MIDI is currently active, allow playing along
	// with the sequence. This might need to be changed to a separate
	// option.
	if (theProject->prjMidiIn.IsOn())
		oldState |= seqPlay;

	// Generate the output...
	wop = &wvd;
	bsInt32 fromSamp = from*synthParams.isampleRate;
	bsInt32 toSamp = to*synthParams.isampleRate;
	if (seq.GetTrackCount() > 1 || oldState & seqPlay)
		seq.SequenceMulti(mgr, fromSamp, toSamp, seqSeqOnce | (oldState & seqPlay));
	else // optimal single track playback
		seq.Sequence(mgr, fromSamp, toSamp);
	seq.SetCB(0, 0, 0);
	wop = 0;

	AmpValue lv, rv;
	long pad;
	if (wvoutInfo)
	{
		pad = (long) (wvoutInfo->GetTailOut() * synthParams.sampleRate);
		while (pad-- > 0)
		{
			mix.Out(&lv, &rv);
			wvd.Output2(lv, rv);
		}
	}

	// Drain all output by filling the output buffer with zeros.
	lv = 0;
	rv = 0;
	pad = (long) (synthParams.sampleRate * prjOptions.playBuf) * nbuf;
	while (pad-- > 0)
		wvd.Output2(lv, rv);

	wvd.Shutdown();

	// re-initialize in case of dynamic mixer control changes
	mixInfo->InitMixer();

	return 0;
}

// Start live playback from keyboard (virtual and/or MIDI).
// This runs as a background thread. (See Start/Stop below)
int SynthProject::Play()
{
#ifdef _WIN32
	WaveOutDirect wvd;
	if (wvd.Setup(prjOptions.dsoundHWND, 0.02, 3, prjOptions.waveID))
		return 0;
#endif
#ifdef UNIX
	WaveOutALSA wvd;
	if (wvd.Setup(prjOptions.waveDevice, 0.02, 3))
		return 0;
#endif
	mix.Reset();
	mgr.Init(&mix, &wvd);
	wop = &wvd;
	// TODO: set a callback to capture the peak output levels.
	seq.SetCB(0, 0, 0);
	seq.Play(mgr);
	wop = 0;
	wvd.Stop();
	return 1;
}

#ifdef _WIN32
static DWORD WINAPI PlayerProc(LPVOID param)
{
	return theProject->Play();
}

int SynthProject::Start()
{
	if (seq.GetState() == seqOff)
	{
		genThreadH = CreateThread(NULL, 0, PlayerProc, NULL, CREATE_SUSPENDED, &genThreadID);
		if (genThreadH != INVALID_HANDLE_VALUE)
		{
			SetThreadPriority(genThreadH, THREAD_PRIORITY_ABOVE_NORMAL);
			ResumeThread(genThreadH);
			return 1;
		}
	}
	return seq.GetState() != seqOff;
}

int SynthProject::Stop()
{
	SeqState wasRunning = seq.GetState();
	if (genThreadH != INVALID_HANDLE_VALUE)
	{
		try
		{
			seq.Halt();
			WaitForSingleObject(genThreadH, 10000);
		}
		catch (...)
		{
		}
		genThreadH = INVALID_HANDLE_VALUE;
	}
	return wasRunning != seqOff;
}
#endif

#ifdef UNIX

static void *PlayerProc(void *param)
{
	theProject->Play();
	return 0;
}

int SynthProject::Start()
{
	return pthread_create(&genThreadID, NULL, PlayerProc, 0);
}

int SynthProject::Stop()
{
	SeqState wasRunning = seq.GetState();
	if (wasRunning)
	{
		seq.Halt();
		pthread_join(genThreadID, NULL); 
	}
	return wasRunning != seqOff;
}
#endif

// Pause sequencer playback. First signal the sequencer
// and then halt the live output buffer. It's important
// to wait for the sequencer to enter the pause
// state before stopping output. Otherwise you can get
// into a deadlock where the sequencer is waiting on the
// output buffer.
int SynthProject::Pause()
{
	if (seq.GetState() != seqPaused)
	{
		seq.Pause();
		while (seq.GetState() != seqPaused)
			ShortWait();
		if (wop)
			wop->Stop();
		return 1;
	}
	return 0;
}

// Resume sequencer playback. First restart the output
// buffer then signal the sequencer to continue.
int SynthProject::Resume()
{
	if (seq.GetState() == seqPaused)
	{
		if (wop)
			wop->Restart();
		seq.Resume();
		return 1;
	}
	return 0;
}
