//////////////////////////////////////////////////////////////////////
// BasicSynth WaveFile playback instrument
//
// See _BasicSynth_ Chapter 23 for a full explanation
//
// This instrument plays a wave file. The sound is loaded into
// a cache and can be shared by multiple instrument instances.
// The sound can be played once or looped, and a simple AR 
// envelope can be applied to fade in/out.
//
// Copyright 2008, Daniel R. Mitchell
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "WFSynth.h"

static WaveFileIn wfCache[WFSYNTH_MAX_WAVEFILES];
static int wfCacheCount = 0;
static AmpValue dummy;

int WFSynth::GetCacheCount()
{
	return wfCacheCount;
}

WaveFileIn *WFSynth::GetCacheEntry(int n)
{
	return &wfCache[n];
}

void WFSynth::ClearCache()
{
	int n;
	for (n = 0; n < wfCacheCount; n++)
		wfCache[n].Clear();
	wfCacheCount = 0;
}

int WFSynth::AddToCache(const char *filename, bsInt16 id)
{
	int n;
	for (n = 0; n < wfCacheCount; n++)
	{
		const char *wfname = wfCache[n].GetFilename();
		if (wfname && strcmp(filename, wfname) == 0
		 && wfCache[n].GetFileID() == id)
		{
			return n;
		}
	}
	if (wfCacheCount >= WFSYNTH_MAX_WAVEFILES)
		return -1;
	if (wfCache[n].LoadWaveFile(filename, id) != 0)
		return -1;
	wfCacheCount++;
	return n;
}

Instrument *WFSynth::WFSynthFactory(InstrManager *m, Opaque tmplt)
{
	WFSynth *ip = new WFSynth;
	ip->im = m;
	if (tmplt)
		ip->Copy((WFSynth *) tmplt);
	return ip;
}

SeqEvent *WFSynth::WFSynthEventFactory(Opaque tmplt)
{
	WFSynthEvent *wp = new WFSynthEvent;
	if (tmplt)
	{
		WFSynth *ip = (WFSynth *) tmplt;
		wp->id = ip->fileID;
		wp->lp = ip->looping;
		wp->pa = ip->playAll;
		wp->ar = ip->eg.GetAtkRt();
		wp->rr = ip->eg.GetRelRt();
	}
	else
	{
		wp->id = 0;
		wp->lp = 0;
		wp->pa = 0;
		wp->ar = 0.0;
		wp->rr = 0.0;
	}
	return (SeqEvent *) wp;
}

void WFSynthEvent::SetParam(bsInt16 idx, float v)
{
	switch (idx)
	{
	case 16:
		id = (bsInt16) v;
		break;
	case 17:
		lp = (bsInt16) v;
		break;
	case 18:
		pa = (bsInt16) v;
		break;
	case 19:
		ar = FrqValue(v);
		break;
	case 20:
		rr = FrqValue(v);
		break;
	default:
		NoteEvent::SetParam(idx, v);
		break;
	}
}

WFSynth::WFSynth()
{
	im = NULL;
	samples = &dummy;
	sampleNumber = 0;
	sampleTotal = 0;
	looping = 0;
	playAll = 0;
	fileID = -1;
	eg.SetAtkRt(0.0);
	eg.SetRelRt(0.0);
	eg.SetSus(1.0);
	eg.SetSusOn(1);
	memset(wfUsed, 0, sizeof(wfUsed));
}

WFSynth::~WFSynth()
{
}

void WFSynth::Copy(WFSynth *tp)
{
	fileID = tp->fileID;
	sampleTotal = tp->sampleTotal;
	sampleNumber = tp->sampleNumber;
	samples = tp->samples;
	looping = tp->looping;
	playAll = tp->playAll;
	eg.Copy(&tp->eg);
}

void WFSynth::Start(SeqEvent *evt)
{
	SetParams((WFSynthEvent*)evt);

	samples = &dummy;
	sampleNumber = 0;
	sampleTotal = 0;

	WaveFileIn *wfp = &wfCache[0];
	WaveFileIn *wfe = &wfCache[WFSYNTH_MAX_WAVEFILES];
	while (wfp < wfe)
	{
		if (wfp->GetFileID() == fileID)
		{
			samples = wfp->GetSampleBuffer();
			sampleTotal = wfp->GetInputLength();
			break;
		}
		wfp++;
	}
	eg.Reset(0);
}

void WFSynth::Param(SeqEvent *evt)
{
	SetParams((WFSynthEvent*)evt);
}

void WFSynth::SetParams(WFSynthEvent *evt)
{
	// pitch is not currently used. An improvement to this
	// instrument is to somehow vary the sound based on the
	// pitch and/or frequency.
	chnl = evt->chnl;
	fileID = evt->id;
	looping = evt->lp;
	playAll = evt->pa;
	eg.SetAtkRt(evt->ar);
	eg.SetRelRt(evt->rr);
	eg.SetSus(evt->vol);
}

void WFSynth::Stop()
{
	eg.Release();
	if (!looping && !playAll)
		sampleNumber = sampleTotal;
}

void WFSynth::Tick()
{
	if (sampleNumber >= sampleTotal)
	{
		if (!looping)
			return;
		sampleNumber = 0;
	}
	im->Output(chnl, samples[sampleNumber++] * eg.Gen());
}

int  WFSynth::IsFinished()
{
	if (looping && sampleTotal > 0)
		return eg.IsFinished();
	return sampleNumber >= sampleTotal;
}

void WFSynth::Destroy()
{
	delete this;
}

int WFSynth::Load(XmlSynthElem *parent)
{
	float atk;
	float rel;
	short ival;

	memset(wfUsed, 0, sizeof(wfUsed));

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("wvf"))
		{
			if (elem->GetAttribute("fn", ival) == 0)
				fileID = (bsInt16) ival;
			if (elem->GetAttribute("lp", ival) == 0)
				looping = (bsInt16) ival;
			if (elem->GetAttribute("pa", ival) == 0)
				playAll = (bsInt16) ival;
		}
		else if (elem->TagMatch("env"))
		{
			elem->GetAttribute("ar", atk);
			elem->GetAttribute("rr", rel);
			eg.InitAR(atk, 1.0, rel, 1, linSeg);
		}
		else if (elem->TagMatch("file"))
		{
			char *filename = 0;
			if (elem->GetAttribute("name", &filename) == 0)
			{
				if (elem->GetAttribute("id", ival) == 0)
				{
					ival = AddToCache(filename, (bsInt16) ival);
					if (ival >= 0)
						wfUsed[ival] = 1;
				}
				delete filename;
			}
		}

		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int WFSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem;

	elem = parent->AddChild("wvf");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("fn", (long) fileID);
	elem->SetAttribute("lp", (short) looping);
	elem->SetAttribute("pa", (short) playAll);
	delete elem;

	elem = parent->AddChild("env");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("ar", eg.GetAtkRt());
	elem->SetAttribute("rr", eg.GetRelRt());
	delete elem;

	for (int n = 0; n < WFSYNTH_MAX_WAVEFILES; n++)
	{
		short id = (short) wfCache[n].GetFileID();
		if (wfUsed[id])
		{
			elem = parent->AddChild("file");
			if (elem == NULL)
				return -1;
			elem->SetAttribute("name", wfCache[n].GetFilename());
			elem->SetAttribute("id", id);
			delete elem;
		}
	}
	return 0;
}
