//////////////////////////////////////////////////////////////////////
/// @file GMSynthDLL.cpp General MIDI Synthesizer
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef UNIX
typedef struct _GUID 
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;
#endif

static const GUID GMSYNTH_MAGIC  = 
{ 0x2d257162, 0x432b, 0x438d, { 0xbd, 0x5e, 0x86, 0x9a, 0xbb, 0x7, 0x3a, 0x3c } };

/// Structure to hold global information.
/// This is returned as the HANDLE on GMSynthInit
class GMSynthDLL
{
private:
	MIDIControl mc;
	GMInstrManager inmgr;
	SequencerCB seq;
	SeqState seqMode;
	WaveFile wvf;
	SoundBank *sbnk;
	SMFFile midFile;
	MIDIInput kbd;
	GMSYNTHCB usrCB;
	Opaque    usrArg;

#ifdef _WIN32
	WaveOutDirect wvd;
	HANDLE genThreadH;
	DWORD  genThreadID;
#endif

#ifdef UNIX
	WaveOutALSA wvd;
	pthread_t genThreadID;
#endif

	int StartSequencer();
	int StopSequencer();
	static void eventCB(bsUint32 tick, bsInt16 evtID, const SeqEvent *evt, Opaque usr);
	static void tickCB(bsInt32 cnt, Opaque arg);

public:
	GUID magic;

	GMSynthDLL(HANDLE w)
	{
		magic = GMSYNTH_MAGIC;
		sbnk = 0;
		seqMode = seqPlay;
		seq.SetController(&mc);
		inmgr.SetController(&mc);
		genThreadID = 0;
		kbd.SetSequenceInfo(&seq, &inmgr);

#ifdef _WIN32
		genThreadH = INVALID_HANDLE_VALUE;
		wvd.Setup((HWND) w, 0.02, 4, NULL);
#endif
#ifdef UNIX
		wvd.Setup((char*)w, 0.02, 4);
#endif
	}

	~GMSynthDLL()
	{
		wvd.Shutdown();
		memset(&magic, 0, sizeof(GUID));
	}

	void SetCallback(GMSYNTHCB cb, bsInt32 cbRate, Opaque arg)
	{
		usrArg = arg;
		usrCB = cb;
		if (cb != 0)
		{
			if (cbRate > 0)
				seq.SetCB(tickCB, (cbRate * synthParams.isampleRate) / 1000, this);
			else
				seq.SetCB(0, 0, 0);
			seq.SetEventCB(eventCB, this);
		}
	}

	void OnTick(bsInt32 cnt);
	void OnEvent(bsInt16 evtid, const SeqEvent *evt);

	void Sequence(bsInt32 st, bsInt32 end)
	{
		seq.SequenceMulti(inmgr, st, end, seqMode);
	}

	int LoadSoundBank(const char *alias, const char *fileName, int preload);
	int GetMetaText(int id, char *txt, size_t len);
	int GetMetaData(int id, long *vals);
	int LoadSequence(const char *fileName, const char *sbnkName);
	int Start(int mode);
	int Stop();
	int Pause();
	int Resume();
	int Generate(const char *fileName);
	int MidiIn(int onoff, int device);
	void ImmediateEvent(short mmsg, short val1, short val2);
};

/////////////////////////////////////////////////////////////////////
// C++ Wrapper 
////////////////////////////////////////////////////////////////////

GMSynth::GMSynth(HANDLE w)
{
	synth = (void*) new GMSynthDLL(w);
}

GMSynth::~GMSynth()
{
	delete (GMSynthDLL*)synth;
}

void GMSynth::SetCallback(GMSYNTHCB cb, bsInt32 cbRate, Opaque arg)
{
	((GMSynthDLL*)synth)->SetCallback(cb, cbRate, arg);
}

int GMSynth::LoadSoundBank(const char *alias, const char *fileName, int preload)
{
	return ((GMSynthDLL*)synth)->LoadSoundBank(alias, fileName, preload);
}

int GMSynth::GetMetaText(int id, char *txt, size_t len)
{
	return ((GMSynthDLL*)synth)->GetMetaText(id, txt, len);
}

int GMSynth::GetMetaData(int id, long *vals)
{
	return ((GMSynthDLL*)synth)->GetMetaData(id, vals);
}

int GMSynth::LoadSequence(const char *fileName, const char *sbnkName)
{
	return ((GMSynthDLL*)synth)->LoadSequence(fileName, sbnkName);
}

int GMSynth::Start(int mode)
{
	return ((GMSynthDLL*)synth)->Start(mode);
}

int GMSynth::Stop()
{
	return ((GMSynthDLL*)synth)->Stop();
}

int GMSynth::Pause()
{
	return ((GMSynthDLL*)synth)->Pause();
}
int GMSynth::Resume()
{
	return ((GMSynthDLL*)synth)->Resume();
}

int GMSynth::Generate(const char *fileName)
{
	return ((GMSynthDLL*)synth)->Generate(fileName);
}

int GMSynth::MidiIn(int onoff, int device)
{
	return ((GMSynthDLL*)synth)->MidiIn(onoff, device);
}

void GMSynth::ImmediateEvent(short mmsg, short val1, short val2)
{
	return ((GMSynthDLL*)synth)->ImmediateEvent(mmsg, val1, val2);
}


/////////////////////////////////////////////////////////////////////////
// Function call API implementation
/////////////////////////////////////////////////////////////////////////

extern "C" {

static GMSynthDLL *CheckHandle(HANDLE gm)
{
	if (gm == 0)
		return 0;

	GMSynthDLL *synth = (GMSynthDLL *)gm;
	if (!memcmp(&synth->magic, &GMSYNTH_MAGIC, sizeof(GUID)))
		return 0;
	return synth;
}

HANDLE EXPORT GMSynthOpen(HANDLE w, bsInt32 sr)
{
	if (sr == 0)
		sr = 44100;
	InitSynthesizer(sr);

	GMSynthDLL *synth = new GMSynthDLL(w);
	return (HANDLE) synth;
}


int EXPORT GMSynthClose(HANDLE gm)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	delete synth;
	return GMSYNTH_NOERROR;
}

int EXPORT GMSynthLoadSoundBank(HANDLE gm, const char *fileName, int preload, const char *alias)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	return synth->LoadSoundBank(alias, fileName, preload);
}

int EXPORT GMSynthLoadSequence(HANDLE gm, const char *fileName, const char *sbnkName)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	return synth->LoadSequence(fileName, sbnkName);
}

int EXPORT GMSynthMetaText(HANDLE gm, int id, char *txt, size_t len)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	return synth->GetMetaText(id, txt, len);
}

int EXPORT GMSynthMetaData(HANDLE gm, int id, long *vals)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	return synth->GetMetaData(id, vals);
}

int EXPORT GMSynthStart(HANDLE gm, int mode)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	return synth->Start(mode);
}

int EXPORT GMSynthStop(HANDLE gm)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	return synth->Stop();
}

int EXPORT GMSynthPause(HANDLE gm)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	return synth->Pause();
}

int EXPORT GMSynthResume(HANDLE gm)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;
	
	return synth->Resume();
}

int EXPORT GMSynthGenerate(HANDLE gm, const char *fileName)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	return synth->Generate(fileName);
}

int EXPORT GMSynthSetCallback(HANDLE gm, GMSYNTHCB cb, bsUint32 cbRate, Opaque arg)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;

	synth->SetCallback(cb, cbRate, arg);
	return 0;
}

int EXPORT GMSynthMIDIKbdIn(HANDLE gm, int onoff, int device)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;
	return synth->MidiIn(onoff, device);
}

int EXPORT GMSynthMIDIEvent(HANDLE gm, short mmsg, short val1, short val2)
{
	GMSynthDLL *synth = CheckHandle(gm);
	if (synth == 0)
		return GMSYNTH_ERR_BADHANDLE;
	synth->ImmediateEvent(mmsg, val1, val2);
	return 0;
}

// TODO:
// Add to sequence
// Enumerate sequence
// Start/stop track

// end extern "C"
} 

/////////////////////////////////////////////////////////////////////////
// Component implementation
/////////////////////////////////////////////////////////////////////////

void GMSynthDLL::tickCB(bsInt32 cnt, Opaque arg)
{
	((GMSynthDLL*)arg)->OnTick(cnt);
}

void GMSynthDLL::eventCB(bsUint32 tick, bsInt16 evtID, const SeqEvent *evt, Opaque usr)
{
	((GMSynthDLL*)usr)->OnEvent(evtID, evt);
}

void GMSynthDLL::OnTick(bsInt32 cnt)
{
	if (usrCB)
		usrCB(GMSYNTH_EVENT_TICK, cnt, usrArg);
}

void GMSynthDLL::OnEvent(bsInt16 evtid, const SeqEvent *evt)
{
	if (usrCB)
	{
		NoteEvent *nevt;
		ControlEvent *cevt;
		TrackEvent *tevt;
		bsInt16 uevt;
		bsInt32 val = 0;
		switch (evtid)
		{
		case SEQEVT_SEQSTART:
			uevt = GMSYNTH_EVENT_START;
			break;
		case SEQEVT_SEQSTOP:
			uevt = GMSYNTH_EVENT_STOP;
			break;
		case SEQEVT_SEQPAUSE:
			uevt = GMSYNTH_EVENT_PAUSE;
			break;
		case SEQEVT_SEQRESUME:
			uevt = GMSYNTH_EVENT_RESUME;
			break;
		case SEQEVT_START:
			nevt = (NoteEvent *)evt;
			uevt = GMSYNTH_EVENT_NOTEON;
			val = 0x900000 | (nevt->chnl << 16) | ((nevt->pitch+12)<<8) | nevt->noteonvel;
			break;
		case SEQEVT_STOP:
			nevt = (NoteEvent *)evt;
			uevt = GMSYNTH_EVENT_NOTEOFF;
			val = 0x800000 | (nevt->chnl << 16) | ((nevt->pitch+12)<<8);
			break;
		case SEQEVT_STARTTRACK:
			tevt = (TrackEvent *)evt;
			uevt = GMSYNTH_EVENT_TRKON;
			val = tevt->trkNo;
			break;
		case SEQEVT_STOPTRACK:
			tevt = (TrackEvent *)evt;
			uevt = GMSYNTH_EVENT_TRKOFF;
			val = tevt->trkNo;
			break;
		case SEQEVT_CONTROL:
			uevt = GMSYNTH_EVENT_CTLCHG;
			cevt = (ControlEvent *)evt;
			val = (cevt->mmsg | cevt->chnl) << 16;
			if (cevt->mmsg == MIDI_CTLCHG)
				val |= (cevt->ctrl << 8) | cevt->cval;
			else if (cevt->mmsg == MIDI_PWCHG)
				val |= ((cevt->cval << 1) & 0xff) | (cevt->cval & 0x7f);
			else
				val |= cevt->cval;
			break;
		case SEQEVT_PARAM:
		case SEQEVT_RESTART:
			return;
		}
		usrCB(uevt, val, usrArg);
	}
}

int GMSynthDLL::LoadSoundBank(const char *alias, const char *fileName, int preload)
{
	SoundBank *sb = 0;
	if (SFFile::IsSF2File(fileName))
	{
		SFFile file;
		sb = file.LoadSoundBank(fileName, preload);
	}
	else if (DLSFile::IsDLSFile(fileName))
	{
		DLSFile file;
		sb = file.LoadSoundBank(fileName, preload);
	}
	else
		return GMSYNTH_ERR_FILETYPE;

	if (sb == NULL)
		return GMSYNTH_ERR_FILEOPEN;

	sb->Lock();
	if (alias)
		sb->name = alias;
	SoundBank::SoundBankList.Insert(sb);
	sbnk = sb;
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::LoadSequence(const char *fileName, const char *sbnkName)
{
	Stop();

	if (sbnkName && *sbnkName)
	{
		SoundBank *sb = SoundBank::FindBank(sbnkName);
		if (sb == 0)
			return GMSYNTH_ERR_BADID;
		sbnk = sb;
		inmgr.SetSoundBank(sb);
	}
	if (sbnk == 0)
		return GMSYNTH_ERR_BADID;

	midFile.Reset();
	if (midFile.LoadFile(fileName))
		return GMSYNTH_ERR_FILEOPEN;

	InstrConfig *inc = inmgr.FindInstr((bsInt16)0);
	SMFInstrMap map[16];
	for (int i = 0; i < 16; i++)
	{
		map[i].inc = inc;
		map[i].bnkParam = -1;
		map[i].preParam = -1;
	}

	if (midFile.GenerateSeq(&seq, map, sbnk))
		return GMSYNTH_ERR_GENERATE;

	return GMSYNTH_NOERROR;
}

int GMSynthDLL::Start(int mode)
{
	switch (mode)
	{
	case GMSYNTH_MODE_PLAY:
		seqMode = seqPlay;
		break;
	case GMSYNTH_MODE_SEQUENCE:
		seqMode = seqPlaySeqOnce;
		break;
	case GMSYNTH_MODE_SEQPLAY:
		seqMode = seqPlaySeq;
		break;
	default:
		return GMSYNTH_ERR_BADID;
	}

	Stop();
	inmgr.SetWaveOut(&wvd);
	wvd.Restart();
	return StartSequencer();
}

int GMSynthDLL::Stop()
{
	SeqState wasRunning = seq.GetState();
	if (wasRunning != seqOff)
		seq.Halt();
	StopSequencer();
	if (seqMode == seqSeqOnce)
		wvf.CloseWaveFile();
	else if (seqMode & seqPlay)
		wvd.Stop();
	seqMode = seqOff;
	return wasRunning != seqOff;
}

int GMSynthDLL::Pause()
{
	SeqState st = seq.GetState();
	if (st == seqOff)
		return GMSYNTH_ERR_BADID;

	if (st != seqPaused)
	{
		seq.Pause();
		// Don't stop the wave output until the sequencer is paused
		// or the sequencer can get stuck attempting to write
		// to the DirectSound/ALSA buffer!
		while (seq.GetState() != seqPaused)
			Sleep(0);
		if (seqMode & seqPlay)
			wvd.Stop();
	}
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::Resume()
{
	SeqState st = seq.GetState();
	if (st == seqOff)
		return GMSYNTH_ERR_BADID;
	if (st == seqPaused)
	{
		if (seqMode & seqPlay)
			wvd.Restart();
		seq.Resume();
	}
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::Generate(const char *fileName)
{
	Stop();

	if (wvf.OpenWaveFile(fileName, 2))
		return GMSYNTH_ERR_FILEOPEN;

	inmgr.SetWaveOut(&wvf);
	seqMode = seqSeqOnce;
	return StartSequencer();
}

int GMSynthDLL::GetMetaText(int id, char *txt, size_t len)
{
	*txt = '\0';

	switch (id)
	{
	case GMSYNTH_META_SEQTEXT:
		strncpy(txt, midFile.MetaText(), len);
		break;
	case GMSYNTH_META_SEQCPYR:
		strncpy(txt, midFile.Copyright(), len);
		break;
	case GMSYNTH_META_SEQNAME:
		strncpy(txt, midFile.SeqName(), len);
		break;
	case GMSYNTH_META_TIMESIG:
		strncpy(txt, midFile.TimeSignature(), len);
		break;
	case GMSYNTH_META_KEYSIG:
		strncpy(txt, midFile.KeySignature(), len);
		break;
	case GMSYNTH_META_SBKNAME:
		if (sbnk)
			strncpy(txt, sbnk->info.szName, len);
		break;
	case GMSYNTH_META_SBKCPYR:
		if (sbnk)
			strncpy(txt, sbnk->info.szCopyright, len);
		break;
	case GMSYNTH_META_SBKCMNT:
		if (sbnk)
			strncpy(txt, sbnk->info.szComment, len);
		break;
	case GMSYNTH_META_SBKVER:
		if (sbnk)
			snprintf(txt, len, "%d.%d.%d.%d",
				sbnk->info.wMajorFile, sbnk->info.wMinorFile, 
				sbnk->info.wMajorVer, sbnk->info.wMinorVer);
		break;
	default:
		return GMSYNTH_ERR_BADID;
	}
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::GetMetaData(int id, long *vals)
{
	switch (id)
	{
	case GMSYNTH_META_TIMESIG:
		vals[0] = (long) midFile.timeSigNum;
		vals[1] = (long) midFile.timeSigDiv;
		vals[2] = (long) midFile.timeSigBeat;
		break;
	case GMSYNTH_META_KEYSIG:
		vals[0] = (long) midFile.keySigKey;
		vals[1] = (long) midFile.keySigMaj;
		break;
	case GMSYNTH_META_SBKVER:
		if (sbnk)
		{
			vals[0] = (long) sbnk->info.wMajorFile;
			vals[1] = (long) sbnk->info.wMinorFile;
			vals[2] = (long) sbnk->info.wMajorVer;
			vals[3] = (long) sbnk->info.wMinorVer;
		}
		break;
	default:
		return GMSYNTH_ERR_BADID;
	}
	return GMSYNTH_NOERROR;
}

void GMSynthDLL::ImmediateEvent(short mmsg, short val1, short val2)
{
	kbd.MIDIInput::ReceiveMessage(mmsg, val1, val2, 0);
}

int GMSynthDLL::MidiIn(int onoff, int device)
{
	if (onoff)
	{
		kbd.SetDevice(device, 0);
		kbd.Start();
	}
	else
		kbd.Stop();
	return 0;
}

#ifdef _WIN32

static DWORD WINAPI PlayerProc(LPVOID param)
{
	GMSynthDLL *synth = (GMSynthDLL *) param;
	synth->Sequence(0, 0);
	return 0;
}

int GMSynthDLL::StartSequencer()
{
	genThreadH = CreateThread(NULL, 0, PlayerProc, this, CREATE_SUSPENDED, &genThreadID);
	if (genThreadH != INVALID_HANDLE_VALUE)
	{
		SetThreadPriority(genThreadH, THREAD_PRIORITY_ABOVE_NORMAL);
		ResumeThread(genThreadH);
		return 1;
	}
	return -2;
}

int GMSynthDLL::StopSequencer()
{
	try
	{
		WaitForSingleObject(genThreadH, 10000);
	}
	catch(...)
	{
	}
	genThreadH = INVALID_HANDLE_VALUE;
	return 0;
}

#endif

#ifdef UNIX

// use a mutex to sync threads. 
// could possibly use pthread_barrier_* ?
static pthread_mutex_t genDlgGuard;

static void *PlayerProc(void *param)
{
	// synchronize with the main thread...
	pthread_mutex_lock(&genDlgGuard);
	pthread_mutex_unlock(&genDlgGuard);
	GMSynthDLL *synth = (GMSynthDLL *) param;
	synth->Sequence(0, 0);
	pthread_exit((void*)0);
	return 0;
}

int GMSynthDLL::StartSequencer()
{
	pthread_mutex_init(&genDlgGuard, NULL);
	pthread_mutex_lock(&genDlgGuard);
	int err = pthread_create(&genThreadID, NULL, PlayerProc, this);
	pthread_mutex_unlock(&genDlgGuard);
	return err;
}

int GMSynthDLL::StopSequencer()
{
	try
	{
		pthread_join(genThreadID, NULL); 
		pthread_mutex_destroy(&genDlgGuard);
	}
	catch (...)
	{
	}
	genThreadH = 0;
	return 0;
}
#endif
