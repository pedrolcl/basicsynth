//////////////////////////////////////////////////////////////////
// BasicSynth
//
// Sequencer code that is not inline
//
// Copyright 2008, Daniel R. Mitchell
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

// Add the event to the sequence sorted by start time.
// The caller is responsible for setting valid values 
// for inum, start, duration, type and eventid.
void Sequencer::AddEvent(SeqEvent *evt)
{
	if (evt == 0)
		return;

	//printf("Add Event %d at time %d\n", evt->evid, evt->start);
	if (evtLast == NULL)
		evtLast = evtHead;
	SeqEvent *inspt;
	if (evt->start >= evtLast->start)
	{
		do
		{
			inspt = evtLast;
			evtLast = evtLast->next;
		} while (evt->start >= evtLast->start);
		inspt->Insert(evt);
		evtLast = evt;
	}
	else
	{
		do
		{
			inspt = evtLast;
			evtLast = evtLast->prev;
		} while (evt->start < evtLast->start);
		evtLast->Insert(evt);
		evtLast = evt;
	}
	bsInt32 e = evt->start + evt->duration;
	if (e >= seqLength)
		seqLength = e+1;
}

// The sequencer loop. 

bsInt32 Sequencer::Sequence(InstrManager& instMgr, bsInt32 startTime, bsInt32 endTime)
{
	ActiveEvent *act;
	ActiveEvent *actHead = new ActiveEvent;
	ActiveEvent *actTail = new ActiveEvent;
	actHead->ip = NULL;
	actTail->ip = NULL;
	actHead->evid = -1;
	actTail->evid = -2;
	actHead->ison = false;
	actTail->ison = false;
	actHead->Insert(actTail);

	instMgr.Start();
	tickCount = 0;
	wrapCount = 0;
	if (tickCB)
		tickCB(0, tickArg);

	int actCount;
	int actWait;
	int startit;
	bsInt16 typ;

	SeqEvent *evt = evtHead->next;
	if (startTime > 0)
	{
		while (evt != evtTail && evt->start < startTime)
			evt = evt->next;
	}

	playing = true;
	while (playing)
	{
		// find any events that are ready to activate
		while (evt != evtTail)
		{
			if (evt->start <= startTime)
			{
				if ((typ = evt->type) != SEQEVT_START)
				{
					startit = (typ == SEQEVT_RESTART);
					// try to match this event to a prior event
					for (act = actHead->next; act != actTail; act = act->next)
					{
						if (act->evid == evt->evid)
						{
							startit = 0;
							if (typ == SEQEVT_PARAM)
								act->ip->Param(evt);
							else if (typ == SEQEVT_STOP)
								act->count = 0;
							else if (typ == SEQEVT_RESTART)
							{
								startit = false;
								act->ip->Start(evt);
								act->count = evt->duration;
								act->ison = true;
							}
							break;
						}
					}
				}
				else
					startit = true;

				if (startit)
				{
					// Start an instrument. The instrument manager must
					// locate the instrument by id (inum) and return
					// a valid instance. We then initialize the instrument
					// by passing the event structure. 
					if ((act = new ActiveEvent) == NULL)
					{
						playing = false;
						break;
					}
					actTail->InsertBefore(act);
					act->count = evt->duration;
					act->evid = evt->evid;
					act->ison = true;

					// assume: allocate should not fail, even if inum is invalid...
					if ((act->ip = instMgr.Allocate(evt->inum)) == NULL)
					{
						// ...except if we are out of memory, so give up now.
						playing = false;
						break;
					}
					act->ip->Start(evt);
				}
				evt = evt->next;
			}
			else
				break;
		}
		// Cycle all active events (Tick)
		// This is "IT" - where we actually generate samples...
		actCount = 0;
		actWait = 0;
		act = actHead->next;
		while (act != actTail)
		{
			if (act->ison)
			{
				if (act->count-- == 0)
				{
					act->ip->Stop();
					act->ison = false;
					//printf("Stop Note for event %d\n", act->evid);
				}
				act->ip->Tick();
				act = act->next;
				actCount++;
			}
			else
			{
				if (act->ip->IsFinished())
				{
					//printf("Remove Note for event %d\n", act->evid);
					instMgr.Deallocate(act->ip);
					ActiveEvent *p = act->Remove();
					delete act;
					act = p;
				}
				else
				{
					actWait++;
					act->ip->Tick();
					act = act->next;
				}
			}
		}
		instMgr.Tick();
		startTime++;
		if (tickCB)
		{
			if (++tickCount >= tickWrap)
			{
				tickCB(++wrapCount, tickArg);
				tickCount = 0;
			}
		}

		// When we have reached the end of the sequence
		// AND all events have finished, 
		// OR, we have hit the last time caller wanted,
		// we can quit. N.B. this can leave active events!
		if ((evt == evtTail && actHead->next == actTail)
		 || (endTime > 0 && startTime >= endTime))
			playing = false;
	}
	instMgr.Stop();

	// Since it is possible to halt the sequence while events are still
	// active, we do clean-up here. We don't bother with Stop or IsFinished
	// since we are no longer generating samples.
	while ((act = actHead->next) != actTail)
	{
		instMgr.Deallocate(act->ip);
		act->Remove();
		delete act;
	}
	
	delete actTail;
	delete actHead;

	return startTime; // in case the caller wants to know how long we played...
}
