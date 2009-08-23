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
		gm->instrList.AddItem(ip);
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
	localVals = 0;
	volValue = 1.0;
	bankValue = 0;
	progValue = 0;
}

GMManager::~GMManager()
{
	if (sndbnk)
		sndbnk->Unlock();
	GMPlayer *ip = 0;
	while ((ip = instrList.EnumItem(ip)) != 0)
		ip->Destroy();
}

/// Allocate a variable parameters object (needed for editor)
VarParamEvent *GMManager::AllocParams()
{
	return (VarParamEvent*) EventFactory(0);
}

void GMManager::SetSoundFile(const char *b)
{
	SetSoundBank(SoundBank::FindBank(b));
}

void GMManager::SetSoundBank(SoundBank *b)
{
	if (sndbnk)
		sndbnk->Unlock();
	sndbnk = b;
	if (sndbnk)
	{
		sndbnk->Lock();
		sndFile = sndbnk->name;
	}
	else
		sndFile = "";
	GMPlayer *ip = 0;
	while ((ip = instrList.EnumItem(ip)) != 0)
		ip->SetSoundBank(b);
}

void GMManager::SetMidiControl(MIDIControl *mc)
{
	midiCtrl = mc;
	GMPlayer *ip = 0;
	while ((ip = instrList.EnumItem(ip)) != 0)
		ip->SetMidiControl(mc);
}

void GMManager::SetLocalPan(bsInt16 lp)
{
	if (lp)
		localVals |= GMM_LOCAL_PAN;
	else
		localVals &= ~GMM_LOCAL_PAN;
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
	{
		sndbnk = SoundBank::FindBank(sndFile);
		if (sndbnk)
			sndbnk->Lock();
	}

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
	osc = 0;
	Reset();
}

GMPlayer::GMPlayer(GMManager *g, InstrManager *m)
{
	im = m;
	gm = g;
	sndbnk = gm->GetSoundBank();
	if (sndbnk)
		sndbnk->Lock();
	midiCtrl = gm->GetMidiControl();
	osc = 0;
	Reset();
}

void GMPlayer::Reset()
{
	if (osc)
		delete osc;
	preset = 0;
	zone = 0;
	osc = 0;
	chnl = 0;
	mkey = 69;
	novel = 0;
	frq = 440.0;
	volEnv.InitEnv(0, 0);
}

GMPlayer::~GMPlayer()
{
	Remove();
	if (sndbnk)
		sndbnk->Unlock();
	delete osc;
}

void GMPlayer::SetSoundBank(SoundBank *s)
{
	if (sndbnk)
		sndbnk->Unlock();
	sndbnk = s;
	if (sndbnk) 
		sndbnk->Lock();
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
		novel = 100;
	vol = MIDIControl::volCB[novel];
	sustainOn = midiCtrl->GetCC(chnl, MIDI_CTRL_SUS_ON) > 64;
	sostenuto = midiCtrl->GetCC(chnl, 67) > 64;

	vibDelay = 0;
	modDelay = 0;
	genFlags = 0;
	zone = 0;
	preset = sndbnk->GetInstr(gm->GetBank(chnl), gm->GetPatch(chnl));
	if (preset)
	{
		if (preset->fixedKey != -1)
			mkey = preset->fixedKey;
		if (preset->fixedVel != -1)
			novel = preset->fixedVel;
		zone = preset->GetZone(mkey, novel);
		if (zone)
		{
			genFlags = zone->genFlags;
			if (zone->sample->channels == 2 || zone->sample->linkSamp)
				osc = new GenWaveSF2;
			else if (zone->linkZone)
				osc = new GenWaveSF3;
			else
				osc = new GenWaveSF1;
			osc->InitSF(frq, zone, 0);
			vol += zone->volAtten;
			volEnv.InitEnv(&zone->volEg, mkey, novel);
			if (genFlags & SBGEN_VIBLFOF)
			{
				viblfo.InitWT(zone->vibLfo.rate, WT_SIN);
				vibDelay = (bsInt32) (zone->vibLfo.delay * synthParams.sampleRate);
			}
			if (genFlags & SBGEN_MODLFOX)
			{
				modlfo.InitWT(zone->modLfo.rate, WT_SIN);
				modDelay = (bsInt32) (zone->modLfo.delay * synthParams.sampleRate);
			}
			if (genFlags & SBGEN_MODENVF)
				modEnv.InitEnv(&zone->modEg, mkey, novel);
		}

		AmpValue panLft = midiCtrl->GetPan(chnl);
		AmpValue panRgt = panLft;
		if (zone->chan == 0)
			panRgt += zone->pan;
		else
			panLft += zone->pan;
		if (zone->linkZone)
		{
			if (zone->linkZone->chan == 0)
				panRgt += zone->linkZone->pan;
			else
				panLft += zone->linkZone->pan;
		}
		else
		{
			if (zone->chan == 0)
				panLft += (0 - zone->pan);
			else
				panRgt += (0 - zone->pan);
		}
		panr.Set(panSqr, panRgt);
		panl.Set(panSqr, panLft);
	}
	if (!zone)
	{
		// failsafe - init so that output is all zeros
		osc = new GenWaveSF1;
		osc->InitSF(frq, 0, 0);
		volEnv.InitEnv(NULL, mkey, novel);
		panr.Set(panSqr, 0);
		panl.Set(panSqr, 0);
	}
}

void GMPlayer::Param(SeqEvent *se)
{
	// variable params are set indirectly through the MIDI controller object
	// We handle a pitch change here so that the virtual keyboard works,
	// and treat it like a note-off immediately followed by a note-on,
	// iow - do a "START", but retain ENV level
	VarParamEvent *evt = (VarParamEvent *)se;
	int nkey = evt->pitch + 12;
	if (nkey != mkey)
	{
		AmpValue v = volEnv.GetCurLevel();
		Start(se);
		volEnv.SetCurLevel(v);
	}
}

void GMPlayer::Stop()
{
	sustainOn = (gm->GetCC(chnl, MIDI_CTRL_SUS_ON) > 64)
		     || (!sostenuto && gm->GetCC(chnl, MIDI_CTRL_SOS_ON) > 64);
	if (sustainOn)
		return;
	osc->Release();
	volEnv.Release();
	modEnv.Release();
}

/// Produce the next sample.
void GMPlayer::Tick()
{
	FrqValue frqPC = midiCtrl->GetPitchbendN(chnl);
	AmpValue ampCB = vol + midiCtrl->GetVolumeCB(chnl) + ((1.0 - volEnv.Gen()) * 960.0);

	if (genFlags)
	{
		FrqValue mwFrq = 0;
		AmpValue mwAmp = 0;
		if (genFlags & SBGEN_MODENVF)
			frqPC +=  modEnv.Gen() * zone->modEnvFrq;
		if (genFlags & SBGEN_MODWHLX)
		{
			AmpValue mw = midiCtrl->GetCCN(chnl, MIDI_CTRL_MOD);
			mwFrq = mw * zone->mwfScale;
			mwAmp = mw * zone->mwaScale;
		}
		if (genFlags & SBGEN_VIBLFOF)
		{
			if (vibDelay == 0)
				frqPC += viblfo.Gen() * (zone->vibLfoFrq + mwFrq);
			else
				vibDelay--;
		}
		if (genFlags & SBGEN_MODLFOX)
		{
			if (modDelay == 0)
			{
				AmpValue m = modlfo.Gen();
				if (genFlags & SBGEN_MODLFOA)
					ampCB += m * (zone->modLfoVol + mwAmp);
				if (genFlags & SBGEN_MODLFOF)
					frqPC += m * (zone->modLfoFrq + mwFrq);
			}
			else
				modDelay--;
		}

		if (genFlags & SBGEN_BRTHAMP)
			ampCB += (zone->bthaScale * midiCtrl->GetCCN(chnl, MIDI_CTRL_BRTH));
		if (genFlags & SBGEN_EXPRAMP)
			ampCB += (zone->expaScale * midiCtrl->GetCCN(chnl, MIDI_CTRL_EXPR));
	}

	FrqValue frqVal = frq * synthParams.GetCentsMult((int)frqPC);
	AmpValue ampVal = synthParams.AttenCB((int)ampCB);

	osc->UpdateFrequency(frqVal);

	AmpValue oscValR;
	AmpValue oscValL;
	osc->Tick(oscValL, oscValR);

	im->Output2(chnl,
		ampVal * ((oscValR * panr.panlft) + (oscValL * panl.panlft)),
		ampVal * ((oscValR * panr.panrgt) + (oscValL * panl.panrgt)));
}

int  GMPlayer::IsFinished()
{
	if (sustainOn)
	{
		if (midiCtrl->GetCC(chnl, MIDI_CTRL_SUS_ON) > 64) // sustain on
			return 0;
		if (!sostenuto && midiCtrl->GetCC(chnl, MIDI_CTRL_SOS_ON) > 64)
			return 0;
		Stop(); // sustain was on during Stop(), so do it now.
	}
	return volEnv.IsFinished() || osc->IsFinished();
}

void GMPlayer::Destroy()
{
	Remove();
	delete this;
}

