/////////////////////////////////////////////////////////////////////////////
// BasicSynth - Player object
//
// See Player.h for an explanation...
//
/////////////////////////////////////////////////////////////////////////////
#include <windows.h>
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
#include <Player.h>

void Player::AddEvent(SeqEvent *evt)
{
	if (evt == 0)
		return;

	EnterCriticalSection(&guard);
	evtTail->InsertBefore(evt);
	LeaveCriticalSection(&guard);
}

void Player::Play(InstrManager& instMgr)
{
	bsInt16 typ;
	SeqEvent *evt;
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
	playing = true;
	while (playing)
	{
		// in-line message peek/get
		EnterCriticalSection(&guard);
		evt = evtHead->next;
		if (evt != evtTail)
			evt->Remove();
		else
			evt = NULL;
		LeaveCriticalSection(&guard);
		if (evt)
		{
			//OutputDebugString("Got event...\r\n");

			if ((typ = evt->type) != SEQEVT_START)
			{
				// try to match this event to a prior event
				for (act = actHead->next; act != actTail; act = act->next)
				{
					if (act->evid == evt->evid)
					{
						if (typ == SEQEVT_PARAM)
						{
							//OutputDebugString("Changing...\r\n");
							act->ip->Param(evt);
						}
						else if (typ == SEQEVT_STOP)
						{
							//OutputDebugString("Stopping ...\r\n");
							act->ip->Stop();
							act->ison = false;
						}
						break;
					}
				}
			}
			else
			{
				//OutputDebugString("Starting...\r\n");
				// Start an instrument. The instrument manager must
				// eturn a valid instance. We then initialize the instrument
				// by passing the event structure. 
				if ((act = new ActiveEvent) != NULL)
				{
					if (evt->im)
						act->ip = instMgr.Allocate(evt->im);
					else
						act->ip = instMgr.Allocate(evt->inum);
					if (act->ip)
					{
						actTail->InsertBefore(act);
						act->count = evt->duration;
						act->evid = evt->evid;
						act->ison = true;
						act->ip->Start(evt);
					}
					else
						delete act;
				}
			}
			delete evt;
		}

		// Cycle all active events (Tick)
		// This is "IT" - where we actually generate samples...
		act = actHead->next;
		while (act != actTail)
		{
			act->ip->Tick();
			if (!act->ison && act->ip->IsFinished())
			{
				instMgr.Deallocate(act->ip);
				ActiveEvent *p = act->Remove();
				delete act;
				act = p;
			}
			else
				act = act->next;
		}
		instMgr.Tick();
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
}
