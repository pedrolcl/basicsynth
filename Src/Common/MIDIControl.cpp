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

void MIDISeqControl::ProcessEvent(SeqEvent *evt, bsInt16 flags)
{
	MIDIChannelStatus *ctl = &channel[evt->chnl];
	ControlEvent *cevt = (ControlEvent *)evt;
	ctl->ControlMessage(cevt->mmsg, cevt->ctrl, cevt->cval);
}

void MIDISeqControl::Tick()
{
/*	MIDIChannelStatus *chp = &channel[0];
	MIDIChannelStatus *end = &channel[16];
	while (chp != end)
	{
		if (chp->enable)
			chp->Tick();
		chp++;
	}*/
}

void MIDIChannelStatus::ControlMessage(bsInt16 mmsg, bsInt16 ctrl, bsInt16 cval)
{
	bsInt32 bigVal;
	switch (mmsg)
	{
	case MIDI_CTLCHG:
		cc[ctrl] = cval;
		switch (ctrl)
		{
		case MIDI_CTRL_BANK:
		case MIDI_CTRL_BANK_LSB:
			bank = (cc[MIDI_CTRL_BANK] << 7) + cc[MIDI_CTRL_BANK_LSB];
			break;
		case MIDI_CTRL_MOD:
		case MIDI_CTRL_MOD_LSB:
			bigVal = (cc[MIDI_CTRL_MOD] << 7) + cc[MIDI_CTRL_MOD_LSB];
			modwheel = (FrqValue) bigVal / 16384.0;
			break;
		case MIDI_CTRL_VOL:
		case MIDI_CTRL_VOL_LSB:
			bigVal = (cc[MIDI_CTRL_VOL] << 7) + cc[MIDI_CTRL_VOL_LSB];
			volume = (AmpValue) bigVal / 16384.0;
			break;
		case MIDI_CTRL_PAN:
		//case MIDI_CTRL_PAN_LSB:
			//bigVal = (ctl->cc[MIDI_CTRL_PAN] << 7) + ctl->cc[MIDI_CTRL_PAN_LSB];
			pan.Set(panSqr, (AmpValue) (cval - 64) / 64);
			break;
		case MIDI_CTRL_EXPR:
			expression = (AmpValue) cval / 127.0;
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
		}
		break;
	case MIDI_PRGCHG:
		patch = cval;
		break;
	case MIDI_CHNAT:
		aftertouch = (AmpValue)cval / 127.0;
		break;
	case MIDI_PWCHG:
		bigVal = (bsInt32) cval * ((pbsemi * 100) + pbcents) / 8192;
		pitchbend = synthParams.GetCentsMult(bigVal);
		break;
	}
}
