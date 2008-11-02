//////////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file Sequencer.h Play a sequence of events.
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{
#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_


///////////////////////////////////////////////////////////
/// Active note sequencer event
//
/// Active events are used internally by the sequencer
/// to keep track of currently sounding notes. The event is
/// created when the note starts and continues until the
/// instrument indicates it is finished.
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
/// Sequencer class.
///
/// The sequencer processes a list of time-ordered
/// events, allocating and invoking "instruments" as needed. The
/// implementation of instruments, signal generators, and sample
/// output is opaque to the sequencer. Consequently, the same
/// sequencer algorithm can be used for a variety of synthesizers.
///
/// The sequencer class maintains two lists. The first list
/// contains the events to sequence and must be built by 
/// calling AddEvent(). The sequence list is persistent until
/// Reset() is called and may be stopped and restarted.
/// The second list contains all active notes and exists only
/// while the sequencer is playing.
///////////////////////////////////////////////////////////
class Sequencer
{
private:
	SeqEvent *evtHead;
	SeqEvent *evtTail;
	SeqEvent *evtLast;
	bool playing;
	bsInt32 seqLength;
	void (*tickCB)(bsInt32 count, Opaque arg);
	bsInt32 tickCount;
	bsInt32 tickWrap;
	Opaque  tickArg;
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
		tickArg = 0;
		wrapCount = 0;
	}

	~Sequencer()
	{
		Reset();
		evtHead->Destroy();
		evtTail->Destroy();
	}

	/// Reset the sequencer.
	/// Reset should be called to clean up any memory before filling in a new sequence. 
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

	/// Get the length of the current sequence. 
	/// The length of the sequence is in samples
	/// and is an estimate since an instrument can
	/// continue playing after the configured duration.
	bsInt32 GetLength()
	{
		return seqLength;
	}

	/// Set the callback function. This is optional.
	/// If set, the callback will be called every 
	/// "wrap" ticks. This allows the program to display
	/// the current sequencer time value, or do other
	/// things to inform the user what is going on.
	/// The callback should be short and not attempt
	/// to manipulate either the instrument manager
	/// or sequencer other than possibly telling the
	/// sequencer to halt playback.
	/// @param f callback function
	/// @param wrap number of ticks between callbacks
	/// @param arg caller supplied data
	virtual void SetCB(void (*f)(bsInt32, Opaque), bsInt32 wrap, Opaque arg)
	{
		tickCB = f;
		tickWrap = wrap;
		tickArg = arg;
	}


	/// Halt the sequencer.
	/// Halt can be called to stop any further sequencing.
	/// This would have to be done by an instrument manager,
	/// another thread, or a callback function.
	void Halt()
	{
		playing = false;
	}

	/// Add the event to the sequence. The caller is responsible for setting
	/// valid values for inum, start, duration, type and eventid.
	void AddEvent(SeqEvent *evt);

	/// The sequencer loop. 
	///
	/// When invoked, this method enters into a loop processing events.
	/// The instrument manager is used to allocated and deallocate
	/// instrument instances. It is also the responsibility of the
	/// instrument manager to push samples out on every tick.
	///
	/// The optional start and end times can be used to play a portion
	/// of the sequence. These times are in samples.
	/// @param instMgr instrument manager
	/// @param startTime if non-zero, start at the indicated time
	/// @param endTime if non-zero, stop at the indicated time
	bsInt32 Sequence(InstrManager& instMgr, bsInt32 startTime = 0, bsInt32 endTime = 0);
};
//@}
#endif
