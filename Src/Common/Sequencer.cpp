//////////////////////////////////////////////////////////////////
/// @file Sequencer.cpp Implementation of the sequencer.
//
// BasicSynth
//
// Sequencer code that is not inline
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
#include <WaveTable.h>
#include <WaveFile.h>
#include <Mixer.h>
#include <SynthList.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <Instrument.h>
#include <Sequencer.h>

//////////////////////////// TRACK ////////////////////////////

void SeqTrack::Reset()
{
	SeqEvent *evt;
	while ((evt = evtHead->next) != evtTail)
	{
		evt->Remove();
		evt->Destroy();
	}
	evtLast = evtHead;
	seqLength = 0;
}

void SeqTrack::AddEvent(SeqEvent *evt)
{
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

//////////////////////////// SEQUENCER ////////////////////////////

Sequencer::Sequencer()
{ 
	state = seqOff;
	playing = false;
	pausing = false;
	seqLength = 0;
	tickCB = 0;
	tickCount = 0;
	tickWrap = 0;
	tickArg = 0;
	wrapCount = 0;
	cntrlMgr = 0;
	instMgr = 0;

	track = new SeqTrack(0);

	immHead = new SeqEvent;
	immTail = new SeqEvent;
	immHead->Insert(immTail);
	immHead->start = 0;
	immTail->start = 0x7FFFFFFFL;
	immHead->evid = -1;
	immTail->evid = -2;

	actHead = new ActiveEvent;
	actTail = new ActiveEvent;
	actHead->ip = NULL;
	actTail->ip = NULL;
	actHead->evid = -1;
	actTail->evid = -2;
	actHead->ison = SEQ_AE_OFF;
	actTail->ison = SEQ_AE_OFF;
	actHead->flags = SEQ_AE_KEEP;
	actTail->flags = SEQ_AE_KEEP;
	actHead->Insert(actTail);

	CreateMutex();
}

Sequencer::~Sequencer()
{
	Reset();
	immHead->Destroy();
	immTail->Destroy();
	delete actHead;
	delete actTail;
	delete track;
	DestroyMutex();
}

// Add the event to the sequence sorted by start time.
// The caller is responsible for setting valid values 
// for inum, start, duration, type and eventid.
void Sequencer::AddEvent(SeqEvent *evt)
{
	if (evt == 0)
		return;

	SeqTrack *pp = 0;
	SeqTrack *tp = track;
	while (tp != 0)
	{
		if (tp->Track() == evt->track)
			break;
		pp = tp;
		tp = tp->next;
	}
	if (!tp)
	{
		tp = new SeqTrack(evt->track);
		if (pp)
			pp->Insert(tp);
		else
			track = tp;
	}
	tp->AddEvent(evt);
}


void Sequencer::AddImmediate(SeqEvent *evt)
{
	if (evt == 0)
		return;

	EnterCritical();
	immTail->InsertBefore(evt);
	LeaveCritical();
}

// Multi-mode sequencer can play live, sequence, loop tracks or any combination.
bsInt32 Sequencer::SequenceMulti(InstrManager& im, bsInt32 startTime, bsInt32 endTime, SeqState st)
{
	if (state != seqOff)
		return 0;

	if (track == 0)
		return 0;

	SeqEvent *imm = 0;
	SeqEvent *evt = 0;
	SeqTrack *tp = 0;
	int trkActive = 0;
	int evtActive = 0;

	track->LoopCount(1);
	track->Start(startTime);

	instMgr = &im;
	instMgr->Start();

	tickCount = 0;
	wrapCount = 0;
	if (tickCB)
		tickCB(0, tickArg);

	state = st;
	int live = st & seqPlay;
	int sequenced = st & seqSequence;
	int once = st & seqOnce;

	playing = true;
	while (playing)
	{
		if (live)
		{
			// in-line message peek/get
			EnterCritical();
			imm = immHead->next;
			if (imm != immTail)
				imm->Remove();
			else
				imm = NULL;
			LeaveCritical();
			if (imm)
			{
				ProcessEvent(imm, 0);
				imm->Destroy();
			}
		}

		if (sequenced)
		{
			// find any events that are ready to activate
			trkActive = 0;
			for (tp = track; tp; tp = tp->next)
			{
				while ((evt = tp->NextEvent()) != 0)
					ProcessEvent(evt, SEQ_AE_TM);
				trkActive |= tp->Tick();
			}
		}

		// invoke all active instruments for one sample
		evtActive = Tick();

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
		if (once)
		{
			if ((!trkActive && !evtActive)
			  || (endTime > 0 && startTime >= endTime))
				playing = false;
		}
	}
	instMgr->Stop();

	// Since it is possible to halt the sequence while events are still
	// active, we do clean-up here. We don't bother with Stop or IsFinished
	// since we are no longer generating samples.
	ClearActive();
	
	state = seqOff;
	if (tickCB)
		tickCB(wrapCount, tickArg);

	return startTime; // in case the caller wants to know how long we played...
}

/// Optimal sequencing - no live events are checked and loop tracks are not played.
bsInt32 Sequencer::Sequence(InstrManager& im, bsInt32 startTime, bsInt32 endTime)
{
	if (state != seqOff)
		return 0;

	if (track == 0)
		return 0;

	SeqEvent *evt = 0;
	int trkActive = 0;
	int evtActive = 0;

	track->LoopCount(1);
	track->Start(startTime);

	instMgr = &im;
	instMgr->Start();

	tickCount = 0;
	wrapCount = 0;
	if (tickCB)
		tickCB(0, tickArg);

	state = seqSeqOnce;

	playing = true;
	while (playing)
	{
		// find any events that are ready to activate
		while ((evt = track->NextEvent()) != 0)
			ProcessEvent(evt, SEQ_AE_TM);
		trkActive = track->Tick();

		// invoke all active instruments for one sample
		evtActive = Tick();

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
		if ((!trkActive && !evtActive)
		  || (endTime > 0 && startTime >= endTime))
			playing = false;
	}
	instMgr->Stop();

	// Since it is possible to halt the sequence while events are still
	// active, we do clean-up here. We don't bother with Stop or IsFinished
	// since we are no longer generating samples.
	ClearActive();
	if (tickCB)
		tickCB(wrapCount, tickArg);

	state = seqOff;

	return startTime; // in case the caller wants to know how long we played...
}

/// Optimal live playback - no tracks are checked, only immediate input.
void Sequencer::Play(InstrManager& im)
{
	if (state != seqOff)
		return;

	instMgr = &im;

	SeqEvent *evt;

	ClearActive();

	EnterCritical();
	while ((evt = immHead->next) != immTail)
	{
		evt->Remove();
		evt->Destroy();
	}
	LeaveCritical();

	state = seqPlay;

	instMgr->Start();
	playing = true;
	while (playing)
	{
		// in-line message peek/get
		EnterCritical();
		evt = immHead->next;
		if (evt != immTail)
			evt->Remove();
		else
			evt = NULL;
		LeaveCritical();
		if (evt)
		{
			ProcessEvent(evt, 0);
			evt->Destroy();
		}

		Tick();
	}
	instMgr->Stop();

	ClearActive();

	state = seqOff;
}

/// Reset the sequencer.
/// Reset should be called to clean up any memory before filling in a new sequence. 
void Sequencer::Reset()
{
	SeqEvent *evt;
	ClearActive();

	EnterCritical();
	while ((evt = immHead->next) != immTail)
	{
		evt->Remove();
		evt->Destroy();
	}
	LeaveCritical();

	track->Reset();
	SeqTrack *tp;
	while ((tp = track->next) != 0)
	{
		tp->Remove();
		delete tp;
	}
	seqLength = 0;
}

// Since it is possible to halt the sequence while events are still
// active, we do clean-up here. We don't bother with Stop or IsFinished
// since we are no longer generating samples.
void Sequencer::ClearActive()
{
	ActiveEvent *act;

	while ((act = actHead->next) != actTail)
	{
		instMgr->Deallocate(act->ip);
		act->Remove();
		delete act;
	}
}

// Cycle all active events (Tick)
// This is "IT" - where we actually generate samples...
int Sequencer::Tick()
{
	if (pausing)
	{
		SeqState was = state;
		state = seqPaused;
		Sleep();
		state = was;
		if (!playing)
			return 0;
	}

	if (cntrlMgr)
		cntrlMgr->Tick();

	int actCount = 0;
	ActiveEvent *act = actHead->next;
	while (act != actTail)
	{
		actCount++;
		if (act->ison == SEQ_AE_ON)
		{
			if ((act->flags & SEQ_AE_TM) && act->count-- == 0)
			{
				act->ip->Stop();
				act->ison = SEQ_AE_REL;
				//printf("Stop Note for event %d\n", act->evid);
			}
			act->ip->Tick();
			act = act->next;
		}
		else if (act->ison == SEQ_AE_REL)
		{
			if (act->ip->IsFinished())
			{
				//printf("Remove Note for event %d\n", act->evid);
				instMgr->Deallocate(act->ip);
				ActiveEvent *p = act->Remove();
				delete act;
				act = p;
			}
			else
			{
				act->ip->Tick();
				act = act->next;
			}
		}
	}
	instMgr->Tick();
	return actCount;
}

void Sequencer::ProcessEvent(SeqEvent *evt, bsInt16 flags)
{
	bsInt16 typ = evt->type;
	ActiveEvent *act;
	SeqTrack *tp;
	TrackEvent *tevt;

	switch (typ)
	{
	case SEQEVT_RESTART:
	case SEQEVT_PARAM:
	case SEQEVT_STOP:
		// try to match this event to a prior event
		for (act = actHead->next; act != actTail; act = act->next)
		{
			if (act->evid == evt->evid)
			{
				if (typ == SEQEVT_PARAM)
					act->ip->Param(evt);
				else if (typ == SEQEVT_STOP)
				{
					act->ip->Stop();
					act->ison = SEQ_AE_REL;
				}
				else if (typ == SEQEVT_RESTART)
				{
					act->ip->Start(evt);
					act->count = evt->duration;
					act->ison = SEQ_AE_ON;
				}
				return;
			}
		}
		if (typ != SEQEVT_RESTART)
			return;
		/// FALTHROUGH on RESTART event no longer playing
	case SEQEVT_START:
		// Start an instrument. The instrument manager must
		// locate the instrument by id (inum) and return
		// a valid instance. We then initialize the instrument
		// by passing the event structure. 
		if ((act = new ActiveEvent) == NULL)
		{
			playing = false;
			return;
		}
		actTail->InsertBefore(act);
		act->count = evt->duration;
		act->evid = evt->evid;
		act->ison = SEQ_AE_ON;
		act->flags = flags;
		act->chnl = evt->chnl;

		// assume: allocate should not fail, even if inum is invalid...
		if (evt->im)
			act->ip = instMgr->Allocate(evt->im);
		else
			act->ip = instMgr->Allocate(evt->inum);
		if (act->ip != 0)
			act->ip->Start(evt);
		else	// ...except if we are out of memory, so give up now.
			playing = false;
		break;
	case SEQEVT_STARTTRACK:
		tevt = (TrackEvent *)evt;
		for (tp = track; tp; tp = tp->next)
		{
			if (tp->Track() == tevt->trkNo)
			{
				tp->LoopCount(tevt->loopCount);
				tp->Start(0);
				break;
			}
		}
		break;
	case SEQEVT_STOPTRACK:
		tevt = (TrackEvent *)evt;
		for (tp = track->next; tp; tp = tp->next)
		{
			if (tp->Track() == tevt->trkNo)
			{
				tp->Stop();
				break;
			}
		}
		break;
	case SEQEVT_CONTROL:
		if (cntrlMgr)
			cntrlMgr->ProcessEvent(evt, flags);
		break;
	}
}

void Sequencer::Halt()
{
	playing = false;
	if (pausing)
	{
		pausing = false;
		Wakeup();
	}
}


#if _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>

void Sequencer::CreateMutex()
{
	CRITICAL_SECTION *cs = new CRITICAL_SECTION;
	::InitializeCriticalSection(cs);
	critMutex = (void*)cs;
	pauseSignal = (void*)CreateEvent(NULL, FALSE, 0, NULL);
}

void Sequencer::DestroyMutex()
{
	CRITICAL_SECTION *cs = (CRITICAL_SECTION*)critMutex;
	if (cs)
	{
		::DeleteCriticalSection(cs);
		delete cs;
	}
	CloseHandle((HANDLE)pauseSignal);
	pauseSignal = 0;
}

inline void Sequencer::EnterCritical()
{
	::EnterCriticalSection((CRITICAL_SECTION*)critMutex);
}

inline void Sequencer::LeaveCritical()
{
	::LeaveCriticalSection((CRITICAL_SECTION*)critMutex);
}

inline void Sequencer::Sleep()
{
#ifdef _DEBUG
	OutputDebugString("Sequencer paused\r\n");
#endif
	::WaitForSingleObject((HANDLE)pauseSignal, (DWORD)-1);
#ifdef _DEBUG
	OutputDebugString("Sequencer wakeup\r\n");
#endif
}

inline void Sequencer::Wakeup()
{
	::SetEvent((HANDLE)pauseSignal);
}

#endif

#if UNIX
#include <sys/types.h>
#include <pthread.h>

struct pthread_event
{
	pthread_mutex_t m;
	pthread_cond_t  c;
};

void Sequencer::CreateMutex()
{
	pthread_mutex_t *cs = new pthread_mutex_t;
	pthread_mutex_init(cs, NULL);
	critMutex = (void*)cs;
	pthread_event *e = new pthread_event;
	pthread_mutex_init(&e->m, NULL);
	pthread_cond_init(&e->c, NULL);
	pauseSignal = (void*)e;
}

void Sequencer::DestroyMutex()
{
	pthread_mutex_t *cs = (pthread_mutex_t*)critMutex;
	if (cs)
	{
		pthread_mutex_destroy(cs);
		delete cs;
		critMutex = 0;
	}
	pthread_event *e = (pthread_event*)pauseSignal;
	if (e)
	{
		pthread_cond_destroy(&e->c);
		pthread_mutex_destroy(&e->m);
		delete e;
		pauseSignal = 0;
	}
}

inline void Sequencer::EnterCritical()
{
	pthread_mutex_lock((pthread_mutex_t*)critMutex);
}

inline void Sequencer::LeaveCritical()
{
	pthread_mutex_unlock((pthread_mutex_t*)critMutex);
}

void Sequencer::Sleep()
{
	pthread_event *e = (pthread_event*)pauseSignal;
	pthread_mutex_lock(&e->m);
    pthread_cond_wait(&e->c, &e->m);
	pthread_mutex_unlock(&e->m);
}

void Sequencer::Wakeup()
{
	pthread_event *e = (pthread_event*)pauseSignal;
	pthread_mutex_lock(&e->m);
	pthread_cond_signal(&e->c);
	pthread_mutex_unlock(&e->m);
}
#endif

// Load and instrument library from the file "fname"
// The file must be an XML file with a document node of "instrlib"
int InstrManager::LoadInstrLib(const char *fname)
{
	int err;
	XmlSynthDoc doc;
	XmlSynthElem *root = doc.Open((char*)fname);
	if (root != NULL)
		err = LoadInstrLib(root);
	else
		err = -1;
	doc.Close();
	return err;
}

// Load and instrument library from the XML document
// node "root". root must point to a node of type "instrlib"
int InstrManager::LoadInstrLib(XmlSynthElem *root)
{
	int err = 0;
	XmlSynthElem *instr = root->FirstChild();
	XmlSynthElem *next;
	while (instr != NULL)
	{
		if (instr->TagMatch("instr"))
		{
			if (LoadInstr(instr) == 0)
				err++;
		}
		else if (instr->TagMatch("wvtable"))
		{
			if (LoadWavetable(instr))
				err++;
		}
		next = instr->NextSibling();
		delete instr;
		instr = next;
	}
	return err;
}

// Load an instrument defintion from the XML node
// "instr" which must be of type <instr>.
InstrConfig *InstrManager::LoadInstr(XmlSynthElem *instr)
{
	if (instr == 0)
		return 0;

	InstrConfig *instEnt = 0;
	InstrMapEntry *instTyp = 0;

	instEnt = 0;
	long inum = 0;

	char *type = NULL;
	char *name = NULL;
	char *desc = NULL;
	if (instr->GetAttribute("type", &type) == 0)
	{
		Opaque tp;
		instTyp = FindType(type);
		if (instTyp)
		{
			if (instTyp->manufTmplt)
				tp = instTyp->manufTmplt(instr);
			else if (instTyp->manufInstr)
			{
				Instrument *ip = instTyp->manufInstr(this, 0);
				if (ip)
					ip->Load(instr);
				tp = (Opaque) ip;
			}
			else
				tp = 0;
			instr->GetAttribute("id", inum);
			instEnt = AddInstrument(inum, instTyp, tp);
			if (instEnt)
			{
				instr->GetAttribute("desc", &desc);
				instr->GetAttribute("name", &name);
				instEnt->SetName(name);
				instEnt->SetDesc(desc);
				delete name;
				delete desc;
			}
		}
		delete type;
	}
	return instEnt;
}

int InstrManager::LoadWavetable(XmlSynthElem *wvnode)
{
	short sumParts = 1;
	long wvID = -1;
	long wvNdx = -1;
	long wvParts = 0;
	long gibbs = 0;
	bsInt32 *mult;
	double *amps;
	double *phs;

	if (wvnode->GetAttribute("type", sumParts))
		sumParts = 1;
	if (wvnode->GetAttribute("parts", wvParts))
		return -1;
	if (wvParts <= 0)
		return -1;

	if (wvnode->GetAttribute("id", wvID) == 0)
	{
		wvNdx = wtSet.FindWavetable(wvID);
		if (wvNdx == -1)
		{
			wvNdx = wtSet.GetFreeWavetable(wvID);
			if (wvNdx == -1)
				wvNdx = wtSet.wavTblMax;
		}
	}
	else
	{
		if (wvnode->GetAttribute("ndx", wvNdx))
			return -1;
		wvID = wvNdx;
	}

	if (wvNdx >= wtSet.wavTblMax)
		wtSet.SetMax(wvNdx+4);
	wtSet.wavSet[wvNdx].wavID = wvID;

	wvnode->GetAttribute("gibbs", gibbs);
	mult = new bsInt32[wvParts];
	if (mult == 0)
		return -1;
	amps = new double[wvParts];
	if (amps == 0)
	{
		delete[] mult;
		return -1;
	}
	phs = new double[wvParts];
	if (phs == 0)
	{
		delete[] amps;
		delete[] mult;
		return -1;
	}
	long ptndx = 0;
	XmlSynthElem *ptnode = wvnode->FirstChild();
	XmlSynthElem *sib;
	while (ptnode && ptndx < wvParts)
	{
		if (ptnode->TagMatch("part"))
		{
			long m;
			ptnode->GetAttribute("mul", m);
			mult[ptndx] = (bsInt32) m;
			ptnode->GetAttribute("amp", amps[ptndx]);
			ptnode->GetAttribute("phs", phs[ptndx]);
			ptndx++;
		}
		sib = ptnode->NextSibling();
		delete ptnode;
		ptnode = sib;
	}

	if (sumParts == 1)
		wtSet.SetWaveTable(wvNdx, ptndx, mult, amps, phs, gibbs);
	else if (sumParts == 2)
		wtSet.SegWaveTable(wvNdx, ptndx, phs, amps);

	delete[] mult;
	delete[] amps;
	delete[] phs;

	return 0;
}
