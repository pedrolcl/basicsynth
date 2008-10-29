/////////////////////////////////////////////////////////////////////////////
// BasicSynth - Player object
//
// This class manages immediate playback of sounds. It waits for
// a start event, then allocates an instrument and adds the instrument
// to the list of active sounds. The instrument remains active until
// a corresponding stop event is received. When no instruments are
// active, the Player will continue to call the instrument manager
// which must output zeros to the DAC buffer.
//
// To play a sound, allocate a sequencer event and call the AddEvent
// method from another thread.
//
// Timing of sample output is under control of the instrument manager.
// In a typical setup, the instrument manager sends samples to
// a wave output buffer. The wave output buffer will synchronize
// sample output to the sample rate by blocking on the sound card
// driver output routine.
//
// At the present time, this is Windows only code and should be
// runs as a separate thread. New events are appended to the event list
// by one thread and removed by the playback thread. In short, this is
// a very simple message queue between threads. That's what
// the critical section is for - to synchronize access to the
// event list. To port to another platform, you need to implement
// either the critical section routines, or use an async message queue
// in place of directly adding to the event object list.
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////////////
#ifndef _PLAYER_H_
#define _PLAYER_H_

#pragma once

/// Immediate event playback. Events are removed from
/// the event list as soon as they are discovered. For
/// every start event, a matching stop event is required.
/// The event id field of the SeqEvent object is used to
/// match events.
class Player
{
private:
	SeqEvent *evtHead;
	SeqEvent *evtTail;
	SeqEvent *evtLast;
	bool playing;
	CRITICAL_SECTION guard;

public:
	Player()
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
		InitializeCriticalSection(&guard);
	}

	~Player()
	{
		Reset();
		evtHead->Destroy();
		evtTail->Destroy();
		DeleteCriticalSection(&guard);
	}

	/// Reset should be called to clean up any old
	/// events before playback is started. Usually, 
	/// there is nothing in the list.
	void Reset()
	{
		while ((evtLast = evtHead->next) != evtTail)
		{
			evtLast->Remove();
			evtLast->Destroy();
		}
		evtLast = evtHead;
	}

	/// Halt is called to stop any further sequencing.
	void Halt()
	{
		playing = false;
	}

	/// Add the event to the sequence. The caller is responsible for setting
	/// valid values for inum, start, duration, type and eventid.
	void AddEvent(SeqEvent *evt);

	/// The playback loop. 
	void Play(InstrManager& instMgr);
};
#endif
