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
/// @addtogroup grpMIDI
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
	FrqValue pitchbendN;  ///< Normalized pitch bend (a/k/a pitch wheel)
	AmpValue aftertouchN; ///< Normalized channel aftertouch
	bsInt16 pitchbend;    ///< Raw pitch bend
	bsInt16 aftertouch;   ///< raw channel aftertouch
	bsInt16 cc[128];      ///< "raw" values
	float   ccn[128];     ///< "normalized" values
	bsInt16 bank;         ///< calculated bank number
	bsInt16 patch;        ///< patch (aka program)
	bsInt32 pbcents;      ///< pitch bend resolution, cents
	bsInt32 pbsemi;       ///< pitch bend resolution, semi-tones
	bsInt16 rpnlsb;       ///< currently selected parameter, lsb
	bsInt16 rpnmsb;       ///< currently selected parameter, msb
	int enable;           ///< channel is enabled to process messages
	int channel;          ///< channel number, for reference
	bsUint32 tickcnt;     ///< tick count - for recording, debugging or notification of changes
	bsUint32 tickchng;    ///< tick number of last changed value

	MIDIChannelStatus();
	void Reset();
	void ControlMessage(bsInt16 mmsg, bsInt16 ctrl, bsInt16 cval);

	void Tick()
	{
		tickcnt++;
		// at present - nothing to do here
		// possbile: send out notifications of changes to registered objects
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

/// MIDI channel control information.
/// This class aggregates some of the bits and pieces needed to
/// emulate a MIDI keyboard synth. Sequencer control events
/// are routed through the seqControl object and stored
/// in the channel[] objects. The channel objects can be
/// accessed directly for programatic control, i.e. when
/// live MIDI data is being received.
///
/// A MIDI-aware instrument should be passed a reference
/// to a MIDIControl object when then instrument is created.
/// During playback, the instrument retrieves various
/// parameters from the channel[] objects rather than
/// having the information passed in a BasicSynth START or PARAM
/// event.
///
/// Functions with 'N' return normalized values in the
/// range [0,1]. Functions with 'CB' return centibels
/// of attenation, range [0,960] where 0 is no attenuation and
/// 960 is silence. The volCB[] and volAmp[] tables can also
/// be used to perform transforms on raw values into either
/// centibels of attenuation or linear volume using a convex curve.
class MIDIControl
{
public:
	MIDIChannelStatus channel[16];  ///< Channel status information
	MIDISeqControl seqControl;      ///< Sequencer event handler
	static AmpValue volCB[128];     ///< volume (or velocity) to centibels (960 - 0)
	static AmpValue volAmp[128];    ///< volume (or velocity) to amplitude (0 - 1)

	MIDIControl() : seqControl(channel)
	{
		for (int ch = 0; ch < 16; ch++)
			channel[ch].channel = ch;
		volCB[0] = 960.0;
		volAmp[0] = 0.0;
		for (int vol = 1; vol < 128; vol++)
		{
			double cb = 200.0 * log10((127.0*127.0) / (double) (vol * vol));
			volCB[vol] = (AmpValue) cb;
			volAmp[vol] = synthParams.AttenCB((int)(cb+0.5));
		}
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

	/// Convenience functions

	inline FrqValue GetPitchbend(int chnl)
	{
		return channel[chnl].pitchbend;
	}

	inline FrqValue GetPitchbendN(int chnl)
	{
		return channel[chnl].pitchbendN;
	}

	inline bsInt16 GetVolume(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_VOL];
	}

	inline void SetVolume(int chnl, AmpValue vol)
	{
		channel[chnl].ccn[MIDI_CTRL_VOL] = vol;
		channel[chnl].cc[MIDI_CTRL_VOL] = (bsInt16) (vol * 127.0);
		channel[chnl].cc[MIDI_CTRL_VOL_LSB] = 0;
	}

	inline float GetVolumeN(int chnl)
	{
		return channel[chnl].ccn[MIDI_CTRL_VOL];
	}

	inline AmpValue GetVolumeCB(int chnl)
	{
		return volCB[channel[chnl].cc[MIDI_CTRL_VOL]];
	}

	inline bsInt16 GetModwheel(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_MOD];
	}

	inline float GetModwheelN(int chnl)
	{
		return channel[chnl].ccn[MIDI_CTRL_MOD];
	}

	inline AmpValue GetModwheelCB(int chnl)
	{
		return volCB[channel[chnl].cc[MIDI_CTRL_MOD]];
	}

	inline bsInt16 GetExpr(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_EXPR];
	}

	inline float GetExprN(int chnl)
	{
		return channel[chnl].ccn[MIDI_CTRL_EXPR];
	}

	inline AmpValue GetExprCB(int chnl)
	{
		return volCB[channel[chnl].cc[MIDI_CTRL_EXPR]];
	}

	inline bsInt16 GetAftertouch(int chnl)
	{
		return channel[chnl].aftertouch;
	}

	inline AmpValue GetAftertouchN(int chnl)
	{
		return (AmpValue) channel[chnl].aftertouchN;
	}

	inline AmpValue GetAftertouchCB(int chnl)
	{
		return volCB[channel[chnl].aftertouch];
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

	inline AmpValue GetPan(int chnl)
	{
		return channel[chnl].ccn[MIDI_CTRL_PAN];
	}

	inline void SetPan(int chnl, AmpValue val)
	{
		channel[chnl].ccn[MIDI_CTRL_PAN] = val;
		bsInt16 ival = 8192 + (bsInt16) (val * 8192);
		channel[chnl].cc[MIDI_CTRL_PAN] = (ival >> 7) & 0x7f;
		channel[chnl].cc[MIDI_CTRL_PAN_LSB] = ival & 0x7f;
	}

	inline bsInt16 GetCC(int chnl, int ccn)
	{
		return channel[chnl].cc[ccn];
	}

	inline AmpValue GetCCN(int chnl, int ccn)
	{
		return channel[chnl].ccn[ccn];
	}

	inline void SetCC(int chnl, bsInt16 ccn, bsInt16 val)
	{
		channel[chnl].ControlMessage(MIDI_CTLCHG, ccn, val);
	}
};

//@}

#endif
