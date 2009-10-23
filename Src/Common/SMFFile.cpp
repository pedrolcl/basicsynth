//////////////////////////////////////////////////////////////////
/// @file SMFFile.cpp Standard MIDI Format file (.mid) loader.
//
// BasicSynth
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
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
#include <SMFFile.h>

int SMFFile::LoadFile(const char *file)
{
	CloseFile();

	if (fp.FileOpen(file))
		return -1;

	bsUint32 trkSize;
	if (fp.FileRead(&hdr.chnkID, 4) != 4
	 || hdr.chnkID != SMF_MTHD_CHUNK)
	{
		CloseFile();
		return -1;
	}

	hdr.size = ReadLong();
	if (hdr.size != 6)
	{
		CloseFile();
		return -1;
	}

	hdr.format = ReadShort();
	hdr.numTrk = ReadShort();
	hdr.tmDiv = ReadShort();
	if (hdr.tmDiv & 0x8000)
	{
		// SMTPE - punt for now
		CloseFile();
		return -2;
	}

	ppqn = ((double) hdr.tmDiv * 1.0e6);

	bsUint32 chnkID;
	int err = 0;
	bsUint16 trkNum = 0;
	while (trkNum < hdr.numTrk)
	{
		if (fp.FileRead(&chnkID, 4) != 4)
			break;

		trkSize = ReadLong();
		if (chnkID != SMF_MTRK_CHUNK)
		{
			// Ignore anythnig but MTrk chunks
			fp.FileSkip(trkSize);
			continue;
		}

		// Read in the track data
		inpBuf = new bsUint8[trkSize];
		if (inpBuf == NULL)
		{
			err = -1;
			break;
		}

		if ((bsUint32)fp.FileRead(inpBuf, trkSize) != trkSize)
		{
			err = -1;
			break;
		}

		inpPos = inpBuf;
		inpEnd = inpBuf + trkSize;
		lastMsg = 0;
		trackObj = new SMFTrack(trkNum);
		trackList.AddItem(trackObj);

		while (inpPos < inpEnd)
		{
			deltaT = GetVarLen();
			bsUint16 msg = *inpPos;
			if ((msg & MIDI_EVTMSK) == MIDI_SYSEX)
			{
				inpPos++;
				if (msg == MIDI_META)
					MetaEvent();
				else
					SysCommon(msg);
			}
			else
			{
				if (msg & MIDI_MSGBIT)
				{
					inpPos++;
					lastMsg = msg;
				}
				else
					msg = lastMsg;
				ChnlMessage(msg);
			}
		}

		delete inpBuf;
		inpBuf = NULL;
		inpPos = NULL;
		inpEnd = NULL;
		trkNum++;
	}

	CloseFile();
	return err;
}

void SMFFile::MetaEvent()
{
	static char *flats[]  = {"C", "F", "Bb", "Eb", "Ab", "Db", "Gb", "Cb"};
	static char *sharps[] = {"C", "G", "D",  "A",  "E",  "B",  "F#", "C#"};
	MIDIEvent *evt;

	bsUint16 meta = *inpPos++;
	bsUint32 metaLen = GetVarLen();

	bsUint16 val, val2, val3, val4;

	switch (meta)
	{
	case MIDI_META_TEXT:
		ReadString(metaText, metaLen);
		break;
	case MIDI_META_CPYR:
		ReadString(metaCpyr, metaLen);
		break;
	case MIDI_META_TRK:
		ReadString(metaSeqName, metaLen);
		break;
	case MIDI_META_TMSG:
		val  = *inpPos++;
		val2 = *inpPos++;
		val3 = *inpPos++;
		val4 = *inpPos++;
		timeSigNum = val;
		timeSigDiv = val2;
		timeSigBeat = val3;
		IntToStr(timeSig, val);
		timeSig += '/';
		IntToStr(timeSig, val2);
		timeSig += " (";
		IntToStr(timeSig, val3);
		timeSig += ')';
		break;
	case MIDI_META_KYSG:
		val  = *inpPos++;
		val2 = *inpPos++;
		keySigKey = val;
		keySigMaj = val2;
		keySig += (val & 0x80) ? flats[val&0x7f] : sharps[val];
		keySig += ' ';
		keySig += val2 ? "min." : "maj.";
		break;
	case MIDI_META_EOT:
		// The TAIL of the event list already has the EOT meta event.
		trackObj->EOTOffset(deltaT);
		break;
	case MIDI_META_TMPO:
		evt = new MIDIEvent;
		evt->deltat = (bsUint32) deltaT;
		evt->mevent = MIDI_META;
		evt->chan = MIDI_META_TMPO;
		evt->val1 = 0;
		evt->val2 = 0;
		evt->val3.lval = *inpPos++;
		evt->val3.lval = (evt->val3.lval << 8) + *inpPos++;
		evt->val3.lval = (evt->val3.lval << 8) + *inpPos++;
		trackObj->AddEvent(evt);
		break;
	case MIDI_META_INST:
	case MIDI_META_LYRK:
	case MIDI_META_MRKR:
	case MIDI_META_CUE:
	case MIDI_META_CHNL:
	default:
		inpPos += metaLen;
		break;
	}
}

void SMFFile::SysCommon(bsUint16 msg)
{
	bsUint32 dataLen;
	MIDIEvent *evt;

	switch (msg)
	{
	case MIDI_SYSEX:
		dataLen = GetVarLen();
		if (*inpPos == 0x7E || *inpPos == 0x7F)
		{
			// universal system exclusive.
			evt = new MIDIEvent;
			evt->deltat = deltaT;
			evt->mevent = msg;
			evt->val1 = inpPos[0];
			evt->val2 = (inpPos[2] << 8) | inpPos[3];
			trackObj->AddEvent(evt);
			if (inpPos[0] == 0x7E && inpPos[2] == 9) // GM level
				gmbank = inpPos[3];
		}
		inpPos += dataLen;
		break;
	case MIDI_SNGPOS:
		inpPos += 2;
		break;
	case MIDI_TMCODE:
	case MIDI_SNGSEL:
		inpPos += 1;
		break;
	case MIDI_TUNREQ:
	case MIDI_ENDEX:
	case MIDI_TMCLK:
	case MIDI_START:
	case MIDI_CONT:
	case MIDI_STOP:
	case MIDI_ACTSNS:
		break;
	default:
		break;
	}
}

void SMFFile::ChnlMessage(bsUint16 msg)
{
	MIDIEvent *evt = new MIDIEvent;
	evt->deltat = deltaT;
	evt->mevent = msg & MIDI_EVTMSK;
	evt->chan  = msg & MIDI_CHNMSK;

	switch (msg & MIDI_EVTMSK)
	{
	case MIDI_NOTEOFF:
	case MIDI_NOTEON:
	case MIDI_KEYAT:
	case MIDI_CTLCHG:
	case MIDI_PWCHG:
		evt->val1 = *inpPos++;
		evt->val2 = *inpPos++;
		break;
	case MIDI_PRGCHG:
	case MIDI_CHNAT:
		evt->val1 = *inpPos++;
		evt->val2 = 0;
		break;
	default:
		delete evt;
		return;
	}

	trackObj->AddEvent(evt);
}

int SMFFile::GenerateSeq(Sequencer *s, SMFInstrMap *map, SoundBank *sb)
{
	sbnk = sb;
	seq = s;
	instrMap = map;
	int trkNum = 0;
	SMFTrack *tp = 0;

	for (int ch = 0; ch < 16; ch++)
		chnlStatus[ch].Clear();
	chnlStatus[9].bank = 128; // FUNKY MIDI SMF STUFF

	while ((tp = trackList.EnumItem(tp)) != 0)
		tp->Start(this);

	theTick = 0;
	do
	{
		trkNum = 0;
		tp = 0;
		while ((tp = trackList.EnumItem(tp)) != 0)
		{
			if (tp->Generate())
				trkNum++;
		}
		theTick += srTicks;
	} while (trkNum > 0);

	return 1;
}

int SMFFile::GetChannelMap(bsInt32 *ch)
{
	int maxChnl = -1;
	for (int i = 0; i < 16; i++)
	{
		ch[i] = chnlStatus[i].count;
		if (ch[i])
			maxChnl = i;
	}
	return maxChnl+1;
}

void SMFFile::NoteOn(short chnl, short key, short vel, short track)
{
	SMFChnlStatus *cs = &chnlStatus[chnl];
	if (++cs->noteison[key] > 1)
	{
		// ruh-roh... In *theory* this should never happen.
		// you can't have more than one note-on active
		// since a keyboard SHOULD send a note-off before
		// it can possibly send another note-on! Right?
		// But it happens (possibly from someone manually
		// editing the .mid file) and we need to do something.
		// I'm not sure what the *official* MIDI protocol is --
		// I just count the multiple note-on/note-off events and 
		// do the note-off when the count is zero.
		return;
	}
	cs->noteOn[key] = theTick;
	cs->velocity[key] = vel;
}

void SMFFile::NoteOff(short chnl, short key, short vel, short track)
{
	SMFChnlStatus *cs = &chnlStatus[chnl];
	short on = --cs->noteison[key];
	if (on > 0)
		return; // multiple note-on; see comment above
	cs->noteison[key] = 0;
	if (on < 0)
		return; // note-off without note-on?

	FrqValue start = cs->noteOn[key];
	cs->noteOn[key] = 0;

	FrqValue dur = theTick - start;
	if (dur < 1.0) // "it happens..."
		return;

	SMFInstrMap *pm = &instrMap[chnl];
	InstrConfig *inc = pm->inc;

	NoteEvent *evt = (NoteEvent *) inc->instrType->manufEvent(inc->instrTmplt);
	evt->SetType(SEQEVT_START);
	evt->SetID(seq->NextEventID());
	evt->SetInum(pm->inc->inum);
	if (hdr.format == 2)
		evt->SetTrack(track);
	else
		evt->SetTrack(0);
	evt->SetChannel(chnl);
	evt->SetStart((bsInt32) start);
	if (explNoteOff)
		evt->SetDuration(0);
	else
		evt->SetDuration((bsInt32) dur);
	evt->SetPitch(key - 12);
	//evt->frq = synthParams.GetFrequency(evt->pitch);
	// volume gets applied at playback time from CC#7
	evt->SetVolume(1.0);
	evt->SetVelocity(cs->velocity[key]);
	if (pm->bnkParam > 0)
		evt->SetParam(pm->bnkParam, cs->bank);
	if (pm->preParam > 0)
		evt->SetParam(pm->preParam, cs->patch);
	seq->AddEvent(evt);
	cs->count++;

	if (explNoteOff)
	{
		NoteEvent *evt2 = (NoteEvent *) inc->instrType->manufEvent(inc->instrTmplt);
		evt2->SetType(SEQEVT_STOP);
		evt2->SetID(evt->evid);
		evt2->SetStart((bsUint32) (start + dur));
		evt2->SetDuration(0);
		evt2->SetInum(pm->inc->inum);
		evt2->SetPitch(evt->pitch);
		evt2->SetVolume(0);
		evt2->SetVelocity(0);
		seq->AddEvent(evt2);
		cs->count++;
	}
	//printf("Add Note @%f for %f key %d on channel %d\n", start / synthParams.sampleRate, dur / synthParams.sampleRate, key, chnl);
}

void SMFFile::SetTempo(long val)
{
	srTicks = ((FrqValue)val * synthParams.sampleRate) / ppqn;
}

void SMFFile::ProgChange(short chnl, short val, short track)
{
	if (gmbank == 1)
	{
		if (chnl == 9) // always "STANDARD DRUM SET"
			val = 0;
	}
	chnlStatus[chnl].patch = val;
	if (sbnk)
		sbnk->GetInstr(chnlStatus[chnl].bank, chnlStatus[chnl].patch, 1);
	AddControlEvent(MIDI_PRGCHG, chnl, -1, val, track);
}

void SMFFile::ControlChange(short chnl, short ctl, short val, short track)
{
	if (chnl < 0 || chnl > 15)
		return;
	SMFChnlStatus *cs = &chnlStatus[chnl];
	switch (ctl)
	{
	case MIDI_CTRL_BANK_LSB:
		if (gmbank == 1)
			break; // for GM1, bank is always 0 or 128, determined by the channel
		cs->bank &= ~0x7f;
		cs->bank |= val;
		AddControlEvent(MIDI_CTLCHG, chnl, ctl, val, track);
		break;
	case MIDI_CTRL_BANK:
		if (gmbank == 1)
			break;
		cs->bank &= ~0x3F8;
		cs->bank |= (val << 7);
		AddControlEvent(MIDI_CTLCHG, chnl, ctl, val, track);
		break;
	default:
		AddControlEvent(MIDI_CTLCHG, chnl, ctl, val, track);
		break;
	}
}

void SMFFile::AddControlEvent(short mmsg, short chnl, short ctl, short val, short track)
{
	if (chnl >= 0 && chnl <= 15)
		chnlStatus[chnl].count++;

	ControlEvent *evt = new ControlEvent;
	evt->SetType(SEQEVT_CONTROL);
	evt->SetChannel(chnl);
	if (hdr.format == 2)
		evt->SetTrack(track);
	else
		evt->SetTrack(0);
	evt->SetStart((bsInt32) theTick);
	evt->SetDuration(0);
	evt->SetID(seq->NextEventID());
	evt->SetMessage(mmsg);
	evt->SetControl(ctl);
	evt->SetValue(val);
	seq->AddEvent(evt);
	//printf("AddEvent @%f chnl=%d mmsg=%d ctrl=%d cval=%d\n", (FrqValue) evt->start / synthParams.sampleRate,
	//	evt->mmsg, evt->ctrl, evt->cval);
}

void SMFFile::KeyAfterTouch(short chnl, short key, short val, short track)
{
	// Not implemented
}

void SMFFile::ChannelAfterTouch(short chnl, short val, short track)
{
	AddControlEvent(MIDI_CHNAT, chnl, -1, val, track);
}

void SMFFile::PitchBend(short chnl, short val, short track)
{
	AddControlEvent(MIDI_PWCHG, chnl, -1, val, track);
}

int SMFTrack::Generate()
{
	if (eot || !smf)
		return 0;
	while (!eot && deltaTime == 0)
	{
		short chan = evtLast->chan;
		short val1 = evtLast->val1;
		short val2 = evtLast->val2;
		switch (evtLast->mevent)
		{
		case MIDI_NOTEON:
			if (evtLast->val2 != 0)
				smf->NoteOn(chan, val1, val2, trkNum);
			else // heh, heh
		case MIDI_NOTEOFF:
			smf->NoteOff(chan, val1, val2, trkNum);
			break;
		case MIDI_KEYAT:
			smf->KeyAfterTouch(chan, val1, val2, trkNum);
			break;
		case MIDI_CTLCHG:
			smf->ControlChange(chan, val1, val2, trkNum);
			break;
		case MIDI_PRGCHG:
			smf->ProgChange(chan, val1, trkNum);
			break;
		case MIDI_CHNAT:
			smf->ChannelAfterTouch(chan, val1, trkNum);
			break;
		case MIDI_PWCHG:
			smf->PitchBend(chan, val1 + (val2 << 7), trkNum);
			break;
		case MIDI_META:
			if (chan == MIDI_META_TMPO)
				smf->SetTempo(evtLast->val3.lval);
			else if (chan == MIDI_META_EOT)
			{
				eot = true;
				return 0;
			}
			break;
		}
		evtLast = evtLast->next;
		if (evtLast) // sanity check, EOT should stop us before this point
			deltaTime = evtLast->deltat;
		else
			eot = true;
	}
	deltaTime--;
	return 1;
}
