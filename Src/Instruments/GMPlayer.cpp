//////////////////////////////////////////////////////////////////////
/// @file GMPlayer.cpp Implementation of the General MIDI player
//
// This instrument provides an emulation of a MIDI keyboard using
/// SoundFont GM file.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "GMPlayer.h"

MIDIControl *GMManager::midiCtrl;

/// Create an instance of the GMPlayer instrument
Instrument *GMManager::InstrFactory(InstrManager *m, Opaque tmplt)
{
	GMPlayer *ip = 0;
	GMManager *gm = (GMManager *)tmplt;
	if (gm)
	{
		ip = new GMPlayer(gm, m);
		gm->instrTail.InsertBefore(ip);
	}
	return ip;
}

Opaque GMManager::TmpltFactory(XmlSynthElem *tmplt)
{
	GMManager *gm = new GMManager;
	if (tmplt)
		gm->Load(tmplt);
	return gm;
}

void GMManager::TmpltDump(Opaque tmplt)
{
	GMManager *gm = (GMManager *)tmplt;
	if (gm)
		delete gm;
}

/// Create an event object for GMManager
SeqEvent *GMManager::EventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 19;
	return (SeqEvent*)ep;
}

GMManager::GMManager()
{
	sndbnk = 0;
	im = 0;
	instrHead.Insert(&instrTail);
	localVals = 0;
}

GMManager::~GMManager()
{
	GMPlayer *ip;
	while ((ip = instrHead.next) != &instrTail)
		ip->Destroy();
}

/// Allocate a variable parameters object (needed for editor)
VarParamEvent *GMManager::AllocParams()
{
	return (VarParamEvent*) EventFactory(0);
}

void GMManager::SetSoundFile(const char *b)
{
	sndFile = b;
	SetSoundBank(SoundBank::FindBank(b));
}

void GMManager::SetSoundBank(SoundBank *b)
{
	sndbnk = b;
	GMPlayer *ip;
	for (ip = instrHead.next; ip != &instrTail; ip = ip->next)
		ip->SetSoundBank(b);
}

void GMManager::SetMidiControl(MIDIControl *mc)
{
	midiCtrl = mc;
	GMPlayer *ip;
	for (ip = instrHead.next; ip != &instrTail; ip = ip->next)
		ip->SetMidiControl(mc);
}

void GMManager::SetLocalPan(bsInt16 lp)
{
	if (lp)
		localVals |= GMM_LOCAL_PAN;
	else
		localVals &= ~GMM_LOCAL_PAN;
	GMPlayer *ip;
	for (ip = instrHead.next; ip != &instrTail; ip = ip->next)
		ip->SetLocalPan(lp);
}

/////////////////////////////////////////////////////////////////

int GMManager::Load(XmlSynthElem *parent)
{
	char *cval;

	XmlSynthElem elem;
	XmlSynthElem *next = parent->FirstChild(&elem);
	while (next != NULL)
	{
		if (elem.TagMatch("gm"))
		{
			long lv = 0;
			elem.GetAttribute("local", lv);
			localVals = lv;
			elem.GetAttribute("vol", volValue);
			elem.GetAttribute("bank", bankValue);
			elem.GetAttribute("prog", progValue);
			elem.GetContent(&cval);
			if (cval)
				sndFile.Attach(cval);
		}
		next = elem.NextSibling(&elem);
	}
	if (sndFile.Length() > 0)
		sndbnk = SoundBank::FindBank(sndFile);

	return 0;
}

int GMManager::Save(XmlSynthElem *parent)
{
	XmlSynthElem elem;

	if (!parent->AddChild("gm", &elem))
		return -1;

	elem.SetAttribute("local", (long) localVals);
	elem.SetAttribute("vol",  (float) volValue);
	elem.SetAttribute("bank", bankValue);
	elem.SetAttribute("prog", progValue);
	elem.SetContent(sndFile);

	return 0;
}

int GMManager::SetParams(VarParamEvent *params)
{
	int err = 0;
	bsInt16 *id = params->idParam;
	float *valp = params->valParam;
	int n = params->numParam;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int GMManager::SetParam(bsInt16 idval, float val)
{
	switch (idval)
	{
	case 16:
		localVals = (bsInt32) val;
		break;
	case 17:
		volValue = AmpValue(val);
		break;
	case 18:
		bankValue = (bsInt16) val;
		break;
	case 19:
		progValue = (bsInt16) val;
		break;
	default:
		return 1;
	}
	return 0;
}

int GMManager::GetParams(VarParamEvent *params)
{
	params->SetParam(16, (float) localVals);
	params->SetParam(17, (float) volValue);
	params->SetParam(18, (float) bankValue);
	params->SetParam(19, (float) progValue);
	return 0;
}

int GMManager::GetParam(bsInt16 idval, float *val)
{
	switch (idval)
	{
	case 16:
		*val = (float) localVals;
		break;
	case 17:
		*val = volValue;
		break;
	case 18:
		*val = (float) bankValue;
		break;
	case 19:
		*val = (float) progValue;
		break;
	default:
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////

static InstrParamMap gmManagerParams[] = 
{
	{"bank", 18 },
	{"local", 16 }, 
	{"prog", 19 },
	{"volume", 17 }
};

bsInt16 GMManager::MapParamID(const char *name, Opaque tmplt)
{
	return SearchParamID(name, gmManagerParams, sizeof(gmManagerParams)/sizeof(InstrParamMap));
}

const char *GMManager::MapParamName(bsInt16 id, Opaque tmplt)
{
	return SearchParamName(id, gmManagerParams, sizeof(gmManagerParams)/sizeof(InstrParamMap));
}

/////////////////////////////////////////////////////////////////////

GMPlayer::GMPlayer()
{
	im = 0;
	gm = 0;
	midiCtrl = 0;
	sndbnk = 0;
	localPan = 1;
	Reset();
}

GMPlayer::GMPlayer(GMManager *g, InstrManager *m)
{
	im = m;
	gm = g;
	sndbnk = gm->GetSoundBank();
	localPan = gm->GetLocalPan();
	Reset();
}

void GMPlayer::Reset()
{
	preset = 0;
	zonel = 0;
	zoner = 0;
	chnl = 0;
	mkey = 69;
	novel = 0;
	frq = 440.0;
	volEnv.InitEnv(0, 0);
}

GMPlayer::~GMPlayer()
{
}

/// Start playing a note.
/// Locate the preset if needed, then the zone(s).
/// Initialize oscillators and envelopes.
void GMPlayer::Start(SeqEvent *se)
{
	VarParamEvent *evt = (VarParamEvent *)se;
	chnl = evt->chnl;
	frq = evt->frq;
	mkey = evt->pitch + 12;
	novel = evt->noteonvel;
	if (novel == 0)
		novel = 127;
	vol = evt->vol;

	zoner = 0;
	zonel = 0;

	sostenuto = gm->GetCC(chnl, 67) > 64;
	preset = sndbnk->GetInstr(gm->GetBank(chnl), gm->GetPatch(chnl));
	if (preset)
	{
		FrqValue oscFrq = frq;
		if (gm)
			oscFrq *= gm->GetPitchbend(chnl);
		zoner = preset->GetZone(mkey, novel, 0);
		if (zoner)
		{
			oscr.InitSF(oscFrq, zoner);
			panr.Set(panSqr, zoner->pan);
			vol *= zoner->volAtten ;//* preset->velVolume[novel];
			volEnv.InitEnv(preset, &zoner->volEg, mkey, novel);
			//modEnv.InitEnv(preset, &zoner->modEG, mkey, novel);
			AmpValue vibamt = zoner->vibAmount;
			if (vibamt != 0.0)
			{
				viblfo.InitLFO(zoner->vibRate, WT_SIN, 
					zoner->vibDelay, frq * vibamt, 0);
			}
			else
				viblfo.SetLevel(0.0);
		}
		zonel = preset->GetZone(mkey, novel, 1);
		if (zonel)
		{
			if (zoner->sample == 0)
			{
				SBSample *samp = sndbnk->GetSample(zoner->sampleNdx, 1);
				zoner->sample = samp->sample;
			}
			oscl.InitSF(oscFrq, zonel);
			panl.Set(panSqr, zonel->pan);
		}
	}
	volEnv.Reset(0);
	modEnv.Reset(0);
}

void GMPlayer::Param(SeqEvent *se)
{
	// variable params are set indirectly through the MIDI controller object
	// We handle a pitch change here so that the virtual keyboard works,
	// and treat it like a note-off immediately followed by a note-on,
	// iow - do a "START" with the new information...
	VarParamEvent *evt = (VarParamEvent *)se;
	int nkey = evt->pitch + 12;
	if (nkey != mkey)
		Start(se);
}

void GMPlayer::Stop()
{
	oscr.Release();
	oscl.Release();
	volEnv.Release();
}

/// Produce the next sample.
void GMPlayer::Tick()
{
	FrqValue frqVal = frq * gm->GetPitchbend(chnl);
	AmpValue ampVal = vol * volEnv.Gen() * gm->GetVolume(chnl);
	int modFrq = 0;

	if (viblfo.On())
		frqVal += viblfo.Gen();

	AmpValue oscValR = 0.0;
	AmpValue oscValL = 0.0;
	if (zoner)
	{
		oscl.UpdateFrequency(frqVal);
		oscValR = ampVal * oscr.Gen();
	}
	if (zonel)
	{
		oscl.UpdateFrequency(frqVal);
		oscValL = ampVal * oscl.Gen();
	}
	if (localPan)
	{
		im->Output2(chnl,
			(oscValR * panr.panlft) + (oscValL * panl.panlft),
			(oscValR * panr.panrgt) + (oscValL * panl.panrgt));
	}
	else
		im->Output(chnl, (oscValR + oscValL) / 2.0);
}

int  GMPlayer::IsFinished()
{
	if (gm->GetCC(chnl, 64) > 64) // sustain on
		return 0;
	if (!sostenuto && gm->GetCC(chnl, 67) > 64)
		return 0;
	return volEnv.IsFinished() || (zoner && oscr.IsFinished());
}

void GMPlayer::Destroy()
{
	Remove();
	delete this;
}

