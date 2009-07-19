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
#include <SFSoundBank.h>
#include <SFGen.h>

/// BasicSynth Instrument to play SoundFont files.
///
/// This instrument provides playback of a SoundFont instrument. 
//
/// The SFPlayerInstr plays a SoundFont file sample. The instrument is
/// specified using a SoundFont file alias, bank number and preset number.
/// Bank numbers are 0-128, with bank 128 being the precussion sets. Preset
/// number is 0-127, equivalent to MIDI program change value. This does not
/// fully implement SoundFonts or support MIDI stuff. It is for use with
/// Notelist and expects parameter information to be set via the SeqEvent object.
///
/// The synthesis model is similar to that shown in the SF 2.x specification.
/// Two digital oscillators are used so that stereo samples are reproduced
/// independently. If the left channel is missing, the second oscillator is
/// not used. Two LFO units are available, one for vibrato and one for
/// other modulation effects. The second LFO can be applied to either amplitude
/// or pan position. Two envelope generators are also available. The first
/// is used for overall volume control. The second may be applied as a pitch
/// bend envelope or as a pan position control. Effects levels are not controlled
/// in the instrument (although they could be added if desired) since the
/// BasicSynth mixer is typically set up to control effects for the channel.
/// At present, a filter is not implemented, but could be easily added if needed.
/// (Use FilterIIR2p and apply it to the oscillator outputs.)
///
/// This instrument will respond to real-time frequency changes through the Param()
/// function, but it does not change "zones" when doing so and does not support
/// portamento. Large frequency changes may produce a timbre different from when
/// the sound is retriggered on each frequency change.
///
/// Note that the envelope values from the SoundFont can be applied
/// to this instrument, or envelope can be specified directly via parameter values.
/// This allows overriding the SF2 envelope through Notelist note parameters and thus
/// provides note-by-note articulation control.
class SFPlayerInstr : public InstrumentVP
{
protected:
	int chnl;           // output mixer channel
	int pitch;          // played pitch
	int mono;           // force monophonic output
	AmpValue vol;       // volume level
	FrqValue frq;       // playback frequency
	GenWaveSF oscl;     // left channel oscillator
	GenWaveSF oscr;     // right channel oscillator
	GenWaveSF oscl2;    // cross-fade oscillator right
	GenWaveSF oscr2;    // cross-fade oscillator right
	EnvSegLin fadeEG;
	bsInt32 xfade;
	GenWaveWT oscfm;    // FM 'boost' oscillator
	EnvGenADSR volEnv;  // volume envelope
	EnvGenADSR modEnv;  // modulation envelope (for pitch bend, panning, etc)
	LFO vibLFO;         // vibrato LFO
	LFO modLFO;         // modulation LFO (for tremolo, auto panning, etc)

	EnvDef  volSet;     // if sfEnv is false, use these values
	EnvDef  modSet;     // ""

	bsInt16 sfEnv;      // use file envelope if true
	bsInt16 monoSet;    // force monophonic even if sample is stereo, turn internal pan off
	bsInt16 modFrq;     // bit 0 = apply vibLFO, bit 1 = apply modEnv
	bsInt16 modPan;     // bit 0 = apply modLFO, bit 1 = apply modEnv
	bsInt16 modVol;     // bit 0 = apply modLFO
	FrqValue modFrqScale; // range for pitch bend envelope, in cents
	AmpValue modPanScale; // range for panning, 0-1
	AmpValue modVolScale; // range for tremolo, 0-1

	FrqValue fmCM;      // c:m ratio for 'fm'
	AmpValue fmIM;      // index of mod for 'fm'
	AmpValue fmDf;      // delta f (I*Fm) 

	bsString sndFile;   // sound bank name (alias or file)
	bsString preName;   // preset name (for reference)

	SFSoundBank *sndbnk; // run-time sound bank object
	SFPreset *preset;    // run-time preset object
	bsInt16 bnkNum;     // bank number, 0-128, 128=drum kit
	bsInt16 preNum;     // preset number 0-127

	SFZone *zonel;      // left channel zone
	SFZone *zonel2;     // portamento zone
	SFZone *zoner;      // right channel zone
	SFZone *zoner2;
	Panner panl;        // left channel pan
	Panner panr;        // right channel pan

	InstrManager *im;

	int LoadEnv(XmlSynthElem *elem, EnvDef& env);
	int SaveEnv(XmlSynthElem *elem, EnvDef& env);

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

	void SetSoundBank(SFSoundBank *b);
	SFSoundBank *GetSoundBank()       { return sndbnk; }

	const char *GetSoundFile()        { return sndFile; }
	const char *GetPresetName()       { return preName; }

	void SetSoundFile(const char *str) { sndFile = str; }
	void SetPresetName(const char *str) { preName = str; }

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
