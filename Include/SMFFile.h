/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file MIDISequencer.h MIDI Sequencer Classes
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{
#ifndef _SMFFILE_H_
#define _SMFFILE_H_

#include <MIDIDefs.h>
#include <SoundBank.h>

/// A MIDI event.
/// The MIDI event holds information needed
/// for each event. There are never more than
/// two parameters for an event.
class MIDIEvent : public SynthList<MIDIEvent>
{
public:
	bsUint32 deltat; // deltaTime
	bsUint16 mevent;
	bsUint16 chan;  // channel or meta
	bsUint16 val1;
	bsUint16 val2;
	union
	{
		bsUint32 lval;
		void *pval;
	} val3;

	MIDIEvent()
	{
		deltat = 0;
		mevent = 0;
		chan = 0;
		val1 = 0;
		val2 = 0;
		val3.lval = 0;
	}
};

class SMFFile; // forward reference

/// MIDI track.
/// This is the class that maintains a list of MIDI events.
/// To produce a playable set of events, the SMFFile class
/// repeatedly evokes Generate() until the track is finished.
class SMFTrack : public SynthList<SMFTrack>
{
public:
	SMFFile   *smf;
	MIDIEvent evtHead;
	MIDIEvent evtTail;
	MIDIEvent *evtLast;
	int eot;
	int trkNum;

	bsUint32 deltaTime;

	SMFTrack()
	{
		trkNum = -1;
		Init();
	}

	SMFTrack(int t)
	{
		trkNum = t;
		Init();
	}

	void Init()
	{
		smf = 0;
		evtHead.Insert(&evtTail);
		evtLast = evtHead.next;
		evtHead.deltat = 0;
		evtHead.mevent = 0;
		evtTail.deltat = 0;
		evtTail.mevent = MIDI_META;
		evtTail.chan  = MIDI_META_EOT;
		eot = 0;
		deltaTime = 0;
	}

	~SMFTrack()
	{
		while ((evtLast = evtHead.next) != &evtTail)
		{
			evtLast->Remove();
			delete evtLast;
		}
	}

	// Add the event to the sequence. The caller is responsible for setting
	// valid values for inum, start, duration, type and eventid.
	void AddEvent(MIDIEvent *evt)
	{
		//deltaTime += evt->deltat;
		//printf("TRACK(%02d): @%08d (%04d) %02d %02x %3d %3d\n", 
		//	   trkNum, deltaTime, evt->deltat, evt->chan, evt->mevent, evt->val1, evt->val2);
		evtTail.InsertBefore(evt);
	}

	/// Set the delta-time before end-of-track marker.
	void EOTOffset(bsInt32 deltat)
	{
		//printf("TRACK(%02d): @%08d EOT ======================\n", trkNum, deltaTime);
		// Not really necessary, but might 
		// be used to extend the track with silence?
		evtTail.deltat = deltat;
	}

	/// Start the track.
	/// We reset the list pointer and deltatime.
	int Start(SMFFile *p)
	{
		smf = p;
		evtLast = evtHead.next;
		if (evtLast == &evtTail)
		{
			eot = true;
			deltaTime = 0;
			return 0;
		}
		deltaTime = evtLast->deltat;
		eot = false;
		return 1;
	}

	/// Generate checks for the next MIDI event and produces
	/// the appropriate BasicSynth sequencer event. This 
	/// function simply passes the event back to the SMFFile object
	/// for processing.
	int Generate();

	MIDIEvent *Enum(MIDIEvent *evt)
	{
		if (evt == 0)
			evt = &evtHead;
		evt = evt->next;
		if (evt == &evtTail)
			return 0;
		return evt;
	}
};

/// MIDI header chunk
struct MTHDchunk 
{
	//bsUint8  chnkID[4]; // MThd
	bsUint32 chnkID;
	bsUint32 size;
	bsUint16 format;
	bsUint16 numTrk;
	bsUint16 tmDiv;
};

/// Instrument map. 
/// There is an array of these, one for each channel. The SMFFile
/// class uses this map to determine the BasicSynth instrument
/// that should handle noteon/noteoff for each channel. TO play
/// using a SoundBank (or similar setup), the "inc" member can
/// be the same for all channels. The bnkParam and preparam members
/// can be used to pass the current bank and preset numbers to the
/// instrument on a START event. However, it is also possible
/// for the instrument to get this information from a MIDIControl
/// object, if implemented.
struct SMFInstrMap
{
	InstrConfig *inc;
	bsInt16 bnkParam;
	bsInt16 preParam;
};

/// Channel status during event generation.
/// The BasicSynth sequencer uses an absolute time model instead
/// of the relative time model of SMF. We convert by "playing" the
/// MIDI event sequence, measuring the total time between note-on
/// and note-off events. On a note-on, the current sample time, key
/// and velocity are stored for the key number and the note is
/// marked as "on". The note-off event calculates the duration 
/// in samples and outputs the absolute time event. This object also
/// keeps track of all CC values and current pitchbend sensitivity.
class SMFChnlStatus
{
public:
	short noteison[128];
	short velocity[128];
	FrqValue noteOn[128];
	AmpValue volume;
	bsInt32 bank;
	bsInt32 patch;
	bsInt32 cc[128];
	bsInt32 count;

	SMFChnlStatus()
	{
		Clear();
	}

	void Clear()
	{
		for (int i = 0; i < 128; i++)
		{
			noteison[i] = 0;
			velocity[i] = 0;
			noteOn[i] = 0.0;
			cc[i] = 0;
		}
		count = 0;
		bank = 0;
		patch = 0;
		volume = 1.0;
	}
};

/// Processor for a Standard MIDI format file (SMF a/k/a .mid).
/// Processing a file is a two step process. The file is first
/// read into memory and stored as a list of tracks. Each track
/// maintains a list of raw MIDI events for the track. If desired,
/// that information can be used to edit the MIDI file. To play
/// using the BasicSynthesizer, invoke the Generate method, passing
/// in the sequencer object and an instrument map. Generate converts
/// the MIDI events into absolute time events and merges Note ON/OFF
/// into a single START event with duration.
/// The gmbank flag indicates we are doing General MIDI 1 or 2. This affects
/// the processing of the bank/preset number. GM 1 always uses bank 0 or 128.
class SMFFile
{
protected:
	FileReadBuf fp;
	MTHDchunk hdr;
	bsString metaText;
	bsString metaCpyr;
	bsString metaSeqName;
	bsString timeSig;
	bsString keySig;
	bsUint8 *inpBuf;
	bsUint8 *inpPos;
	bsUint8 *inpEnd;
	bsUint16 lastMsg;
	bsUint32 deltaT;
	double srTicks;
	double ppqn;
	FrqValue theTick;
	SMFTrack *trackObj;
	SynthEnumList<SMFTrack> trackList;
	SMFChnlStatus chnlStatus[16];
	SMFInstrMap *instrMap;
	Sequencer *seq;
	SoundBank *sbnk;
	int gmbank;
	int explNoteOff;

	/// Process META messages.
	void MetaEvent();
	/// Process System common messages.
	void SysCommon(bsUint16 msg);
	/// Process channel messages.
	void ChnlMessage(bsUint16 msg);
	
	/// Read a string and store as a bsString.
	void ReadString(bsString& str, bsUint32 len)
	{
		while (len-- > 0)
			str += *inpPos++;
		str += "\r\n";
	}
	
	/// Read a 4-byte value in a portable manner.
	bsUint32 ReadLong()
	{
		return ((bsUint32) ReadShort() << 16) + (bsUint32) ReadShort();
	}

	/// Read a 2-byte value in a portable manner.
	bsUint16 ReadShort()
	{
		return (bsUint16) (fp.ReadCh() << 8) + fp.ReadCh();
	}

	/// Read a variable length value.
	/// MIDI files have values larger than 127 stored as
	/// a series of 7-bit values. We have to catenate them
	/// into a long. This function works off of the in-memory
	/// chunk data.
	bsUint32 GetVarLen()
	{
		bsUint32 value = 0;
		do
			value = (value << 7) + (bsUint32) (*inpPos & 0x7F);
		while (*inpPos++ & 0x80);
		return value;
	}

	/// Format an integer into a bsString
	void IntToStr(bsString& str, int val)
	{
		if (val >= 10)
			IntToStr(str, val/10);
		str += (char) ((val % 10) + '0');
	}

	// Close the input file.
	void CloseFile()
	{
		fp.FileClose();
		if (inpBuf)
			delete inpBuf;
		inpBuf = 0;
		inpPos = 0;
		inpEnd = 0;
	}

public:
	bsInt16 timeSigNum;
	bsInt16 timeSigDiv;
	bsInt16 timeSigBeat;
	bsInt16 keySigKey;
	bsInt16 keySigMaj;

	SMFFile()
	{
		Reset();
	}

	~SMFFile()
	{
	}

	void Reset()
	{
		trackList.Clear();
		hdr.format = 1;
		hdr.numTrk = 0;
		hdr.tmDiv = 24;
		lastMsg = 0;
		inpBuf = 0;
		inpPos = 0;
		inpEnd = 0;
		ppqn = 24.0e6;
		srTicks = (0.5 * synthParams.sampleRate) / 24.0;
		trackObj = 0;
		instrMap = 0;
		seq = 0;
		gmbank = 1;
		sbnk = 0;
		chnlStatus[9].bank = 128;
		metaText = "";
		metaCpyr = "";
		metaSeqName = "";
		timeSig = "";
		keySig = "";
		keySigKey = 0;
		keySigMaj = 0;
		timeSigNum = 4;
		timeSigDiv = 4;
		timeSigBeat = 60;
		explNoteOff = 0;
	}

	/// Set GM level.
	/// By default, we use GM=1, and all instruments
	/// are on bank 0 for all channels except 10, which is
	/// bank 128. Setting this flag 0 will allow exact
	/// specification of bank number from the SMF file.
	/// Setting it to 2 works for GM2 where the bank
	/// select MSB is required to be 121 or 120.
	inline void GMBank(int val)
	{
		gmbank = val;
	}

	/// Emit note off events.
	/// By default, we generate one event per note.
	/// To emulate MIDI NoteOn/NoteOff, we can emit 
	/// two separate events for note start and note stop.
	/// @param onoff if zero, emit two events for each note.
	inline void ExplicitNoteOff(int onoff)
	{
		explNoteOff = onoff;
	}

	/// Return the meta text records.
	const char *MetaText()
	{
		return metaText;
	}

	/// Return the copyright record.
	const char *Copyright()
	{
		return metaCpyr;
	}

	/// Return the sequence name record.
	const char *SeqName()
	{
		return metaSeqName;
	}

	/// Return the formatted time signature.
	const char *TimeSignature()
	{
		return timeSig;
	}

	/// Return the formatted key signature.
	const char *KeySignature()
	{
		return keySig;
	}

	/// Generate sequencer event.
	/// These are the generate functions and should
	/// only be called by the Track object or internally.
	/// They are public so that SMFTrack can get at them.
	//@{
	void NoteOn(short chnl, short key, short vel, short track = 0);
	void NoteOff(short chnl, short key, short vel, short track = 0);
	void SetTempo(long val);
	void ProgChange(short chnl, short val, short track = 0);
	void ControlChange(short chnl, short ctl, short val, short track = 0);
	void KeyAfterTouch(short chnl, short key, short val, short track = 0);
	void ChannelAfterTouch(short chnl, short val, short track = 0);
	void PitchBend(short chnl, short val, short track = 0);
	void AddControlEvent(short mmsg, short chnl, short ctl, short val, short track = 0);
	//@}

	/// Load a SMF file.
	int LoadFile(const char *file);

	/// Generate sequencer events from the SMF file.
	int GenerateSeq(Sequencer *seq, SMFInstrMap *map, SoundBank *sb = 0);

	/// Determine how many notes are on each channel.
	/// This is only valid after calling GenerateSeq.
	/// @returns number of channels found with notes.
	int GetChannelMap(bsInt32 *ch);

	SMFTrack *Enum(SMFTrack *trk)
	{
		return trackList.EnumItem(trk);
	}

};


//@}
#endif
