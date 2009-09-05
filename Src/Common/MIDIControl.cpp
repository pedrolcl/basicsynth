/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file MIDIControl.cpp MIDI Control classes implementation
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthString.h>
#include <WaveFile.h>
#include <Mixer.h>
#include <SynthList.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <Instrument.h>
#include <Sequencer.h>
#include <GenWaveWT.h>
#include <MIDIDefs.h>
#include <MIDIControl.h>

AmpValue MIDIControl::volCB[128];
AmpValue MIDIControl::volAmp[128];

//void MIDISeqControl::ProcessEvent(SeqEvent *evt, bsInt16 flags)
void MIDIControl::ProcessEvent(SeqEvent *evt, bsInt16 flags)
{
	MIDIChannelStatus *ctl = &channel[evt->chnl];
	ControlEvent *cevt = (ControlEvent *)evt;
	ctl->ControlMessage(cevt->mmsg, cevt->ctrl, cevt->cval);
}

//void MIDISeqControl::Tick()
void MIDIControl::Tick()
{
	MIDIChannelStatus *chp = &channel[0];
	MIDIChannelStatus *end = &channel[16];
	while (chp != end)
	{
		if (chp->enable)
			chp->Tick();
		chp++;
	}
}

MIDIChannelStatus::MIDIChannelStatus()
{
	Reset();
}

void MIDIChannelStatus::Reset()
{
	aftertouch = 0;
	aftertouchN = 0.0;
	pitchbendN = 0.0;
	pitchbend = 8192;
	bank = 0;
	patch = 0;
	rpnlsb = -1;
	rpnmsb = -1;
	pbsemi = 2;
	pbcents = 0;
	memset(cc, 0, sizeof(cc));
	memset(ccn, 0, sizeof(ccn));
	cc[MIDI_CTRL_VOL] = 127;
	cc[MIDI_CTRL_VOL_LSB] = 0;
	cc[MIDI_CTRL_PAN] = 64;
	ccn[MIDI_CTRL_VOL] = 1.0;
	ccn[MIDI_CTRL_PAN] = 0.0;
	enable = 1;
	tickcnt = 0;
}


/// Handle a control message.
void MIDIChannelStatus::ControlMessage(bsInt16 mmsg, bsInt16 ctrl, bsInt16 cval)
{
	tickchng = tickcnt;
	switch (mmsg)
	{
	case MIDI_CTLCHG:
		cc[ctrl] = cval & 0x7f;
		switch (ctrl)
		{
		case MIDI_CTRL_BANK:
		case MIDI_CTRL_BANK_LSB:
			if (cc[MIDI_CTRL_BANK] == 120) // GM2 percussion bank
				bank = 128;
			else if (cc[MIDI_CTRL_BANK] == 121) // GM2 melodic bank
				bank = cc[MIDI_CTRL_BANK_LSB];
			else
				bank = (cc[MIDI_CTRL_BANK] << 7) + cc[MIDI_CTRL_BANK_LSB];
			break;
		case MIDI_CTRL_PAN:
		case MIDI_CTRL_PAN_LSB:
			ccn[MIDI_CTRL_PAN] = (AmpValue) (((cc[MIDI_CTRL_PAN] << 7) | cc[MIDI_CTRL_PAN_LSB]) - 8192) / 8192.0;
			break;
		case MIDI_CTRL_DATA:
			if (rpnmsb == 0)
				pbsemi = (bsInt32) cval;
			break;
		case MIDI_CTRL_DATA_LSB:
			if (rpnlsb == 0)
				pbcents = (bsInt32) cval;
			break;
		case MIDI_CTRL_RPNLSB:
			rpnlsb = cval;
			break;
		case MIDI_CTRL_RPNMSB:
			rpnmsb = cval;
			break;
		default:
			if (ctrl < 0x40)
			{
				bsInt16 msb = ctrl & ~0x20;
				ccn[msb] = (float) ((cc[msb] << 7) | cc[ctrl | 0x20]) / 16384.0;
			}
			else if (ctrl < 0x60)
				ccn[ctrl] = (float) cval / 127.0; // or 128?
			break;
		}
		break;
	case MIDI_PRGCHG:
		patch = cval;
		break;
	case MIDI_CHNAT:
		aftertouch = cval;
		aftertouchN = (AmpValue) cval / 128.0;
		break;
	case MIDI_PWCHG:
		pitchbend = cval;
		pitchbendN = (FrqValue) ((bsInt32) (cval - 8192) * ((pbsemi * 100) + pbcents)) / 8192.0;
		break;
	}
}
