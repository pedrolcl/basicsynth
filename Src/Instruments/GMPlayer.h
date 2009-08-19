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
#include <SoundBank.h>
#include <SFGen.h>

class GMManager;

class GMPlayer : 
	public Instrument, 
	public SynthList<GMPlayer>
{
private:
	bsInt16 chnl;       ///< MIDI channel
	bsInt16 mkey;       ///< MIDI key number
	bsInt16 novel;      ///< note-on velocity
	bsInt16 sostenuto;  ///< true when sostenuto was on before note start
	bsInt16 sustainOn;  ///< true when sustain was on at release
	bsInt32 vibDelay;   ///< delay before vibrato begins to affect output
	bsInt32 modDelay;   ///< delay before modulator begins to affect output
	FrqValue frq;       ///< playback frequency
	AmpValue vol;       ///< playback volume level (centibels)
	GenWaveSF *osc;      ///< wavetable oscillator
	EnvGenSF  volEnv;   ///< volume envelope (EG1)
	EnvGenSF  modEnv;   ///< modulation envelope (EG2)
	GenWaveWT viblfo;   ///< LF pitch variation
	GenWaveWT modlfo;   ///< LF amplitude variation
	SBInstr *preset;    ///< instrument patch
	SBZone *zone;       ///< zone for current pitch
	Panner panr;
	Panner panl;
	InstrManager *im;
	GMManager *gm;
	SoundBank *sndbnk;
	MIDIControl *midiCtrl;
	bsInt32 genFlags;
public:
	GMPlayer();
	GMPlayer(GMManager *g, InstrManager *m);
	virtual ~GMPlayer();
	void Reset();

	void SetSoundBank(SoundBank *s) { sndbnk = s; }
	void SetMidiControl(MIDIControl *mc) { midiCtrl = mc; }

	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();
};

#define GMM_LOCAL_VOL   0x01
#define GMM_LOCAL_PATCH 0x02
#define GMM_LOCAL_PAN   0x04

/// BasicSynth Instrument to play GM SoundFont files.
///
/// This class emulates a MIDI keyboard instrument. It manages a 
/// set of instruments, each containing an oscillator/envelope combination,
/// that share the same global patch and modulation oscillators for
/// vibrato and tremolo. The aggregate instrument maintains a reference
/// to a MIDIControl object and a SoundBank object that is passed to
/// the instruments when they are instantiated. One instance of this
/// class is created as the template object for the instrument type.
/// The one instance manages all 16 MIDI channels and instantiates
/// GMPlayer objects as needed. This instrument does not generate
/// sound directly.
class GMManager : public InstrumentVP
{
protected:
	SoundBank *sndbnk;
	InstrManager *im;
	GMPlayer instrHead;
	GMPlayer instrTail;
	bsInt32 localVals;
	AmpValue volValue;
	bsInt16 bankValue;
	bsInt16 progValue;
	bsString sndFile;

public:
	static Instrument *InstrFactory(InstrManager *m, Opaque tmplt);
	static SeqEvent *EventFactory(Opaque tmplt);
	static Opaque TmpltFactory(XmlSynthElem *tmplt);
	static void TmpltDump(Opaque tmplt);
	static bsInt16 MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);
	static MIDIControl *midiCtrl;

	GMManager();
	virtual ~GMManager();

	MIDIControl *GetMidiControl()
	{
		return midiCtrl;
	}

	inline SoundBank *GetSoundBank()
	{ 
		return sndbnk; 
	}

	inline const char *GetSoundFile()
	{ 
		return sndFile; 
	}

	inline bsInt16 GetLocalPan()
	{ 
		return (localVals & GMM_LOCAL_PAN) ? 1 : 0; 
	}

	void SetMidiControl(MIDIControl *mc);
	void SetSoundBank(SoundBank *b);
	void SetSoundFile(const char *b);
	void SetLocalPan(bsInt16 lp);
	
	// Convenience functions

	inline FrqValue GetPitchbend(int chnl)
	{
		return midiCtrl->GetPitchbend(chnl);
	}

	inline FrqValue GetModwheel(int chnl)
	{
		return midiCtrl->GetModwheel(chnl);
	}

	inline AmpValue GetVolume(int chnl)
	{
		if (localVals & GMM_LOCAL_VOL)
			return volValue;
		return midiCtrl->GetVolume(chnl);
	}

	inline AmpValue GetAftertouch(int chnl)
	{
		return midiCtrl->GetAftertouch(chnl);
	}

	inline AmpValue GetPan(int chnl)
	{
		return midiCtrl->GetPan(chnl);
	}

	inline bsInt16 GetPatch(int chnl)
	{
		if (localVals & GMM_LOCAL_PATCH)
			return progValue;
		return midiCtrl->GetPatch(chnl);
	}

	inline bsInt16 GetBank(int chnl)
	{
		if (localVals & GMM_LOCAL_PATCH)
			return bankValue;
		return midiCtrl->GetBank(chnl);
	}

	inline bsInt16 GetCC(int chnl, int ccn)
	{
		return midiCtrl->GetCC(chnl, ccn);
	}

	inline void SetCC(int chnl, bsInt16 ccn, bsInt16 val)
	{
		midiCtrl->SetCC(chnl, ccn, val);
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
