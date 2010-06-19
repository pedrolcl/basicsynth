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
	bsInt16 cc[128];      ///< controller "raw" values
	bsInt16 at[128];      ///< polyphonic after touch
	bsInt16 bank;         ///< bank number
	bsInt16 patch;        ///< patch (aka program)
	bsInt16 aftertouch;   ///< channel pressure
	bsInt16 pitchbend;    ///< pitch bend
	bsInt16 pbsemi;       ///< pitch bend resolution, semi-tones
	bsInt16 pbcents;      ///< pitch bend resolution, cents
	bsInt16 mwsemi;       ///< mod wheel sensitivity, semi-tones
	bsInt16 mwcents;      ///< mod wheel sensitivity, cents
	bsInt16 finetune;     ///< fine tuning, cents
	bsInt16 coarsetune;   ///< course tuning, semi-tones
	bsInt16 rpnnum;       ///< currently selected RPN parameter
	bsInt16 enable;       ///< channel is enabled to process messages
	bsInt16 channel;      ///< channel number, for reference

	MIDIChannelStatus();
	void Reset();
};

/// MIDI channel control information.
/// This class aggregates some of the bits and pieces needed to
/// emulate a MIDI keyboard synth. Sequencer control events
/// are stored in the channel[] objects. The channel objects can be
/// accessed directly for programatic control, i.e. when
/// live MIDI data is being received.
///
/// A MIDI-aware instrument should be passed a reference
/// to a MIDIControl object when then instrument is created.
/// During playback, the instrument can retrieve various
/// parameters from the channel[] objects.
///
/// Functions with 'N' return normalized values in the
/// range [0,1]. Functions with 'CB' return centibels
/// of attenation, range [0,960] where 0 is no attenuation and
/// 960 is silence. Functions with 'C' return pitch cents.
///
/// The instrument manager inherits from this class and implements
/// the VolumeChange() and ControlChange() methods to distribute
/// changes to the mixer and active voices.
class MIDIControl
{
public:
	MIDIChannelStatus channel[16];  ///< Channel status information
	bsInt16 switchLevel;

	MIDIControl();

	virtual void ProcessEvent(SeqEvent *evt, bsInt16 flags);
	virtual void ProcessMessage(short mm, short v1, short v2);
	virtual void ControlChange(int chnl, int ccx, int val) { }
	virtual void PitchbendChange(int chnl, int v1, int v2) { }
	virtual void AftertouchChange(int chnl, int val) { }

	void Reset()
	{
		for (int ch = 0; ch < 16; ch++)
		{
			channel[ch].channel = ch;
			channel[ch].Reset();
		}
		channel[9].bank = 128;
	}

	void EnableChannel(int ch, int enable)
	{
		channel[ch].enable = enable;
	}

	/// Convenience functions

	// Get raw pitchbend
	inline FrqValue GetPitchbend(int chnl)
	{
		return channel[chnl].pitchbend;
	}

	/// Get normalized pitchbend.
	inline FrqValue GetPitchbendN(int chnl)
	{
		return FrqValue(channel[chnl].pitchbend - 8192) / 8192.0;
	}

	/// Get pitchbend in cents. 
	/// Scales normalized with RPN0.
	inline FrqValue GetPitchbendC(int chnl)
	{
		return GetPitchbendN(chnl) * GetPitchbendRange(chnl);
	}

	/// Get pitchbend range in cents. 
	inline FrqValue GetPitchbendRange(int chnl)
	{
		return FrqValue((channel[chnl].pbsemi * 100) + channel[chnl].pbcents);
	}

	/// Get raw volume.
	inline bsInt16 GetVolume(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_VOL];
	}

	inline void SetVolume(int chnl, int val)
	{
		channel[chnl].cc[MIDI_CTRL_VOL] = val;
		ControlChange(chnl, MIDI_CTRL_VOL, val);
	}

	/// Get scaled volume
	inline AmpValue GetVolumeN(int chnl)
	{
		return AmpValue(GetVolume(chnl)) / 127.0;
	}

	inline AmpValue GetVolumeCB(int chnl)
	{
		return GetVolumeN(chnl) * 960.0;
	}

	inline bsInt16 GetModwheel(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_MOD];
	}

	inline AmpValue GetModwheelN(int chnl)
	{
		return AmpValue(channel[chnl].cc[MIDI_CTRL_MOD]) / 127.0;
	}

	inline AmpValue GetModwheelCents(int chnl)
	{
		return GetModwheelN(chnl) * GetModwheelRange(chnl);
	}

	inline AmpValue GetModwheelRange(int chnl)
	{
		return AmpValue((channel[chnl].mwsemi * 100) + channel[chnl].mwcents);
	}

	inline bsInt16 GetExpr(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_EXPR];
	}

	inline AmpValue GetExprN(int chnl)
	{
		return AmpValue(channel[chnl].cc[MIDI_CTRL_EXPR]) / 127.0;
	}

	inline AmpValue GetExprCB(int chnl)
	{
		return AmpValue(channel[chnl].cc[MIDI_CTRL_EXPR]) * 960.0 / 127.0;
	}

	inline bsInt16 GetKeyAftertouch(int chnl, int key)
	{
		return channel[chnl].at[key&0x7f];
	}

	inline AmpValue GetKeyAftertouchN(int chnl, int key)
	{
		return GetKeyAftertouchN(chnl, key) / 127.0;
	}

	inline AmpValue GetKeyAftertouchCB(int chnl, int key)
	{
		return GetKeyAftertouchN(chnl, key) * 960.0 / 127.0;
	}

	inline bsInt16 GetAftertouch(int chnl)
	{
		return channel[chnl].aftertouch;
	}

	inline AmpValue GetAftertouchN(int chnl)
	{
		return AmpValue(channel[chnl].aftertouch) / 127.0;
	}

	inline AmpValue GetAftertouchCB(int chnl)
	{
		return AmpValue(channel[chnl].aftertouch) * 960.0 / 127.0;
	}

	inline bsInt16 GetPatch(int chnl)
	{
		return channel[chnl].patch;
	}

	inline void SetPatch(int chnl, int patch)
	{
		channel[chnl].patch = patch;
	}

	/// Get the bank setting.
	/// This follows the GM/DLS spec of using
	/// either bank MSB or channel to determine
	/// the Sound bank number where 128 is the
	/// percussion bank and 0 is the primary
	/// melodic bank. To get the
	/// actual bank settings (for bank mapping)
	/// use GetCC() with the appropriate CC#.
	/// @param chnl channel number
	/// @returns Sound bank number.
	inline bsInt16 GetBank(int chnl)
	{
		bsInt16 bmsb = channel[chnl].cc[MIDI_CTRL_BANK];
		if (bmsb == 0x78)  // GM percussion bank
			return 128;
		if (bmsb == 0x79) // GM melodic bank
			return channel[chnl].cc[MIDI_CTRL_BANK_LSB];
		if (chnl == 9)
			return 128;
		return 0;
	}

	inline void SetBank(int chnl, int bank)
	{
		if (bank == 128)
		{
			channel[chnl].cc[MIDI_CTRL_BANK] = 0x78;
			channel[chnl].cc[MIDI_CTRL_BANK_LSB] = 0;
		}
		else
		{
			channel[chnl].cc[MIDI_CTRL_BANK] = 0x79;
			channel[chnl].cc[MIDI_CTRL_BANK_LSB] = bank;
		}
		channel[chnl].bank = bank;
	}

	inline bsInt16 GetPan(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_PAN];
	}

	inline void SetPan(int chnl, int val)
	{
		channel[chnl].cc[MIDI_CTRL_PAN] = val;
		ControlChange(chnl, MIDI_CTRL_PAN, val);
	}

	inline AmpValue GetPanN(int chnl)
	{
		return AmpValue(channel[chnl].cc[MIDI_CTRL_PAN] - 64) / 64.0;
	}

	inline bsInt32 GetFineTune(int chnl)
	{
		return channel[chnl].finetune;
	}

	inline bsInt32 GetFineTuneCents(int chnl)
	{
		return ((channel[chnl].finetune - 8192) * 100) / 8192;
	}

	inline bsInt32 GetCoarseTune(int chnl)
	{
		return channel[chnl].coarsetune;
	}

	inline bsInt16 GetSwitch(int chnl, int sw)
	{
		return channel[chnl].cc[sw] > switchLevel;
	}

	inline bsInt16 GetCC(int chnl, int ccx)
	{
		return channel[chnl].cc[ccx];
	}

	inline void SetCC(int chnl, int ccx, int val)
	{
		channel[chnl].cc[ccx] = val;
		ControlChange(chnl, ccx, val);
	}

	inline AmpValue GetCCN(int chnl, int ccx)
	{
		return AmpValue(channel[chnl].cc[ccx]) / 127.0;
	}
};

//@}

#endif
