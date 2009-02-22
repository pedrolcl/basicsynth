//////////////////////////////////////////////////////////////////
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
					if (evt->im)
						act->ip = instMgr.Allocate(evt->im);
					else
						act->ip = instMgr.Allocate(evt->inum);
					if (act->ip == 0)
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
