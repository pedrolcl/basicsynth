//////////////////////////////////////////////////////////////////////
/// @file SFPlayer.h BasicSynth SoundFont(R) Player
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_SFPLAYER_H_)
#define _SFPLAYER_H_

#include "LFO.h"
#include <SFDefs.h>
#include <SoundBank.h>
#include <SFGen.h>

/// BasicSynth Instrument to play SoundBank sounds.
///
/// This instrument provides playback of a SoundFont or DLS instrument,
/// specified using a SoundBank alias, bank number and preset number.
/// Bank numbers are 0-128, with bank 128 being the precussion sets. Preset
/// number is 0-127, equivalent to MIDI program change value. 
///
/// The articulation/modulation information from the sound bank is not used.
/// Since the instrument is for use with Notelist, it expects parameter information
/// to be set through SetParam or the SeqEvent object.
///
/// This instrument will respond to real-time frequency changes through the Param()
/// function, changing zones as needed. It does not support portamento, however.
///
class SFPlayerInstr : public InstrumentVP
{
protected:
	int chnl;           // output mixer channel
	int pitch;          // played pitch
	AmpValue vol;       // volume level
	FrqValue frq;       // playback frequency
	GenWaveSF *osc1;    // wavetable oscillator
	GenWaveSF *osc2;    // cross-fade oscillator
	EnvSegLin fadeEG;
	bsInt32 xfade;      // flag indicating we are cross-fading
	EnvGenADSR volEnv;  // volume envelope
	LFO vibLFO;         // vibrato LFO

	bsInt16 monoSet;    // force monophonic even if sample is stereo, turn internal pan off

	GenWaveWT oscfm;    // FM 'boost' oscillator
	EnvGenADSR modEnv;  // envelope for FM
	FrqValue fmCM;      // c:m ratio for 'fm'
	AmpValue fmIM;      // index of mod for 'fm'
	AmpValue fmAmp;     // fm amplitude
	int fmOn;

	bsString sndFile;   // sound bank name (alias or file)
	bsString preName;   // preset name (for reference)

	SoundBank *sndbnk; // run-time sound bank object
	SBInstr *preset;    // run-time preset object
	bsInt16 bnkNum;     // bank number, 0-128, 128=drum kit
	bsInt16 preNum;     // preset number 0-127

	SBZone *zone;      // left channel zone
	SBZone *zone2;     // cross-fade zone

	InstrManager *im;

	int LoadEnv(XmlSynthElem *elem, EnvGenADSR& env);
	int SaveEnv(XmlSynthElem *elem, EnvGenADSR& env);

	void FindPreset();

public:
	static Instrument *SFPlayerInstrFactory(InstrManager *m, Opaque tmplt);
	static SeqEvent *SFPlayerEventFactory(Opaque tmplt);
	static bsInt16 MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	SFPlayerInstr();
	SFPlayerInstr(SFPlayerInstr *tp);
	virtual ~SFPlayerInstr();
	virtual void Copy(SFPlayerInstr *tp);
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	void SetSoundBank(SoundBank *b);
	SoundBank *GetSoundBank()       { return sndbnk; }

	const char *GetSoundFile()        { return sndFile; }
	const char *GetInstrName()       { return preName; }

	void SetSoundFile(const char *str) { sndFile = str; }
	void SetInstrName(const char *str) { preName = str; }

	virtual int Load(XmlSynthElem *parent);
	virtual int Save(XmlSynthElem *parent);

	virtual VarParamEvent *AllocParams();
	virtual int GetParams(VarParamEvent *params);
	virtual int GetParam(bsInt16 id, float* val);
	virtual int SetParams(VarParamEvent *params);
	virtual int SetParam(bsInt16 id, float val);
};
//@}
#endif
