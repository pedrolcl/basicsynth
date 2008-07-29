//////////////////////////////////////////////////////////////////
// BasicSynth
//
// Sequencer class
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////

#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_


///////////////////////////////////////////////////////////
// Active Event class
//
// Active events are used internally by the sequencer
// to keep track of currently sounding notes. The event is
// created when the note starts and continues until the
// instrument indicates it is finished.
///////////////////////////////////////////////////////////
struct ActiveEvent : public SynthList<ActiveEvent>
{
	Instrument *ip;
	bsInt32 count;  // number of samples left to play (duration)
	bsInt32 evid;   // id of the event that activated this event
	bsInt16 ison;   // true after the duration is finished
	bsInt16 spare;  // force 32-bit alignment, remove if not applicable
};


///////////////////////////////////////////////////////////
// Sequencer class
//
// The sequencer class maintains two lists. The first list
// contains the events to sequence and must be built by 
// calling AddEvent. The sequence list is persistent until
// Reset() is called and may be stopped and restarted.
// The second list contains all active notes and exists only
// while the sequencer is playing.
///////////////////////////////////////////////////////////
class Sequencer
{
private:
	SeqEvent *evtHead;
	SeqEvent *evtTail;
	SeqEvent *evtLast;
	bool playing;
	bsInt32 seqLength;
	void (*tickCB)(bsInt32 count);
	bsInt32 tickCount;
	bsInt32 tickWrap;
	bsInt32 wrapCount;

public:
	Sequencer()
	{ 
		evtHead = new SeqEvent;
		evtTail = new SeqEvent;
		evtHead->Insert(evtTail);
		evtLast = evtHead;
		evtHead->start = 0;
		evtTail->start = 0x7FFFFFFFL;
		evtHead->evid = -1;
		evtTail->evid = -2;
		playing = false;
		seqLength = 0;
		tickCB = 0;
		tickCount = 0;
		tickWrap = 0;
		wrapCount = 0;
	}

	~Sequencer()
	{
		Reset();
		evtHead->Destroy();
		evtTail->Destroy();
	}

	// Reset should be called to clean up any memory
	// before filling in a new sequence. 
	void Reset()
	{
		while ((evtLast = evtHead->next) != evtTail)
		{
			evtLast->Remove();
			evtLast->Destroy();
		}
		evtLast = evtHead;
		seqLength = 0;
	}

	// returns the length of the sequence in samples;
	// this is an estimate since an instrument can
	// continue playing after the specified duration.
	bsInt32 GetLength()
	{
		return seqLength;
	}

	// Set the callback function. This is optional.
	// If set, the callback will be called every 
	// "wrap" ticks. This allows the program to display
	// the current sequencer time value, or do other
	// things to inform the user what is going on.
	// The callback should be short and not attempt
	// to manipulate either the instrument manager
	// or sequencer other than possibly telling the
	// sequencer to halt playback.
	virtual void SetCB(void (*f)(bsInt32), bsInt32 wrap)
	{
		tickCB = f;
		tickWrap = wrap;
	}


	// Halt can be called to stop any further sequencing.
	// This would have to be done by an instrument manager,
	// another thread, or a callback function.
	void Halt()
	{
		playing = false;
	}

	// Add the event to the sequence. The caller is responsible for setting
	// valid values for inum, start, duration, type and eventid.
	void AddEvent(SeqEvent *evt);

	// The sequencer loop. 
	bsInt32 Sequence(InstrManager& instMgr, bsInt32 startTime = 0, bsInt32 endTime = 0);
};
#endif
