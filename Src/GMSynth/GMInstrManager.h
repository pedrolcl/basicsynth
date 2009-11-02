//////////////////////////////////////////////////////////////////////
/// @file GMInstrManager.h Instrument manager for GMSynth
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpMIDI
//@{
#ifndef GMINSTRMANAGER_H
#define GMINSTRMANAGER_H

/// Specialized instrument manager for GM.
/// GMInstrManager inherits from InstrManager and overrides
/// the generic instrument allocation functions, always
/// returning a GMPlayer instrument regardless of the value
/// set for the instrument in the SeqEvent structure.
///
/// We bypass the mixer because the volume levels are controlled
/// by the MIDI CC# 7 on a per-channel basis. The mixer inputs
/// are, therefore, redunant.
///
/// This implementation adds/destroys instruments on each
/// note. A possible improvement is to maintain a cache
/// of instruments, much the same way a keyboard synth
/// has a fixed number of voices.
class GMInstrManager : public InstrManager
{
protected:
	AmpValue outLft;
	AmpValue outRgt;
	AmpValue masterVol;
	GMManager *gm;
	MIDIControl *mc;

public:
	GMInstrManager()
	{
		masterVol = 1.0;
		InstrMapEntry *ime = AddType("GMPlayer",
			GMManager::InstrFactory,
			GMManager::EventFactory,
			GMManager::TmpltFactory);
		ime->dumpTmplt = GMManager::TmpltDump;
		gm = new GMManager;
		AddInstrument(1, ime, gm);
	}

	~GMInstrManager()
	{
		InstrConfig *ime;
		while ((ime = instList) != 0)
		{
			instList = ime->next;
			delete ime;
		}
	}

	void SetController(MIDIControl *p)
	{
		mc = p;
		gm->SetMidiControl(mc);
	}

	void SetSoundBank(SoundBank *sb)
	{
		gm->SetSoundBank(sb);
	}

	void SetVolume(AmpValue v)
	{
		masterVol = v;
	}

	virtual void Clear()
	{
		// don't discard the GMManager object.
	}

	virtual Instrument *Allocate(bsInt16 inum)
	{
		return GMManager::InstrFactory(this, gm);
	}

	virtual Instrument *Allocate(InstrConfig *in)
	{
		return GMManager::InstrFactory(this, gm);
	}

	virtual void Deallocate(Instrument *ip)
	{
		ip->Destroy();
	}

	virtual SeqEvent *ManufEvent(bsInt16 inum)
	{
		return GMManager::EventFactory(gm);
	}

	virtual SeqEvent *ManufEvent(InstrConfig *in)
	{
		return GMManager::EventFactory(gm);
	}

	/// Start is called by the sequencer 
	/// when the sequence starts.
	virtual void Start()
	{
		outLft = 0;
		outRgt = 0;
	}
	
	/// Tick outputs the current sample.
	virtual void Tick()
	{
		wvf->Output2(masterVol * outLft, masterVol * outRgt);
		outLft = 0;
		outRgt = 0;
	}

	/// FxSend sends values to an effects unit (TDB)
	virtual void FxSend(int unit, AmpValue val)
	{
		// TODO: send to reverb/chorus
	}

	/// Output a mono sample
	virtual void Output(int ch, AmpValue val)
	{
		outLft += val;
		outRgt += val;
	}

	/// Output a stereo mono sample
	virtual void Output2(int ch, AmpValue lft, AmpValue rgt)
	{
		outLft += lft;
		outRgt += rgt;
	}
};
#endif
//@}
