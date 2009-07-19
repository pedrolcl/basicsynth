//////////////////////////////////////////////////////////////////////
/// @file GMPlayer.h BasicSynth General MIDI Player
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_GMPLAYER_H_)
#define _GMPLAYER_H_

#include "LFO.h"
#include <SFDefs.h>
#include <SFSoundBank.h>
#include <SFGen.h>
#include <MIDIDefs.h>
#include <MIDIControl.h>

class GMManager;

class GMPlayer : 
	public Instrument, 
	public SynthList<GMPlayer>
{
private:
	bsInt16 chnl;       // MIDI channel
	bsInt16 mkey;       // MIDI key number
	bsInt16 novel;      // note-on velocity
	bsInt16 localPan;   // use zone pan information
	bsInt16 sostenuto;  // true when sostenuto was on before note start
	FrqValue frq;       // playback frequency
	AmpValue vol;       // playback volume level
	GenWaveSF oscl;     // left channel oscillator
	GenWaveSF oscr;     // right channel oscillator
	EnvGenSegSus volEnv;  // volume envelope
	LFO vibrato;        // a/k/a LF pitch variation
	SFPreset *preset;   // instrument patch
	SFZone *zonel;      // left channel zone
	SFZone *zoner;      // right channel zone
	Panner panr;
	Panner panl;
	InstrManager *im;
	GMManager *gm;
	SFSoundBank *sndbnk;
	MIDIControl *midiCtl;
public:
	GMPlayer();
	GMPlayer(GMManager *g, InstrManager *m);
	virtual ~GMPlayer();
	void Reset();

	void SetSoundBank(SFSoundBank *s) { sndbnk = s; }
	void SetLocalPan(bsInt16 n) { localPan = n; }
	void SetMidiControl(MIDIControl *mc) { midiCtl = mc; }

	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();
};

/// BasicSynth Instrument to play GM SoundFont files.
///
/// This class emulates a MIDI keyboard instrument. It manages a 
/// set of instruments, each containing an oscillator/envelope combination,
/// that share the same global patch and modulation oscillators for
/// vibrato and tremolo. The aggregate instrument maintains a reference
/// to a MIDIControl object and a SFSoundBank object that is passed to
/// the instruments when they are instantiated. One instance of this
/// class is created as the template object for the instrument type.
/// The one instance manages all 16 MIDI channels and instantiates
/// GMPlayer objects as needed. This instrument does not generate
/// sound directly.
class GMManager : public InstrumentVP
{
protected:
	MIDIControl *midiCtl;
	SFSoundBank *sndbnk;
	InstrManager *im;
	GMPlayer instrHead;
	GMPlayer instrTail;
	bsInt16 localPan;
	bsString sndFile;

public:
	static Instrument *InstrFactory(InstrManager *m, Opaque tmplt);
	static SeqEvent *EventFactory(Opaque tmplt);
	static Opaque TmpltFactory(XmlSynthElem *tmplt);
	static void TmpltDump(Opaque tmplt);
	static bsInt16 MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	GMManager();
	virtual ~GMManager();

	MIDIControl *GetMidiControl()
	{
		return midiCtl;
	}

	inline SFSoundBank *GetSoundBank()
	{ 
		return sndbnk; 
	}

	inline const char *GetSoundFile()
	{ 
		return sndFile; 
	}

	inline bsInt16 GetLocalPan()
	{ 
		return localPan; 
	}

	void SetMidiControl(MIDIControl *mc);
	void SetSoundBank(SFSoundBank *b);
	void SetSoundFile(const char *b);
	void SetLocalPan(bsInt16 lp);
	
	// Convenience functions

	inline FrqValue GetPitchbend(int chnl)
	{
		return midiCtl->GetPitchbend(chnl);
	}

	inline FrqValue GetModwheel(int chnl)
	{
		return midiCtl->GetModwheel(chnl);
	}

	inline AmpValue GetVolume(int chnl)
	{
		return midiCtl->GetVolume(chnl);
	}

	inline AmpValue GetAftertouch(int chnl)
	{
		return midiCtl->GetAftertouch(chnl);
	}

	inline bsInt16 GetPatch(int chnl)
	{
		return midiCtl->GetPatch(chnl);
	}

	inline bsInt16 GetBank(int chnl)
	{
		return midiCtl->GetBank(chnl);
	}

	inline bsInt16 GetCC(int chnl, int ccn)
	{
		return midiCtl->GetCC(chnl, ccn);
	}

	// InstrumentVP functions
	GMPlayer *AllocPlayer(InstrManager *m);

	virtual VarParamEvent *AllocParams();
	virtual int GetParams(VarParamEvent *params);
	virtual int GetParam(bsInt16 id, float* val);
	virtual int SetParams(VarParamEvent *params);
	virtual int SetParam(bsInt16 id, float val);

	virtual int Load(XmlSynthElem *parent);
	virtual int Save(XmlSynthElem *parent);
};
//@}
#endif
