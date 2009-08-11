/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file MIDIControl.h MIDI Control classes
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef MIDICONTROL_H
#define MIDICONTROL_H

/// MIDI Channel status information.
/// This class stores information for one MIDI channel.
/// It has members to hold the raw MIDI values (cc) as
/// well as the transformed values for the most commonly
/// used voice controls.
class MIDIChannelStatus
{
public:
	AmpValue volume;
	AmpValue aftertouch;
	AmpValue expression;
	FrqValue pitchbend;
	FrqValue modwheel;
	Panner pan;
	bsInt16 bank;
	bsInt16 patch;
	bsInt16 cc[128]; // "raw" values
	int enable;
	bsInt32 pbcents;
	bsInt32 pbsemi;
	bsInt16 rpnlsb;
	bsInt16 rpnmsb;

	MIDIChannelStatus()
	{
		Reset();
	}

	void Reset()
	{
		volume = 1.0;
		aftertouch = 0.0;
		expression = 0.0;
		pitchbend = 1.0;
		modwheel = 0.0;
		bank = 0;
		patch = 0;
		rpnlsb = -1;
		rpnmsb = -1;
		pbsemi = 2; // should this be 1 or 2 by default?
		pbcents = 0;
		memset(cc, 0, sizeof(cc));
		cc[MIDI_CTRL_VOL] = 127;
		cc[MIDI_CTRL_VOL_LSB] = 127;
		cc[MIDI_CTRL_PAN] = 64;
		enable = 1;
	}

	void ControlMessage(bsInt16 mmsg, bsInt16 ctrl, bsInt16 cval);

	void Tick()
	{
		// at present - nothing to do here
	}
};


/// Channel controller class for MIDI events.
/// This handles controller events from the sequencer.
class MIDISeqControl : public SeqControl
{
private:
	MIDIChannelStatus *channel;

public:
	MIDISeqControl(MIDIChannelStatus *c)
	{
		channel = c;
	}

	virtual void ProcessEvent(SeqEvent *evt, bsInt16 flags);
	virtual void Tick();
};

/// MIDI Instrument control information.
/// This class aggregates the bits and pieces needed to
/// emulate a MIDI keyboard synth. Sequencer control events
/// are routed through the seqControl object and stored
/// in the channel[] objects. The channel objects can be
/// accessed directly for programatic control, i.e. when
/// live MIDI data is being received.
/// A MIDI-aware instrument should be passed a reference
/// to a MIDIControl object when then instrument is created.
/// During playback, the instrument retrieves various
/// parameters from the channel[] objects rather than
/// having the information passed in a BasicSynth START or PARAM
/// event. All MIDI-aware instruments should use the common LFO object as well.
class MIDIControl
{
public:
	MIDIChannelStatus channel[16];  // Channel status information
	MIDISeqControl seqControl;      // Sequencer event handler

	MIDIControl() : seqControl(channel)
	{
	}
	
	void Reset()
	{
		for (int ch = 0; ch < 16; ch++)
			channel[ch].Reset();
	}

	void EnableChannel(int ch, int enable)
	{
		channel[ch].enable = enable;
	}

	// Convenience functions

	inline FrqValue GetPitchbend(int chnl)
	{
		return channel[chnl].pitchbend;
	}

	inline FrqValue GetModwheel(int chnl)
	{
		return channel[chnl].modwheel;
	}

	inline AmpValue GetVolume(int chnl)
	{
		return channel[chnl].volume;
	}

	inline void SetVolume(int chnl, AmpValue vol)
	{
		channel[chnl].cc[7] = (bsInt16) (vol * 127.0);
		channel[chnl].volume = vol;
	}

	inline AmpValue GetAftertouch(int chnl)
	{
		return channel[chnl].aftertouch;
	}

	inline bsInt16 GetPatch(int chnl)
	{
		return channel[chnl].patch;
	}

	inline void SetPatch(int chnl, bsInt16 patch)
	{
		channel[chnl].patch = patch;
	}

	inline bsInt16 GetBank(int chnl)
	{
		return channel[chnl].bank;
	}

	inline void SetBank(int chnl, bsInt16 bank)
	{
		channel[chnl].bank = bank;
	}

	inline bsInt16 GetCC(int chnl, int ccn)
	{
		return channel[chnl].cc[ccn];
	}

	inline void SetCC(int chnl, bsInt16 ccn, bsInt16 val)
	{
		channel[chnl].ControlMessage(MIDI_CTLCHG, ccn, val);
	}
};

//@}

#endif
