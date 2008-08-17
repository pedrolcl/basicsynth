// Tone.cpp: implementation of the Tone classses.
//
// ToneInst - selectable waveform
// ToneFM - fixed modulation index
// ToneAM - fixed modulation index (TBD)
//
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "Tone.h"

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

Instrument *ToneInstr::ToneFactory(InstrManager *m, Opaque tmplt)
{
	ToneInstr *ip = new ToneInstr;
	ip->im = m;
	if (tmplt)
		ip->Copy((ToneInstr*)tmplt);
	return ip;
}

SeqEvent *ToneInstr::ToneEventFactory(Opaque tmplt)
{
	ToneEvent *te = new ToneEvent;
	ToneInstr *ip = (ToneInstr *)tmplt;
	if (ip)
	{
		te->vol = ip->vol;
		te->frq = ip->osc.GetFrequency();
		te->waveTable = ip->osc.GetWavetable();
		te->startLevel = ip->env.GetStart();
		te->atkRate = ip->env.GetAtkRt();
		te->atkLevel = ip->env.GetAtkLvl();
		te->decRate = ip->env.GetDecRt();
		te->susLevel = ip->env.GetSusLvl();
		te->relRate = ip->env.GetRelRt();
		te->endLevel = ip->env.GetRelLvl();
		ip->lfoGen.GetSettings(te->lfoFreq, te->lfoWaveTable, te->lfoAtkRate, te->lfoAmp);
	}
	else
	{
		te->frq = 440.0;
		te->vol = 1.0;
		te->startLevel = 0.0;
		te->atkLevel = 1.0;
		te->susLevel = 1.0;
		te->endLevel = 0.0;
		te->atkRate = 0.001;
		te->decRate = 0.001;
		te->relRate = 0.001;
		te->envType = linSeg;
		te->waveTable = WT_SIN;
		te->lfoFreq = 3.5;
		te->lfoWaveTable = WT_SIN;
		te->lfoAtkRate = 0.0;
		te->lfoAmp = 0.0;
	}
	return (SeqEvent*) te;
}

ToneInstr::ToneInstr()
{
	chnl = 0;
	vol = 1.0;
	im = NULL;
}

ToneInstr::~ToneInstr()
{

}

void ToneInstr::Copy(ToneInstr *tp)
{
	env.Copy(&tp->env);
	osc.SetFrequency(tp->osc.GetFrequency());
	osc.SetWavetable(tp->osc.GetWavetable());
	lfoGen.Copy(&tp->lfoGen);
}

void ToneInstr::Start(SeqEvent *evt)
{
	chnl = evt->chnl;
	SetParam((ToneEvent*)evt);
	osc.Reset(0);
	env.Reset(0);
	lfoGen.Reset(0);
}

void ToneInstr::Param(SeqEvent *evt)
{
	SetParam((ToneEvent*)evt);
	osc.Reset(0);
	env.Reset(0);
	lfoGen.Reset(0);
}

void ToneInstr::SetParam(ToneEvent *te)
{
	chnl = te->chnl;
	vol  = te->vol;
	env.InitADSR(te->startLevel, te->atkRate, te->atkLevel, te->decRate, 
		         te->susLevel, te->relRate, te->endLevel, te->envType);
	osc.SetFrequency(te->frq);
	osc.SetWavetable(te->waveTable);
	lfoGen.SetFrequency(te->lfoFreq);
	lfoGen.SetWavetable(te->lfoWaveTable);
	lfoGen.SetAttack(te->lfoAtkRate);
	lfoGen.SetLevel(te->lfoAmp);
}

void ToneInstr::Stop()
{
	env.Release();
}

void ToneInstr::Tick()
{
	if (lfoGen.On())
		osc.PhaseModWT(lfoGen.Gen() * synthParams.frqTI);
	im->Output(chnl, vol * env.Gen() * osc.Gen());
}

int  ToneInstr::IsFinished()
{
	return env.IsFinished();
}

void ToneInstr::Destroy()
{
	delete this;
}

int ToneInstr::Load(XmlSynthElem *parent)
{
	double dvals[7];
	long ival;


	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("osc"))
		{
			if (elem->GetAttribute("frq", dvals[0]) == 0)
				osc.SetFrequency((FrqValue)dvals[0]);
			if (elem->GetAttribute("wt", ival) == 0)
				osc.SetWavetable((int) ival);
			if (elem->GetAttribute("vol", dvals[0]) == 0)
				vol = (AmpValue) dvals[0];
		}
		else if (elem->TagMatch("env"))
		{
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
		}
		else if (elem->TagMatch("lfo"))
		{
			lfoGen.Load(elem);
		}
		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int ToneInstr::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem = parent->AddChild("osc");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("frq", osc.GetFrequency());
	elem->SetAttribute("wt", (long) osc.GetWavetable());
	elem->SetAttribute("vol", (double) vol);
	delete elem;

	elem = parent->AddChild("env");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("st", (double)env.GetStart());
	elem->SetAttribute("atk", (double)env.GetAtkRt());
	elem->SetAttribute("pk",  (double)env.GetAtkLvl());
	elem->SetAttribute("dec", (double)env.GetDecRt());
	elem->SetAttribute("sus",  (double)env.GetSusLvl());
	elem->SetAttribute("rel", (double)env.GetRelRt());
	elem->SetAttribute("end",  (double)env.GetRelLvl());
	elem->SetAttribute("ty", (long) env.GetType());
	delete elem;

	elem = parent->AddChild("lfo");
	if (elem == NULL)
		return -1;
	lfoGen.Save(elem);
	delete elem;

	return 0;
}

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
	ToneFMEvent *te = new ToneFMEvent;
	ToneFM *ip = (ToneFM *)tmplt;
	if (ip)
	{
		te->vol = ip->vol;
		te->frq = ip->osc.GetFrequency();
		te->modIndex = ip->osc.GetModIndex();
		te->startLevel = ip->env.GetStart();
		te->atkRate = ip->env.GetAtkRt();
		te->atkLevel = ip->env.GetAtkLvl();
		te->decRate = ip->env.GetDecRt();
		te->susLevel = ip->env.GetSusLvl();
		te->relRate = ip->env.GetRelRt();
		te->endLevel = ip->env.GetRelLvl();
		ip->lfoGen.GetSettings(te->lfoFreq, te->lfoWaveTable, te->lfoAtkRate, te->lfoAmp);
	}
	else
	{
		te->frq = 440.0;
		te->vol = 1.0;
		te->modIndex = 1.0;
		te->modMult = 1.0;
		te->startLevel = 0.0;
		te->atkLevel = 1.0;
		te->susLevel = 1.0;
		te->endLevel = 0.0;
		te->atkRate = 0.001;
		te->decRate = 0.001;
		te->relRate = 0.001;
		te->envType = linSeg;
		te->lfoFreq = 3.5;
		te->lfoWaveTable = WT_SIN;
		te->lfoAtkRate = 0.0;
		te->lfoAmp = 0.0;
	}
	return (SeqEvent*) te;
}

ToneFM::ToneFM()
{
	chnl = 0;
	vol = 1.0;
	im = NULL;
}

ToneFM::~ToneFM()
{

}

void ToneFM::Copy(ToneFM *tp)
{
	env.Copy(&tp->env);
	osc.SetFrequency(tp->osc.GetFrequency());
	osc.SetModIndex(tp->osc.GetModIndex());
	osc.SetModMultiple(tp->osc.GetModMultiple());
	lfoGen.Copy(&tp->lfoGen);
}

void ToneFM::Start(SeqEvent *evt)
{
	chnl = evt->chnl;
	SetParam((ToneFMEvent*)evt);
	osc.Reset(0);
	env.Reset(0);
	lfoGen.Reset(0);
}

void ToneFM::Param(SeqEvent *evt)
{
	SetParam((ToneFMEvent*)evt);
	osc.Reset(0);
	env.Reset(0);
	lfoGen.Reset(0);
}

void ToneFM::SetParam(ToneFMEvent *te)
{
	chnl = te->chnl;
	vol  = te->vol;
	env.InitADSR(te->startLevel, te->atkRate, te->atkLevel, te->decRate, 
		         te->susLevel, te->relRate, te->endLevel, te->envType);
	osc.SetWavetable(WT_SIN);
	osc.SetFrequency(te->frq);
	osc.SetModMultiple(te->modMult);
	osc.SetModIndex(te->modIndex);
	lfoGen.SetFrequency(te->lfoFreq);
	lfoGen.SetWavetable(te->lfoWaveTable);
	lfoGen.SetAttack(te->lfoAtkRate);
	lfoGen.SetLevel(te->lfoAmp);
}

void ToneFM::Stop()
{
	env.Release();
}

void ToneFM::Tick()
{
	if (lfoGen.On())
		osc.PhaseModWT(lfoGen.Gen() * synthParams.frqTI);
	im->Output(chnl, vol * env.Gen() * osc.Gen());
}

int  ToneFM::IsFinished()
{
	return env.IsFinished();
}

void ToneFM::Destroy()
{
	delete this;
}

int ToneFM::Load(XmlSynthElem *parent)
{
	double dvals[7];
	long ival;

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("osc"))
		{
			if (elem->GetAttribute("frq", dvals[0]) == 0)
				osc.SetFrequency(FrqValue(dvals[0]));
			if (elem->GetAttribute("mnx", dvals[0]) == 0)
				osc.SetModIndex(AmpValue(dvals[0]));
			if (elem->GetAttribute("mul", dvals[0]) == 0)
				osc.SetModMultiple(FrqValue(dvals[0]));
			if (elem->GetAttribute("vol", dvals[0]) == 0)
				vol = AmpValue(dvals[0]);
		}
		else if (elem->TagMatch("env"))
		{
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
		}
		else if (elem->TagMatch("lfo"))
		{
			lfoGen.Load(elem);
		}
		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int ToneFM::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem = parent->AddChild("osc");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("frq", osc.GetFrequency());
	elem->SetAttribute("vol",  vol);
	elem->SetAttribute("mnx",  osc.GetModIndex());
	elem->SetAttribute("mul",  osc.GetModMultiple());
	delete elem;

	elem = parent->AddChild("env");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("st", env.GetStart());
	elem->SetAttribute("atk", env.GetAtkRt());
	elem->SetAttribute("pk",  env.GetAtkLvl());
	elem->SetAttribute("dec", env.GetDecRt());
	elem->SetAttribute("sus",  env.GetSusLvl());
	elem->SetAttribute("rel", env.GetRelRt());
	elem->SetAttribute("end",  env.GetRelLvl());
	elem->SetAttribute("ty", (short) env.GetType());
	delete elem;

	elem = parent->AddChild("lfo");
	if (elem == NULL)
		return -1;
	lfoGen.Save(elem);
	delete elem;

	return 0;
}
