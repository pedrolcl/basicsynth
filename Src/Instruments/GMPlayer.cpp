//////////////////////////////////////////////////////////////////////
/// @file GMPlayer.cpp Implementation of the General MIDI player
//
// This instrument provides an emulation of a MIDI keyboard using
// SoundFont GM file.
//
// The implementation is partial:
//  - only some CC values are used
//  - filters are not dynamic
//  - modulators (other than defaults) are not implemented
//  - effects send is not implemented
//  - pan is set at note-on and not dynamic
// Well-known modulators (MOD wheel, expression, and breath controllers)
// are "unrolled" and implemented by flags and member variables.
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

// load a template from the project file
Opaque GMManager::TmpltFactory(XmlSynthElem *tmplt)
{
	GMManager *gm = new GMManager;
	if (tmplt)
		gm->Load(tmplt);
	else
		gm->SetSoundBank(SoundBank::DefaultBank());
	return gm;
}

// destroy the template
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
	ep->maxParam = 21;
	return (SeqEvent*)ep;
}

GMManager::GMManager()
{
	sndbnk = 0;
	im = 0;
	localVals = 0;
	volValue = 1.0;
	panValue = 0.0;
	bankValue = 0;
	progValue = 0;
	genFlags = GMGEN_DEF;
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

void GMManager::ExclNoteOn(GMPlayer *player)
{
	GMPlayer *prev = 0;
	while ((prev = instrList.EnumItem(prev)) != 0)
		if (prev->CheckExculsive(player))
			break;
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
			elem.GetAttribute("pan", panValue);
			elem.GetAttribute("bank", bankValue);
			elem.GetAttribute("prog", progValue);
			elem.GetAttribute("flags", lv);
			genFlags = (bsUint32) lv;
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
	elem.SetAttribute("pan",  (float) panValue);
	elem.SetAttribute("bank", bankValue);
	elem.SetAttribute("prog", progValue);
	elem.SetAttribute("flags", (long) genFlags);
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
		panValue = AmpValue(val);
		break;
	case 19:
		bankValue = (bsInt16) val;
		break;
	case 20:
		progValue = (bsInt16) val;
		break;
	case 21:
		genFlags = (bsUint32) val;
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
	params->SetParam(18, (float) panValue);
	params->SetParam(19, (float) bankValue);
	params->SetParam(20, (float) progValue);
	params->SetParam(21, (float) genFlags);
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
		*val = panValue;
		break;
	case 19:
		*val = (float) bankValue;
		break;
	case 20:
		*val = (float) progValue;
		break;
	case 21:
		*val = (float) genFlags;
	default:
		return 1;
	}
	return 0;
}

static InstrParamMap gmManagerParams[] = 
{
	{"bank", 19 },
	{"flags", 21 },
	{"local", 16 }, 
	{"pan", 18 },
	{"prog", 20 },
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

///////////////////////////////////////////////////////////////////////////////

GMPlayer::GMPlayer()
{
	im = 0;
	gm = 0;
	midiCtrl = 0;
	sndbnk = 0;
	instr = 0;
	chnl = 0;
	mkey = 69;
	novel = 0;
	frq = 440.0;
	zoneList = 0;
	allFlags = 0;
}

GMPlayer::GMPlayer(GMManager *g, InstrManager *m)
{
	im = m;
	gm = g;
	sndbnk = gm->GetSoundBank();
	if (sndbnk)
		sndbnk->Lock();
	midiCtrl = gm->GetMidiControl();
	instr = 0;
	chnl = 0;
	mkey = 69;
	novel = 0;
	frq = 440.0;
	zoneList = 0;
	allFlags = 0;
}

GMPlayer::~GMPlayer()
{
	Remove();
	if (sndbnk)
		sndbnk->Unlock();
	ClearZones();
}

void GMPlayer::ClearZones()
{
	GMPlayerZone *pz;
	while ((pz = zoneList) != 0)
	{
		zoneList = pz->next;
		delete pz;
	}
}

void GMPlayer::Reset()
{
	instr = 0;
	chnl = 0;
	mkey = 69;
	novel = 0;
	frq = 440.0;
	ClearZones();
}

void GMPlayer::SetSoundBank(SoundBank *s)
{
	if (sndbnk)
		sndbnk->Unlock();
	sndbnk = s;
	if (sndbnk) 
		sndbnk->Lock();
	instr = 0;
}

/// Start playing a note.
/// Locate the sound bank instrument, then the zone(s).
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
	AmpValue vol = MIDIControl::volCB[novel];
	sustainOn = midiCtrl->GetCC(chnl, MIDI_CTRL_SUS_ON) > 64;
	sostenuto = midiCtrl->GetCC(chnl, 67) > 64;
	AmpValue panCC = gm->GetPan(chnl);

	if (sndbnk)
		instr = sndbnk->GetInstr(gm->GetBank(chnl), gm->GetPatch(chnl));
	if (instr)
	{
		if (instr->fixedKey != -1)
			mkey = instr->fixedKey;
		if (instr->fixedVel != -1)
			novel = instr->fixedVel;
		SBZone *zone = 0;
		while ((zone = instr->EnumZones(zone)) != 0)
		{
			if (zone->Match(mkey, novel))
			{
				GMPlayerZone *pz = new GMPlayerZone(zone);
				if (zoneList)
					zoneList->Insert(pz);
				zoneList = pz;

				pz->genFlags = (zone->genFlags | GMGEN_DEF) & gm->GetGenFlags();

				pz->osc.InitWTLoop(frq, zone->recFreq, zone->rate, zone->tableEnd, 
					   zone->loopStart, zone->loopEnd, zone->mode, zone->sample->sample);
				pz->vol = vol + zone->volAtten;
				pz->volEnv.InitEnv(&zone->volEg, mkey, novel);
				pz->viblfo.InitWT(zone->vibLfo.rate, WT_SIN);
				pz->vibDelay = (bsInt32) (zone->vibLfo.delay * synthParams.sampleRate);
				pz->modlfo.InitWT(zone->modLfo.rate, WT_SIN);
				pz->modDelay = (bsInt32) (zone->modLfo.delay * synthParams.sampleRate);
				if (pz->genFlags & SBGEN_MODENVF)
					pz->modEnv.InitEnv(&zone->modEg, mkey, novel);
				pz->pan.Set(panSqr, zone->pan + panCC);
				// TODO: convert SF2/DLS filter 'q' to resonance and gain
				// we can then calculate a0, b0, and b1 right here...
				// pz->filt.InitFilter(a0, b0, b1);
				if (pz->genFlags & SBGEN_FILTER)
				{
					if (zone->filtFreq == 0 || zone->filtFreq >= 20000)
						pz->genFlags &= ~SBGEN_FILTER;
					else
						pz->filt.CalcCoef(zone->filtFreq, 1.0);
				}
				allFlags |= pz->genFlags;
				exclNote = zone->exclNote;
			}
		}
		if (exclNote)
			gm->ExclNoteOn(this);
	}
}

void GMPlayer::Param(SeqEvent *se)
{
	// variable params are set indirectly through the MIDI controller object
	// We handle a pitch change here so that the virtual keyboard works,
	// and treat it like a note-off immediately followed by a note-on.
	VarParamEvent *evt = (VarParamEvent *)se;
	if (mkey != (evt->pitch + 12))
	{
		Reset();
		Start(se);
	}
}

void GMPlayer::Stop()
{
	sustainOn = (gm->GetCC(chnl, MIDI_CTRL_SUS_ON) > 64)
		     || (!sostenuto && gm->GetCC(chnl, MIDI_CTRL_SOS_ON) > 64);
	if (sustainOn)
		return;
	GMPlayerZone *pz = zoneList;
	while (pz)
	{
		pz->osc.Release();
		pz->volEnv.Release();
		pz->modEnv.Release();
		pz = pz->next;
	}
}

void GMPlayer::Cancel()
{
	ClearZones();
	sustainOn = 0;
}

int GMPlayer::CheckExculsive(GMPlayer *other)
{
	if (other != this && other->chnl == chnl && other->exclNote == exclNote)
	{
		other->Cancel();
		return 1;
	}
	return 0;
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
	GMPlayerZone *pz = zoneList;
	while (pz)
	{
		if (!(pz->volEnv.IsFinished() || pz->osc.IsFinished()))
			return 0;
		pz = pz->next;
	}
	return 1;
}


/// Produce the next sample.
void GMPlayer::Tick()
{
	GMPlayerZone *pz = zoneList;
	if (pz == 0)
		return;

	AmpValue outLeft = 0;
	AmpValue outRight = 0;
	FrqValue pbCC = midiCtrl->GetPitchbendN(chnl);
	AmpValue vol = gm->GetVolume(chnl);
	AmpValue mwCC = midiCtrl->GetCCN(chnl, MIDI_CTRL_MOD);

	// process each zone
	do
	{
		SBZone *zone = pz->zone;
		FrqValue frqPC = 0;
		AmpValue ampCB = pz->vol + ((1.0 - pz->volEnv.Gen()) * 960.0);
		bsInt32 genFlags = pz->genFlags;
		if (genFlags)
		{
			FrqValue mwFrq = 0;
			AmpValue mwAmp = 0;
			if (genFlags & SBGEN_PITWHLF)
				frqPC += pbCC;
			if (genFlags & SBGEN_MODENVF)
				frqPC += pz->modEnv.Gen() * zone->modEnvFrq;
			if (genFlags & SBGEN_MODWHLX && mwCC != 0)
			{
				mwFrq = mwCC * zone->mwfScale;
				mwAmp = mwCC * zone->mwaScale;
			}
			if (genFlags & (SBGEN_VIBLFOF|SBGEN_MODWHLF))
			{
				if (pz->vibDelay == 0)
					frqPC += pz->viblfo.Gen() * (zone->vibLfoFrq + mwFrq);
				else
					pz->vibDelay--;
			}
			if (genFlags & (SBGEN_MODLFOX|SBGEN_MODWHLA))
			{
				if (pz->modDelay == 0)
				{
					AmpValue m = pz->modlfo.Gen();
					if (genFlags & (SBGEN_MODLFOA|SBGEN_MODWHLA))
						ampCB += m * (zone->modLfoVol + mwAmp);
					if (genFlags & (SBGEN_MODLFOF|SBGEN_MODWHLF))
						frqPC += m * (zone->modLfoFrq + mwFrq);
				}
				else
					pz->modDelay--;
			}

			if (genFlags & SBGEN_BRTHAMP)
				ampCB += (zone->bthaScale * midiCtrl->GetCCN(chnl, MIDI_CTRL_BRTH));
			if (genFlags & SBGEN_EXPRAMP)
				ampCB += (zone->expaScale * midiCtrl->GetCCN(chnl, MIDI_CTRL_EXPR));

			pz->osc.UpdateFrequency(frq * synthParams.GetCentsMult((int)frqPC));
		}

		// run the oscillator
		AmpValue out = pz->osc.Gen();

		if (genFlags & SBGEN_FILTER)
			out = pz->filt.Sample(out);

		// apply attenuation and panning
		out *= vol * synthParams.AttenCB((int)ampCB);
		outLeft  += out * pz->pan.panlft;
		outRight += out * pz->pan.panrgt;
	} while ((pz = pz->next) != 0);
	// TODO: apply reverb/chorus
	im->Output2(chnl, outLeft, outRight);
}

void GMPlayer::Destroy()
{
	Remove();
	delete this;
	// optional: keep a list of players and recycle them
	// Reset();
	// Add to free list.
}

