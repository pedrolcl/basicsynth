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

/// Create an instance of the SFPlayer instrument
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
	ep->maxParam = 16;
	return (SeqEvent*)ep;
}

GMManager::GMManager()
{
	midiCtl = 0;
	sndbnk = 0;
	im = 0;
	instrHead.Insert(&instrTail);
	localPan = 1;
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
	SetSoundBank(SFSoundBank::FindBank(b));
}

void GMManager::SetSoundBank(SFSoundBank *b)
{
	sndbnk = b;
	GMPlayer *ip;
	for (ip = instrHead.next; ip != &instrTail; ip = ip->next)
		ip->SetSoundBank(b);
}

void GMManager::SetMidiControl(MIDIControl *mc)
{
	midiCtl = mc;
	GMPlayer *ip;
	for (ip = instrHead.next; ip != &instrTail; ip = ip->next)
		ip->SetMidiControl(mc);
}

void GMManager::SetLocalPan(bsInt16 lp)
{
	localPan = lp;
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
			elem.GetAttribute("locpan", localPan);
			elem.GetContent(&cval);
			if (cval)
				sndFile.Attach(cval);
		}
		next = elem.NextSibling(&elem);
	}
	if (sndFile.Length() > 0)
		sndbnk = SFSoundBank::FindBank(sndFile);

	return 0;
}

int GMManager::Save(XmlSynthElem *parent)
{
	XmlSynthElem elem;

	if (!parent->AddChild("gm", &elem))
		return -1;

	elem.SetAttribute("locpan", localPan);
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
		SetLocalPan((bsInt16) val);
		break;
	default:
		return 1;
	}
	return 0;
}

int GMManager::GetParams(VarParamEvent *params)
{
	params->SetParam(16, (float) localPan);
	return 0;
}

int GMManager::GetParam(bsInt16 idval, float *val)
{
	switch (idval)
	{
	case 16:
		*val = (bsInt16) localPan;
		break;
	default:
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////

static InstrParamMap gmManagerParams[] = 
{
	{"locpan", 17 }, 
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
	midiCtl = 0;
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

	// 5 segments: delay, attack, hold, decay, release
	volEnv.SetSegs(5);
	// default is a 'boxcar' envelope
	volEnv.SetStart(0.0);
	volEnv.SetSusOn(1);
	volEnv.SetRate(0, 0.0);
	volEnv.SetRate(1, 0.0);
	volEnv.SetRate(2, 0.0);
	volEnv.SetRate(3, 0.0);
	volEnv.SetRate(4, 0.0);
	volEnv.SetLevel(0, 0.0);
	volEnv.SetLevel(1, 1.0);
	volEnv.SetLevel(2, 1.0);
	volEnv.SetLevel(4, 0.0);
}

GMPlayer::~GMPlayer()
{
}

/// Start playing a note.
/// Locate the preset if needed, then the zone(s).
/// Initialize oscillators and envelopes.
void GMPlayer::Start(SeqEvent *se)
{
//	if (!gm || !sndbnk)
//		return;

	VarParamEvent *evt = (VarParamEvent *)se;
	chnl = evt->chnl;
	frq = evt->frq;
	vol = evt->vol;
	mkey = evt->pitch + 12;
	novel = evt->noteonvel;

	zoner = 0;
	zonel = 0;

	sostenuto = gm->GetCC(chnl, 67) > 64;
	preset = sndbnk->GetPreset(gm->GetBank(chnl), gm->GetPatch(chnl));
	if (preset)
	{
		FrqValue oscFrq = frq;
		if (gm)
			oscFrq *= gm->GetPitchbend(chnl);
		zoner = preset->GetSample(mkey, novel, 0);
		if (zoner)
		{
			oscr.InitSF(oscFrq, zoner);
			panr.Set(panSqr, zoner->pan);
			volEnv.SetRate(0, SFEnvRate(zoner->genVals[sfgDelayVolEnv]));
			volEnv.SetRate(1, SFEnvRate(zoner->genVals[sfgAttackVolEnv]));
			volEnv.SetRate(2, SFEnvRate(zoner->genVals[sfgHoldVolEnv]));
			volEnv.SetRate(3, SFEnvRate(zoner->genVals[sfgDecayVolEnv]));
			volEnv.SetLevel(3,SFEnvLevel(zoner->genVals[sfgSustainVolEnv]));
			volEnv.SetRate(4, SFEnvRate(zoner->genVals[sfgReleaseVolEnv]));
			short vibamt = zoner->genVals[sfgVibLfoToPitch];
			if (vibamt != 0)
			{
				vibrato.InitLFO(SFAbsCents(zoner->genVals[sfgFreqVibLFO]), WT_SIN,
					SFEnvRate(zoner->genVals[sfgDelayVibLFO]), 
					SFRelCents(frq, vibamt), 0);
			}
			else
				vibrato.SetLevel(0.0);
		}
		zonel = preset->GetSample(mkey, novel, 1);
		if (zonel)
		{
			oscl.InitSF(oscFrq, zonel);
			panl.Set(panSqr, zonel->pan);
		}
	}
	volEnv.Reset(0);
}

void GMPlayer::Param(SeqEvent *evt)
{
	// variable params are set indirectly through the MIDI controller object
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

	if (vibrato.On())
		frqVal += vibrato.Gen();

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

