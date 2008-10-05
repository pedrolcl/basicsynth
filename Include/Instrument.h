///////////////////////////////////////////////////////////
// BasicSynth
//
// Instrument and Instrument manager classes.
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////
#ifndef _INSTRDEF_
#define _INSTRDEF_

class InstrManager; // forward reference for type defs below

///////////////////////////////////////////////////////////
// Instrument - base class for instruments. This class defines
// the common methods needed by the instrument manager and
// sequencer. It also functions as a dummy instrument that
// can be allocated in place of invalid instrument numbers.
//
// Start - method called when the current playback time
//         matches the event start time. The instrument
//         uses the event structure to get the per-note
//         parameters for note initialization.
// Param - used to alter parameters while playing.
// Stop  - method called when the duration has completed.
//         The instrument can stop immediately, or begin
//         the release phase of its envelope if appropriate.
//         The instrument remains active until IsFinished
//         returns true.
// Tick  - called to generate the current sample. Output
//         is done through the instrument manager, set
//         by the InstrFactory function
// IsFinished - called for each sample after Stop has been sent
//         to determine if the instrument can be removed from
//         the active list. 
// Load  - called to load the instrument configuration from
//         an XML file
// Save  - called to save the instrument configuration to
//         an XML file
///////////////////////////////////////////////////////////
class Instrument
{
public:
	virtual ~Instrument() { }
	virtual void Start(SeqEvent *evt) { }
	virtual void Param(SeqEvent *evt) { }
	virtual void Stop() { }
	virtual void Tick() { }
	virtual int  IsFinished() { return 1; }
	virtual void Destroy() { delete this; }
	virtual int Load(XmlSynthElem *parent) { return -1; }
	virtual int Save(XmlSynthElem *parent) { return -1; }
};

///////////////////////////////////////////////////////////
// The InstrFactory is a static method or non-class function
// used to instantiate the instrument. The template parameter
// contains default settings for the instrument and is
// opaque to the sequencer and instrument manager.
// Typically, only this function should create new instances
// of instruments. There are exceptions, of course.
///////////////////////////////////////////////////////////
typedef Instrument *(*InstrFactory)(InstrManager *, Opaque tmplt);

///////////////////////////////////////////////////////////
// The EventFactory is a static method or non-class function
// used to instantiate an event specific to an instrument.
///////////////////////////////////////////////////////////
typedef SeqEvent   *(*EventFactory)(Opaque tmplt);

///////////////////////////////////////////////////////////
// InstrMapEntry class is used by the instrument manager
// to manage one type of instrument, and, a specific 
// configuration of an instrument. 
///////////////////////////////////////////////////////////

class InstrMapEntry : public SynthList<InstrMapEntry>
{
public:
	bsString itype;
	bsString iname;
	bsString idesc;
	bsInt16 inum;
	InstrFactory manufInstr;
	EventFactory manufEvent;
	Opaque instrTemplate;

	InstrMapEntry(bsInt16 ino, InstrFactory in, EventFactory ev, Opaque tmplt = 0)
	{
		inum = ino;
		manufInstr = in;
		manufEvent = ev;
		instrTemplate = tmplt;
	}

	const char *GetType()
	{
		return itype;
	}

	void SetType(const char *str)
	{
		itype = str;
	}

	const char *GetName()
	{
		return iname;
	}

	void SetName(const char *str)
	{
		iname = str;
	}

	const char *GetDesc()
	{
		return idesc;
	}

	void SetDesc(const char *str)
	{
		idesc = str;
	}
};

///////////////////////////////////////////////////////////
// Instrument manager class
//
// This class maintains lists of instrument types and 
// pre-configured instruments. It is called by the sequencer
// to allocate a new instance of the instrument when a note
// is started, and to deallocate an instance when the note
// is finished. 
// Before playback is started, the manager must be initialized
// with mixer and output buffer objects. 
// The instance of the instrument manager is passed to each
// instrument instance. Instruments output samples through
// a method on the instrument manager, not directly to the
// mixer or output device.
///////////////////////////////////////////////////////////
class InstrManager
{
private:
	InstrMapEntry *instList;
	InstrMapEntry *typeList;
	Mixer *mix;
	WaveOutBuf *wvf;
	int internalID;

public:
	InstrManager()
	{
		instList = 0;
		typeList = 0;
		mix = 0;
		wvf = 0;
		internalID = 32768;
	}

	virtual ~InstrManager() { }

	// Init MUST be called first. If you forget, 
	// things will go very wrong very quickly !
	virtual void Init(Mixer *m, WaveOutBuf *w)
	{
		mix =  m;
		wvf = w;
	}

	// Add an entry to the instrument manager type list.
	virtual InstrMapEntry *AddType(const char *type, InstrFactory in, EventFactory ev)
	{
		InstrMapEntry *ent = new InstrMapEntry(-1, in, ev, 0);
		ent->SetType(type);
		if (typeList)
			typeList->Insert(ent);
		else
			typeList = ent;
		return ent;
	}

	// Find the entry for a specific type
	virtual InstrMapEntry *FindType(const char *type)
	{
		InstrMapEntry *ent;
		for (ent = typeList; ent; ent = ent->next)
		{
			if (ent->itype.Compare(type) == 0)
				break;
		}
		return ent;
	}

	// Add a new instrument.
	// The "tmplt" argument is stored in the instrument entry
	// and passed to the instrument instance during construction.
	// Typically the "tmplt" is an instance of the instrument
	// that should be used to initialize a copy for playback,
	// but can be any data the instrument needs to use for
	// initialization.
	// Instrument numbers must be unique. If a duplicate is found,
	// the instrument number is adjusted. Callers should check the
	// return object if specific instrument numbers are needed.
	// In addition, the caller should set the name on the returned
	// object if it is desired to locate instruments by name.
	virtual InstrMapEntry* AddInstrument(bsInt16 inum, InstrMapEntry *type, Opaque tmplt = 0)
	{
		return AddInstrument(inum, type->manufInstr, type->manufEvent, tmplt);
	}

	virtual InstrMapEntry* AddInstrument(bsInt16 inum, InstrFactory in, EventFactory ev, Opaque tmplt = 0)
	{
		if (FindInstr(inum))
			inum = internalID++;
		InstrMapEntry *ent = new InstrMapEntry(inum, in, ev, tmplt);
		if (instList)
			instList->Insert(ent);
		else
			instList = ent;
		return ent;
	}

	// Find instrument by number
	virtual InstrMapEntry *FindInstr(bsInt16 inum)
	{
		InstrMapEntry *in;
		for (in = instList; in != NULL; in = in->next)
		{
			if (in->inum == inum)
				break;
		}
		return in;
	}

	// Find instrument by name
	virtual InstrMapEntry *FindInstr(char *iname)
	{
		InstrMapEntry *in;
		for (in = instList; in != NULL; in = in->next)
		{
			if (in->iname.CompareNC(iname) == 0)
				break;
		}
		return in;
	}

	// Allocate an instance of an instrument for playback.
	virtual Instrument *Allocate(bsInt16 inum)
	{
		InstrMapEntry *in = FindInstr(inum);
		if (in)
			return in->manufInstr(this, in->instrTemplate);
		return new Instrument;
	}

	virtual void Deallocate(Instrument *ip)
	{
		ip->Destroy();
	}

	// Allocate a new event for an instrument 
	virtual SeqEvent *ManufEvent(bsInt16 inum)
	{
		InstrMapEntry *in = FindInstr(inum);
		if (in)
			return in->manufEvent(in->instrTemplate);
		return new NoteEvent;
	}

	// Start is called by the sequencer when the sequence starts.
	virtual void Start()
	{
		// clear the mixer
		if (mix)
			mix->Reset();
	}
	
	// Stop is called by the sequencer when the sequence stops.
	virtual void Stop()
	{
	}

	// Tick is called by the sequencer on each sample. This is
	// the point where all active instruments have produced a
	// value for the current sample. We can now output the sample
	// to the output buffer. Notice that this only supports 
	// two-channel output. However, the buffer can be set for
	// single channel and merge the two outputs together. For
	// more that two channels, derive a class from this one
	// and override Tick.
	virtual void Tick()
	{
		AmpValue outLft, outRgt;
		mix->Out(&outLft, &outRgt);
		wvf->Output2(outLft, outRgt);
	}

	// Direct output to effects units - this bypasses the
	// normal input channel volume, pan, and fx send values
	virtual void FxSend(int unit, AmpValue val)
	{
		mix->FxIn(unit, val);
	}

	// Output a sample on the indicated channel
	virtual void Output(int ch, AmpValue val)
	{
		mix->ChannelIn(ch, val);
	}

	// Output a left/right sample on the indicated channel,
	// This is used by instruments that have internal
	// panning capability as it will bypass the mixer 
	// panning functions.
	virtual void Output2(int ch, AmpValue lft, AmpValue rgt)
	{
		mix->ChannelIn2(ch, lft, rgt);
	}
};

#endif
