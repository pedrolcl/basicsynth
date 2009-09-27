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

#define GMGEN_DEF (SBGEN_PITWHLF|SBGEN_VIBLFOF|SBGEN_MODWHLF|SBGEN_MODWHLA)

/// @brief GM sound player
/// @detail The GM player implements playback of a SoundBank sample.
/// GMPlayer objects are created and managed by the GMManager object,
/// and should not be created directly.
///
/// Each player object recieves a MIDI bank and program number along
/// with a Soundbank object. The associated program (a/k/a patch) holds
/// a list of zones (regions) each covering a range of pitches. The
/// player locates the appropriate zone(s) using the pitch and then 
/// initializes an oscillator, envelope and suitable modulators
/// based on the information in the zone. Control of volume, panning,
/// pitch bend and modulation level is done indirectly via the MIDIControl
/// object associated with the GMManager.
///
/// To optimize playback, a set of flags controls which unit generators
/// are executed. The flags are set when the SBZone object is loaded.
///
/// SoundFont (SF2) files will sometimes have multiple zones per note.
/// This is typically done to implement a stereo sample. GMPlayer uses
/// a list of zones to handle this situation.
class GMPlayer : 
	public Instrument, 
	public SynthList<GMPlayer>
{
private:
	class GMPlayerZone : public SynthList<GMPlayerZone>
	{
	public:
		SBZone *zone;       ///< zone for this key
		GenWaveWTLoop osc;  ///< wavetable oscillator
		EnvGenSF  volEnv;   ///< volume envelope (EG1)
		EnvGenSF  modEnv;   ///< modulation envelope (EG2)
		GenWaveWT viblfo;   ///< LF pitch variation
		GenWaveWT modlfo;   ///< LF amplitude variation
		FilterIIR2p filt;
		Panner pan;
		bsInt32 genFlags;   ///< map of generators that are operational
		bsInt32 vibDelay;   ///< delay before vibrato begins to affect output
		bsInt32 modDelay;   ///< delay before modulator begins to affect output
		AmpValue vol;

		GMPlayerZone(SBZone *z)
		{
			zone = z;
		}
	};

	GMPlayerZone *zoneList;

	bsInt16 chnl;       ///< MIDI channel
	bsInt16 mkey;       ///< MIDI key number
	bsInt16 novel;      ///< note-on velocity
	bsInt16 exclNote;   ///< exclusive note
	bsInt16 sostenuto;  ///< true when sostenuto was on before note start
	bsInt16 sustainOn;  ///< true when sustain was on at release
	FrqValue frq;       ///< playback frequency
	bsInt32 allFlags;   ///< combined map of generators that are operational
	SBInstr *instr;     ///< instrument patch
	InstrManager *im;
	GMManager *gm;
	SoundBank *sndbnk;
	MIDIControl *midiCtrl;
	void ClearZones();
public:
	GMPlayer();
	GMPlayer(GMManager *g, InstrManager *m);
	virtual ~GMPlayer();
	void Reset();

	void SetSoundBank(SoundBank *s);
	void SetMidiControl(MIDIControl *mc) { midiCtrl = mc; }

	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();
	virtual void Cancel();
	virtual int CheckExculsive(GMPlayer *other);
};

#define GMM_LOCAL_VOL   0x01
#define GMM_LOCAL_PATCH 0x02
#define GMM_LOCAL_PAN   0x04

/// BasicSynth Instrument to play GM SoundFont files.
///
/// This class emulates a MIDI keyboard instrument. It manages a 
/// set of instruments, each containing an oscillator, envelope, modulator
/// combination. The aggregate instrument maintains a reference
/// to a MIDIControl object and a SoundBank object that is passed to
/// the instrument objects when they are instantiated. One instance of this
/// class is created as the template object for the instrument type.
/// The one instance manages all 16 MIDI channels and allocates
/// GMPlayer objects as needed. This instrument does not generate
/// sound directly.
///
/// Various convenience functions are provided that access the channel
/// status information.
class GMManager : public InstrumentVP
{
protected:
	SoundBank *sndbnk;   ///< reference to the sample collection
	InstrManager *im;    ///< instrument manager (used for output)
	SynthEnumList<GMPlayer> instrList; ///< List of allocated GMPlayer objects
	bsInt32 localVals;   ///< Local vals, when set, overrides volume and patch number
	AmpValue volValue;   ///< Local volume level
	AmpValue panValue;   ///< Local pan level
	bsInt16 bankValue;   ///< Local bank override
	bsInt16 progValue;   ///< Local program override
	bsString sndFile;    ///< sound file name
	bsUint32 genFlags;   ///< global generator enable flags

public:
	/// @copydoc InstrFactory
	static Instrument *InstrFactory(InstrManager *m, Opaque tmplt);
	/// @copydoc EventFactory
	static SeqEvent *EventFactory(Opaque tmplt);
	static Opaque TmpltFactory(XmlSynthElem *tmplt);
	static void TmpltDump(Opaque tmplt);
	/// @copydoc ParamID
	static bsInt16 MapParamID(const char *name, Opaque tmplt);
	/// @copydoc ParamName
	static const char *MapParamName(bsInt16 id, Opaque tmplt);
	static MIDIControl *midiCtrl; ///< global MIDI channel information

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

	inline bsUint32 GetGenFlags()
	{
		return genFlags;
	}

	inline bsInt16 GetLocalPan()
	{ 
		return (localVals & GMM_LOCAL_PAN) ? 1 : 0; 
	}

	void SetMidiControl(MIDIControl *mc);
	void SetSoundBank(SoundBank *b);
	void SetSoundFile(const char *b);
	void SetLocalPan(bsInt16 lp);
	
	/// @name Convenience functions
	/// These functions reference normalized MIDI channel status values.
	/// @{
	/// Return pitch wheel value for this channel
	inline FrqValue GetPitchbend(int chnl)
	{
		return midiCtrl->GetPitchbend(chnl);
	}

	/// Return modulation wheel value for this channel
	inline FrqValue GetModwheel(int chnl)
	{
		return midiCtrl->GetModwheel(chnl);
	}

	/// Return volume level channel
	inline AmpValue GetVolume(int chnl)
	{
		if (localVals & GMM_LOCAL_VOL)
			return volValue;
		return midiCtrl->GetVolumeN(chnl);
	}

	/// Return channel after touch
	inline AmpValue GetAftertouch(int chnl)
	{
		return midiCtrl->GetAftertouch(chnl);
	}

	/// Get channel pan value
	inline AmpValue GetPan(int chnl)
	{
		if (localVals & GMM_LOCAL_PAN)
			return panValue;
		return midiCtrl->GetPanN(chnl);
	}

	/// Get patch (program) number for the channel
	inline bsInt16 GetPatch(int chnl)
	{
		if (localVals & GMM_LOCAL_PATCH)
			return progValue;
		return midiCtrl->GetPatch(chnl);
	}

	/// Get patch bank number for the channel
	inline bsInt16 GetBank(int chnl)
	{
		if (localVals & GMM_LOCAL_PATCH)
			return bankValue;
		return midiCtrl->GetBank(chnl);
	}

	/// Get the raw controller value
	inline bsInt16 GetCC(int chnl, int ccn)
	{
		return midiCtrl->GetCC(chnl, ccn);
	}

	/// Set the raw controller value
	inline void SetCC(int chnl, bsInt16 ccn, bsInt16 val)
	{
		midiCtrl->SetCC(chnl, ccn, val);
	}
	//@}

	void ExclNoteOn(GMPlayer *player);

	/// Allocate a player object
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
