//////////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file Sequencer.h Play a sequence of events.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{
#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_

/// Sequencer tick callback function.
/// If set, the callback will be called every 
/// "wrap" ticks. This allows the program to display
/// the current sequencer time value, or do other
/// things to inform the user what is going on.
/// The callback should be short and not attempt
/// to manipulate either the instrument manager
/// or sequencer other than possibly telling the
/// sequencer to halt playback. The count value
/// indicates how many times the count has wrapped.
/// @sa Sequencer::SetCB
/// @param count count of how many "wraps" have occurred.
/// @param usr user supplied argument.
typedef void (*SeqTickCB)(bsInt32 count, Opaque usr);

///////////////////////////////////////////////////////////
/// Active note sequencer event
//
/// Active events are used internally by the sequencer
/// to keep track of currently sounding notes. The event is
/// created when the note starts and continues until the
/// instrument indicates it is finished.
///
/// Changes for ver 1.2: 
/// The isOn member now represents the event state.
/// The spare member is now flags and holds event options.
/// The event state transitions: OFF -> ON -> REL -> OFF
/// A transtion from OFF to ON sends a START signal to the
/// instrument. A transition from ON to REL sends the STOP signal.
/// The TM flag indicates whether or not to count down the duration.
/// For live keyboard input and externally controlled sequences, 
/// the TM bit is clear and the count is not used.
/// The caller must enque a subsequent STOP event to move the active
/// event into the release cycle. For internally timed sequence events, 
/// the TM bit is set and the transition to REL happens automatically
/// when the count reaches zero.
/// Events are stored in multiple track objects with track 0
/// holding the main sequence. Track 0 is started by default
/// when Sequence() is called. Tracks 1-n can only be started
/// by events on track 0 or by immediate events.
///////////////////////////////////////////////////////////
#define SEQ_AE_OFF  0 // event is inactive
#define SEQ_AE_ON   1 // indicates we are counting the duration
#define SEQ_AE_REL  2 // indicates we are waiting on IsFinished
#define SEQ_AE_TM   1 // indicates 'count' is valid
#define SEQ_AE_KEEP 2 // keep the active event for possible restart (not currently used)

struct ActiveEvent : public SynthList<ActiveEvent>
{
	Instrument *ip;
	bsInt32 count;  ///< number of samples left to play (duration)
	bsInt32 evid;   ///< id of the event that activated this event
	bsInt16 ison;   ///< SEQ_AE_REL after stop is sent to the instrument
	bsInt16 flags;  ///< event options
	bsInt16 chnl;   ///< output channel
};

///////////////////////////////////////////////////////////
/// Sequencer Control handler class
///
/// This interface defines the handler for SEQEVT_CONTROL events.
/// This is an interface definition. The actual control handler
/// must provide an implementation for the two methods.
//////////////////////////////////////////////////////////
class SeqControl
{
public:
	/// Process a control event.
	/// This is called on SEQEVT_CONTROL events. If flags is 0, this is an
	/// immediate event, otherwise a sequenced event. 
	virtual void ProcessEvent(SeqEvent *evt, bsInt16 flags) = 0;
	/// Called on each sample.
	/// This method allows the controller to count ticks,
	/// or provide per-sample processing.
	virtual void Tick() = 0;
};

/// SeqState defines the sequencer state.
/// These are combinations of bit-flags that control sequencer operations:
/// 0x01 - sequence (track events are executed)
/// 0x02 - play (immediate events are executed)
/// 0x04 - once (sequencer exits when all tracks finished)
/// 0x08 - paused (waits for signal to continue)
typedef int SeqState;
#define seqOff 0
#define seqSequence 0x01
#define seqPlay     0x02
#define seqOnce     0x04
#define seqPaused   0x08
#define seqPlaySeq  (seqSequence|seqPlay)
#define seqSeqOnce  (seqSequence|seqOnce)
#define seqPlaySeqOnce (seqSequence|seqPlay|seqOnce)

class SeqTrack : public SynthList<SeqTrack>
{
protected:
	SeqEvent *evtHead;
	SeqEvent *evtTail;
	SeqEvent *evtLast;
	SeqEvent *evtPlay;
	bsInt32 loopCount;
	bsInt32 startTime;
	bsInt32 seqLength;
	bsInt16 enable;
	bsInt16 trkNum;

public:
	SeqTrack(int tn = 0)
	{
		trkNum = tn;
		enable = 0;
		loopCount = 0;
		startTime = 0;
		seqLength = 0;
		evtHead = new SeqEvent;
		evtTail = new SeqEvent;
		//evtHead->Insert(evtTail);
		evtHead->next = evtTail;
		evtHead->prev = evtTail;
		evtTail->next = evtHead;
		evtTail->prev = evtHead;
		evtLast = evtHead;
		evtHead->start = 0;
		evtTail->start = 0x7FFFFFFFL;
		evtHead->evid = -1;
		evtTail->evid = -2;
		evtPlay = evtTail;
	}

	~SeqTrack()
	{
		Reset();
		delete evtHead;
		delete evtTail;
	}

	inline bsInt32 GetLength() { return seqLength; }
	inline bsInt16 Track() { return trkNum; }
	inline bsInt16 Enable() { return enable; }
	inline bsInt16 Enable(bsInt16 e) { return enable = e; }
	inline bsInt32 LoopCount(bsInt32 c) { return loopCount = c; }

	/// Start the track.
	/// @param st start time relative to start of track (usually 0)
	void Start(bsInt32 st)
	{
		enable = 1;
		startTime = st;
		evtPlay = evtHead->next;
		while (evtPlay != evtTail && evtPlay->start < st)
			evtPlay = evtPlay->next;
	}

	/// Stop the track.
	inline void Stop() { enable = 0; }

	/// Continue the track.
	inline void Continue() { enable = 1; }

	/// Determine if the track is playing or not.
	int Tick()
	{
		if (enable)
		{
			if (++startTime >= seqLength)
			{
				// At the end - see if we should loop or quit
				if (--loopCount == 0)
					enable = 0;
				else
				{
					startTime = 0;
					evtPlay = evtHead->next;
				}
			}
		}
		return enable;
	}

	/// Get the next ready event.
	inline SeqEvent *NextEvent()
	{
		if (enable && evtPlay->start <= startTime)
		{
			SeqEvent *evt = evtPlay;
			evtPlay = evt->next;
			return evt;
		}
		return 0;
	}

	void Reset();
	void AddEvent(SeqEvent *evt);
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
protected:
	bsInt32 globEventID;
	SeqTrack *track;
	bool playing;
	bool pausing;
	bsInt32 seqLength;
	bsUint32 seqTick;
	SeqTickCB tickCB;
	bsInt32 tickCount;
	bsInt32 tickWrap;
	Opaque  tickArg;
	bsInt32 wrapCount;

	ActiveEvent *actHead;
	ActiveEvent *actTail;

	// v 1.2 - add immediate events
	SeqEvent *immHead;
	SeqEvent *immTail;
	void *critMutex;
	void *pauseSignal;

	InstrManager* instMgr;
	SeqControl *cntrlMgr;

	SeqState state;

	void CreateMutex();
	void DestroyMutex();
	inline void EnterCritical();
	inline void LeaveCritical();
	void Sleep();
	void Wakeup();

	virtual void ProcessEvent(SeqEvent *evt, bsInt16 flags);
	virtual int Tick();
	virtual void Wait();

	void ClearActive();

public:
	Sequencer();
	virtual ~Sequencer();

	/// Get the next event ID.
	/// Values with the MSB set are immediate events.
	bsInt32 NextEventID() { return ++globEventID & 0x7FFFFFFF; }

	/// Reset the sequencer.
	/// Reset should be called to clean up any memory before filling in a new sequence. 
	virtual void Reset();

	/// Get the length of the current sequence. 
	/// The length of the sequence is in samples
	/// and is an estimate since an instrument can
	/// continue playing after the configured duration.
	virtual bsInt32 GetLength()
	{
		return track->GetLength();
	}

	/// Get the number of tracks.
	virtual int GetTrackCount()
	{
		int count = 0;
		SeqTrack *tp = track;
		while (tp)
		{
			count++;
			tp = tp->next;
		}
		return count;
	}

	/// Set the tick callback function. 
	/// @param cb callback function
	/// @param wrap number of ticks between callbacks
	/// @param arg caller supplied data
	virtual void SetCB(SeqTickCB cb, bsInt32 wrap, Opaque arg)
	{
		tickCB = cb;
		tickWrap = wrap;
		tickArg = arg;
	}

	/// Set the control event handler.
	/// If this is not set, control events are ignored.
	virtual void SetController(SeqControl *c)
	{
		cntrlMgr = c;
	}

	/// Get the sequencer state.
	/// The sequencer can be in one of the following states:
	/// - seqOff = sequencer is not active
	/// - seqSequence = sequencer is playing sequences continuously
	/// - seqPlay = sequencer is in playing immediate events continuously
	/// - seqPlaySeq = sequencer is playing sequences and imm. events continously
	/// - seqSeqOnce = sequencer will exit when all tracks finish
	/// - seqPlaySeqOnce = sequencer will exit when all tracks finish and play imm. events
	/// - seqPause = sequencer is paused
	virtual SeqState GetState()
	{
		return state;
	}

	/// Halt the sequencer.
	/// Halt can be called to stop any further sequencing.
	/// This would have to be done by an instrument manager,
	/// another thread, or a callback function.
	virtual void Halt();

	/// Pause sequencing without exit.
	/// Halt or Resume will end the pause state. The sequencer may not
	/// enter the paused state immediately. After calling Pause, 
	/// poll the sequencer state until it returns seqPause, or set
	/// an event callback and wait for SEQEVT_SEQPAUSE.
	virtual void Pause()
	{
		pausing = true;
	}

	/// Resume after pause.
	virtual void Resume()
	{
		if (pausing)
		{
			pausing = false;
			Wakeup();
		}
	}

	/// Add the event to the sequence. 
	/// The event is added to the track indicated in the track
	/// value, sorted by start time.
	/// The caller is responsible for setting
	/// valid values for inum, start, duration, type, track and eventid.
	/// Timed events should only be added when the sequencer is stopped.
	/// @param evt the event to schedule
	virtual void AddEvent(SeqEvent *evt);

	/// Add the event for immediate playback.
	/// The caller is responsible for setting
	/// valid values for inum, type and event id where appropriate.
	/// @param evt the event to schedule
	virtual void AddImmediate(SeqEvent *evt);

	/// The sequencer loop. 
	///
	/// When invoked, this method enters into a loop processing events.
	/// The instrument manager is used to allocate and deallocate
	/// instrument instances. It is also the responsibility of the
	/// instrument manager to push samples out on every tick.
	///
	/// The optional start and end times can be used to play a portion
	/// of the sequence. These times are in samples.
	/// The state argument indicates if immediate and/or scheduled events
	/// will be played. See GetState() for more information.
	/// @param im instrument manager
	/// @param startTime if non-zero, start at the indicated sample
	/// @param endTime if non-zero, stop at the indicated sample
	/// @param st sequencer mode of operation
	/// @return number of samples output
	virtual bsUint32 SequenceMulti(InstrManager& im, bsUint32 startTime = 0, bsUint32 endTime = 0, SeqState st = seqSeqOnce);

	/// Sequence Optimally.
	/// Similar to SequenceMulti() but does not check for live events or play multiple tracks.
	/// This is optimal for auditioning sequences and creating wave file output when
	/// only one master track exists.
	/// @param im instrument manager
	/// @param startTime if non-zero, start at the indicated sample
	/// @param endTime if non-zero, stop at the indicated sample
	/// @return number of samples output
	virtual bsUint32 Sequence(InstrManager& im, bsUint32 startTime = 0, bsUint32 endTime = 0);

	/// Play live optimally.
	/// When invoked, this method enters into a loop waiting on immediate
	/// events. Whenever an event appears on the immNext list, it is removed
	/// and executed immediately. This function is used for live playback of
	/// notes received from a keyboard. This is an optimized version
	/// of Sequence. It is equivalent to:
	/// @code
	///	Sequence(im, 0, 0, seqPlay)
	/// @endcode
	/// but does not check track event lists or make callbacks.
	/// Start and Stop track events have no effect. This function only
	/// returns when Halt() is invoked.
	/// @param im reference to an instrument manager object
	/// @return number of samples output
	virtual bsUint32 Play(InstrManager& im);
};

/// Sequencer event callback function.
/// If set, the callback will be called when certain
/// events are processed, and when the sequencer state
/// changes. The callback should be short and not attempt
/// to manipulate either the instrument manager
/// or sequencer other than possibly telling the
/// sequencer to halt playback.
/// @param evtID the event type
/// @param evt the sequencer event or null for internal events
/// @param usr user supplied argument
typedef void (*SeqEventCB)(bsUint32 tick, bsInt16 evtID, const SeqEvent *evt, Opaque usr);

/// SequencerCB adds event callbacks to the Sequencer class.
class SequencerCB : public Sequencer
{
protected:
	SeqEventCB evtCB;
	Opaque     evtArg;

	virtual void ProcessEvent(SeqEvent *evt, bsInt16 flags);
	virtual void Wait();

public:
	SequencerCB()
	{
		evtCB = 0;
		evtArg = 0;
	}

	/// Set the event callback function. 
	/// @param cb callback function
	/// @param arg caller supplied data
	virtual void SetEventCB(SeqEventCB cb, Opaque arg)
	{
		evtCB = cb;
		evtArg = arg;
	}

	virtual void Notify(bsInt16 id, SeqEvent *evt)
	{
		if (evtCB)
			evtCB(seqTick, id, evt, evtArg);
	}

	virtual bsUint32 SequenceMulti(InstrManager& im, bsUint32 startTime = 0, bsUint32 endTime = 0, SeqState st = seqSeqOnce);
	virtual bsUint32 Sequence(InstrManager& im, bsUint32 startTime = 0, bsUint32 endTime = 0);
	virtual bsUint32 Play(InstrManager& im);
};

//@}
#endif
