//////////////////////////////////////////////////////////////////////
// BasicSynth Tone instruments
//
// See _BasciSynth_ Chapter 19 and 22 for a full explanation
//
// ToneInst - selectable waveform
// ToneFM - fixed modulation index
//
// These instruments provide simple, fast tone generators. Although
// limited in variabilty, they are still quite useful for many
// synthesized sounds. These include a signal generator, envelope
// generator, LFO and Pitch bend. The ToneInstr uses a single
// wavetable oscillator. The ToneFM uses a two oscillator FM
// generator. (Although not implemented, a ToneAM instrument could
// be added easily by modeling the class on the ToneFM instrument.)
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "Tone.h"

ToneBase::ToneBase()
{
	osc = 0;
	chnl = 0;
	vol = 1.0;
	im = NULL;
	pbOn = 0;
	lfoOn = 0;
}

ToneBase::~ToneBase()
{
}

void ToneBase::Copy(ToneBase *tp)
{
	chnl = tp->chnl;
	vol = tp->vol;
	env.Copy(&tp->env);
	osc->SetFrequency(tp->osc->GetFrequency());
	osc->SetWavetable(tp->osc->GetWavetable());
	lfoOn = tp->lfoOn;
	lfoGen.Copy(&tp->lfoGen);
	pbOn = tp->pbOn;
	pbGen.Copy(&tp->pbGen);
}

void ToneBase::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);
	osc->Reset(0);
	env.Reset(0);
	if (lfoOn)
		lfoGen.Reset(0);
	if (pbOn)
		pbGen.Reset(0);
}

void ToneBase::Param(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);
	osc->Reset(-1);
	env.Reset(-1);
	if (lfoOn)
		lfoGen.Reset(-1);
	if (pbOn)
		pbGen.Reset(-1);
}


void ToneBase::Stop()
{
	env.Release();
}

void ToneBase::Tick()
{
	if (lfoOn)
		osc->PhaseModWT(lfoGen.Gen() * synthParams.frqTI);
	if (pbOn)
		osc->PhaseModWT(pbGen.Gen() * synthParams.frqTI);
	im->Output(chnl, vol * env.Gen() * osc->Gen());
}

int  ToneBase::IsFinished()
{
	return env.IsFinished();
}

void ToneBase::Destroy()
{
	delete this;
}

int ToneBase::LoadOscil(XmlSynthElem *elem)
{
	double dval;
	long ival;
	if (elem->GetAttribute("frq", dval) == 0)
		osc->SetFrequency(FrqValue(dval));
	if (elem->GetAttribute("wt", ival) == 0)
		osc->SetWavetable((int) ival);
	if (elem->GetAttribute("vol", dval) == 0)
		vol = AmpValue(dval);
	return 0;
}

int ToneBase::LoadEnv(XmlSynthElem *elem)
{
	double dvals[7];
	long ival;
	elem->GetAttribute("st",  dvals[0]);
	elem->GetAttribute("atk", dvals[1]);
	elem->GetAttribute("pk",  dvals[2]);
	elem->GetAttribute("dec", dvals[3]);
	elem->GetAttribute("sus", dvals[4]);
	elem->GetAttribute("rel", dvals[5]);
	elem->GetAttribute("end", dvals[6]);
	elem->GetAttribute("ty", ival);
	env.InitADSR(AmpValue(dvals[0]), 
		FrqValue(dvals[1]), AmpValue(dvals[2]),
		FrqValue(dvals[3]), AmpValue(dvals[4]),
		FrqValue(dvals[5]), AmpValue(dvals[6]), (EGSegType)ival);
	return 0;
}

int ToneBase::Load(XmlSynthElem *parent)
{
	long ival;

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("osc"))
			LoadOscil(elem);
		else if (elem->TagMatch("env"))
			LoadEnv(elem);
		else if (elem->TagMatch("lfo"))
		{
			lfoGen.Load(elem);
			lfoOn = lfoGen.On();
		}
		else if (elem->TagMatch("pb"))
		{
			if (elem->GetAttribute("on", ival) == 0)
				pbOn = (int) ival;
			pbGen.Load(elem);
		}
		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int ToneBase::SaveOscil(XmlSynthElem *elem)
{
	elem->SetAttribute("frq", osc->GetFrequency());
	elem->SetAttribute("wt", (short)osc->GetWavetable());
	elem->SetAttribute("vol", vol);
	return 0;
}

int ToneBase::SaveEnv(XmlSynthElem *elem)
{
	elem->SetAttribute("st", env.GetStart());
	elem->SetAttribute("atk", env.GetAtkRt());
	elem->SetAttribute("pk",  env.GetAtkLvl());
	elem->SetAttribute("dec", env.GetDecRt());
	elem->SetAttribute("sus", env.GetSusLvl());
	elem->SetAttribute("rel", env.GetRelRt());
	elem->SetAttribute("end", env.GetRelLvl());
	elem->SetAttribute("ty", (short)env.GetType());
	return 0;
}

int ToneBase::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem = parent->AddChild("osc");
	if (elem == NULL)
		return -1;
	SaveOscil(elem);
	delete elem;

	elem = parent->AddChild("env");
	if (elem == NULL)
		return -1;
	SaveEnv(elem);
	delete elem;

	elem = parent->AddChild("lfo");
	if (elem == NULL)
		return -1;
	lfoGen.Save(elem);
	delete elem;

	elem = parent->AddChild("pb");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("on", (short) pbOn);
	pbGen.Save(elem);
	delete elem;

	return 0;
}

int ToneBase::SetParams(VarParamEvent *params)
{
	int err = 0;

	FrqValue frq = params->frq;
	chnl = params->chnl;
	vol  = params->vol;
	osc->SetFrequency(frq);
	lfoGen.SetSigFrq(frq);
	pbGen.SetFrequency(frq);

	bsInt16 *id = params->idParam;
	float *valp = params->valParam;
	int n = params->numParam;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int ToneBase::SetParam(int idval, float val)
{
	switch (idval)
	{
	case 16:
		osc->SetWavetable((int) val);
		break;
	case 17:
		env.SetStart(AmpValue(val));
		break;
	case 18:
		env.SetAtkRt(FrqValue(val));
		break;
	case 19:
		env.SetAtkLvl(AmpValue(val));
		break;
	case 20:
		env.SetDecRt(FrqValue(val));
		break;
	case 21:
		env.SetSusLvl(AmpValue(val));
		break;
	case 22:
		env.SetRelRt(FrqValue(val));
		break;
	case 23:
		env.SetRelLvl(AmpValue(val));
		break;
	case 24:
		env.SetType((EGSegType) (int) val);
		break;
	case 25:
		lfoGen.SetFrequency(FrqValue(val));
		break;
	case 26:
		lfoGen.SetWavetable((int) val);
		break;
	case 27:
		lfoGen.SetAttack(FrqValue(val));
		break;
	case 28:
		lfoGen.SetLevel(AmpValue(val));
		break;
	case 29:
		pbOn = (int) val;
		break;
	case 30:
		pbGen.SetRate(0, FrqValue(val));
		break;
	case 31:
		pbGen.SetRate(1, FrqValue(val));
		break;
	case 32:
		pbGen.SetAmount(0, AmpValue(val));
		break;
	case 33:
		pbGen.SetAmount(1, AmpValue(val));
		break;
	case 34:
		pbGen.SetAmount(2, AmpValue(val));
		break;
	default:
		return 1;
	}
	return 0;
}

int ToneBase::GetParams(VarParamEvent *params)
{
	params->SetParam(P_FREQ, (float)osc->GetFrequency());
	params->SetParam(P_VOLUME, (float)vol);
	params->SetParam(16, (float) osc->GetWavetable());
	params->SetParam(17, (float) env.GetStart());
	params->SetParam(18, (float) env.GetAtkRt());
	params->SetParam(19, (float) env.GetAtkLvl());
	params->SetParam(20, (float) env.GetDecRt());
	params->SetParam(21, (float) env.GetSusLvl());
	params->SetParam(22, (float) env.GetRelRt());
	params->SetParam(23, (float) env.GetRelLvl());
	params->SetParam(24, (float) env.GetType());
	params->SetParam(25, (float) lfoGen.GetFrequency());
	params->SetParam(26, (float) lfoGen.GetWavetable());
	params->SetParam(27, (float) lfoGen.GetAttack());
	params->SetParam(28, (float) lfoGen.GetLevel());
	params->SetParam(29, (float) pbOn);
	params->SetParam(30, (float) pbGen.GetRate(0));
	params->SetParam(31, (float) pbGen.GetRate(1));
	params->SetParam(32, (float) pbGen.GetAmount(0));
	params->SetParam(33, (float) pbGen.GetAmount(1));
	params->SetParam(34, (float) pbGen.GetAmount(2));
	return 0;
}

////////////////////////////////////////////////////////////////
Instrument *ToneInstr::ToneFactory(InstrManager *m, Opaque tmplt)
{
	ToneInstr *ip = new ToneInstr;
	if (tmplt)
		ip->Copy((ToneBase*)tmplt);
	ip->im = m;
	return ip;
}

SeqEvent *ToneInstr::ToneEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 35;
	return (SeqEvent*)ep;
}

ToneInstr::ToneInstr()
{
#ifdef USE_OSCILI
	osc = (GenWaveWT*) new GenWaveI;
#else
	osc = new GenWaveWT;
#endif
}

ToneInstr::~ToneInstr()
{
	delete osc;
}

///////////////////////////////////////////////////////////
Instrument *ToneFM::ToneFMFactory(InstrManager *m, Opaque tmplt)
{
	ToneFM *ip = new ToneFM;
	ip->im = m;
	if (tmplt)
		ip->Copy((ToneFM*)tmplt);
	return ip;
}

SeqEvent *ToneFM::ToneFMEventFactory(Opaque tmplt)
{
	VarParamEvent *evt = new VarParamEvent;
	evt->maxParam = 37;
	return (SeqEvent*)evt;
}

ToneFM::ToneFM()
{
	osc = (GenWaveWT*) new GenWaveFM;
}

ToneFM::~ToneFM()
{
	delete osc;
}

void ToneFM::Copy(ToneFM *tp)
{
	ToneBase::Copy((ToneBase *)tp);
	GenWaveFM *osc1 = (GenWaveFM *) tp->osc;
	GenWaveFM *osc2 = (GenWaveFM *) osc;
	osc2->SetModIndex(osc1->GetModIndex());
	osc2->SetModMultiple(osc1->GetModMultiple());
}

int ToneFM::LoadOscil(XmlSynthElem *elem)
{
	int err = ToneBase::LoadOscil(elem);
	GenWaveFM *fm = (GenWaveFM*) osc;
	double dval;
	if (elem->GetAttribute("mnx", dval) == 0)
		fm->SetModIndex(AmpValue(dval));
	if (elem->GetAttribute("mul", dval) == 0)
		fm->SetModMultiple(FrqValue(dval));
	return err;
}

int ToneFM::SaveOscil(XmlSynthElem *elem)
{
	int err = ToneBase::SaveOscil(elem);
	elem->SetAttribute("mnx", ((GenWaveFM*)osc)->GetModIndex());
	elem->SetAttribute("mul", ((GenWaveFM*)osc)->GetModMultiple());
	return err;
}

int ToneFM::SetParam(int id, float val)
{
	int err = 0;
	if (id == 35)
		((GenWaveFM*)osc)->SetModIndex(AmpValue(val));
	else if (id == 36)
		((GenWaveFM*)osc)->SetModMultiple(FrqValue(val));
	else
		err = ToneBase::SetParam(id, val);
	return err;
}

int ToneFM::GetParams(VarParamEvent *params)
{
	int err = ToneBase::GetParams(params);
	params->SetParam(35, (float) ((GenWaveFM*)osc)->GetModIndex());
	params->SetParam(36, (float) ((GenWaveFM*)osc)->GetModMultiple());
	return err;
}
