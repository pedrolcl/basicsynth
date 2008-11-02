//////////////////////////////////////////////////////////////////////
// BasicSynth Subtractive synthesis instrument
//
// See _BasicSynth_ Chapter 21 for a full explanation
//
// This instrument contains an Oscillator, Noise source, BiQuad filter,
// and envelope generators for amplitude and filter frequency.
//
// Copyright 2008, Daniel R. Mitchell
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "SubSynth.h"

Instrument *SubSynth::SubSynthFactory(InstrManager *m, Opaque tmplt)
{
	SubSynth *ip = new SubSynth;
	ip->im = m;
	if (tmplt)
		ip->Copy((SubSynth *) tmplt);
	return ip;
}

SeqEvent *SubSynth::SubSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 50;
	return (SeqEvent *) ep;
}


SubSynth::SubSynth()
{
	vol = 1.0;
	chnl = 0;
	fltGain = 1.0;
	fltRes = 0.5;
	fltType = 0;
	filt = 0;
	sigMix = 1.0;
	nzMix = 0.0;
	nzOn = 0;
	pbOn = 0;
}

SubSynth::~SubSynth()
{
	delete filt;
}

void SubSynth::Copy(SubSynth *tp)
{
	osc.SetWavetable(tp->osc.GetWavetable());
	sigMix = tp->sigMix;
	nzMix  = 1.0 - sigMix;
	fltGain = tp->fltGain;
	fltRes = tp->fltRes;
	fltType = tp->fltType;
	envSig.Copy(&tp->envSig);
	envFlt.Copy(&tp->envFlt);
	CreateFilter();
	filt->Init(&envFlt, fltGain, fltRes);
	filt->Copy(tp->filt);
	lfoGen.Copy(&tp->lfoGen);
	nzOn = nzMix > 0;
	pbOn = tp->pbOn;
	pbGen.Copy(&tp->pbGen);
}

void SubSynth::CreateFilter()
{
	delete filt;
	switch (fltType)
	{
	case 0:
		filt = new SubFiltLP;
		break;
	case 1:
		filt = new SubFiltHP;
		break;
	case 2:
		filt = new SubFiltBP;
		break;
	case 3:
		filt = new SubFiltRES;
		break;
	default:
		filt = new SubFilt;
		break;
	}
	filt->Init(&envFlt, fltGain, fltRes);
}

void SubSynth::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent *)evt);
	osc.Reset(0);
	envSig.Reset(0);
	envFlt.Reset(0);
	filt->Init(&envFlt, fltGain, fltRes);
	filt->Reset(0);
	lfoGen.Reset(0);
	if (pbOn)
		pbGen.Reset(0);
}

void SubSynth::Param(SeqEvent *evt)
{
	SetParams((VarParamEvent *)evt);
	osc.Reset(-1);
	filt->Reset(-1);
	lfoGen.Reset(-1);
}

void SubSynth::SetParams(VarParamEvent *evt)
{
	vol = evt->vol;
	osc.SetFrequency(evt->frq);
	pbGen.SetFrequency(evt->frq);
	lfoGen.SetSigFrq(evt->frq);
	chnl = evt->chnl;
	bsInt16 *id = evt->idParam;
	float *valp = evt->valParam;
	float val;
	short ft;
	int n;
	for (n = evt->numParam; n > 0; n--)
	{
		val = *valp++;
		switch (*id++)
		{
		case 16: //	Sets the mixture of oscillator output and noise output.
			sigMix = val;
			nzMix  = 1.0 - sigMix;
			nzOn = nzMix > 0;
			break;
		case 17: //Wave table index.
			osc.SetWavetable((int) val);
			break;
		case 18: // Filter type
			ft = (short) val;
			if (ft != fltType)
			{
				fltType = ft;
				CreateFilter();
			}
			break;
		case 19: //Filter gain
			fltGain = AmpValue(val);
			break;
		case 20:
			fltRes = AmpValue(val);
			break;
		case 21: //Oscillator envelope start value.
			envSig.SetStart(AmpValue(val));
			break;
		case 22: //Oscillator envelope attack rate
			envSig.SetAtkRt(FrqValue(val));
			break;
		case 23: //Oscillator envelope peak level
			envSig.SetAtkLvl(AmpValue(val));
			break;
		case 24: //	Oscillator envelope decay rate
			envSig.SetDecRt(FrqValue(val));
			break;
		case 25: // Oscillator envelope sustain level
			envSig.SetSusLvl(AmpValue(val));
			break;
		case 26:
			envSig.SetRelRt(FrqValue(val));
			break;
		case 27: //	Oscillator envelope release level
			envSig.SetRelLvl(AmpValue(val));
			break;
		case 28: //Oscillator envelope curve type
			envSig.SetType((EGSegType) (int) val);
			break;
		case 30: //Filter envelope start value.
			envFlt.SetStart(AmpValue(val));
			break;
		case 31: //Filter envelope attack rate
			envFlt.SetAtkRt(FrqValue(val));
			break;
		case 32: //Filter envelope peak level
			envFlt.SetAtkLvl(AmpValue(val));
			break;
		case 33: //Filter envelope decay rate
			envFlt.SetDecRt(FrqValue(val));
			break;
		case 34: //Filter envelope sustain level
			envFlt.SetSusLvl(AmpValue(val));
			break;
		case 35: //Filter envelope release rate
			envFlt.SetRelRt(FrqValue(val));
			break;
		case 36: //Filter envelope final level
			envFlt.SetRelLvl(AmpValue(val));
			break;
		case 37: //Filter envelope curve type
			envFlt.SetType((EGSegType) (int) val);
			break;
		case 40: //LFO Frequency
			lfoGen.SetFrequency(FrqValue(val));
			break;
		case 41: //LFO wavetable index
			lfoGen.SetWavetable((int)val);
			break;
		case 42: //LFO envelope attack rate
			lfoGen.SetAttack(FrqValue(val));
			break;
		case 43: //LFO level
			lfoGen.SetLevel(AmpValue(val));
			break;
		case 44: // PB On
			pbOn = (int) val;
			break;
		case 45:
			pbGen.SetRate(0, FrqValue(val));
			break;
		case 46:
			pbGen.SetRate(1, FrqValue(val));
			break;
		case 47:
			pbGen.SetAmount(0, FrqValue(val));
			break;
		case 48:
			pbGen.SetAmount(1, FrqValue(val));
			break;
		case 49:
			pbGen.SetAmount(2, FrqValue(val));
			break;
		}
	}
}

void SubSynth::Stop()
{
	envSig.Release();
	envFlt.Release();
	//filt.Release();
}

void SubSynth::Tick()
{
	if (lfoGen.On())
		osc.PhaseModWT(lfoGen.Gen() * synthParams.frqTI);
	if (pbOn)
		osc.PhaseModWT(pbGen.Gen() * synthParams.frqTI);
	AmpValue sigVal = osc.Gen();
	if (nzOn)
		sigVal = (sigVal * sigMix) + (nz.Gen() * nzMix);
	im->Output(chnl, filt->Sample(sigVal) * envSig.Gen() * vol);
}

int  SubSynth::IsFinished()
{
	return envSig.IsFinished();
}

void SubSynth::Destroy()
{
	delete this;
}

int SubSynth::Load(XmlSynthElem *parent)
{
	float dvals[7];
	short ival;

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("osc"))
		{
			if (elem->GetAttribute("frq", dvals[0]) == 0)
				osc.SetFrequency(FrqValue(dvals[0]));
			if (elem->GetAttribute("wt", ival) == 0)
				osc.SetWavetable(ival);
			if (elem->GetAttribute("vol", dvals[0]) == 0)
				vol = AmpValue(dvals[0]);
			if (elem->GetAttribute("mix", dvals[0]) == 0)
				sigMix = AmpValue(dvals[0]);
			if (elem->GetAttribute("fg",  dvals[0]) == 0)
				fltGain = AmpValue(dvals[0]);
			if (elem->GetAttribute("ft",  ival) == 0)
				fltType = ival;
			if (elem->GetAttribute("fr",  dvals[0]) == 0)
				fltRes = AmpValue(dvals[0]);

			nzMix  = 1.0 - sigMix;
			nzOn = nzMix > 0;
			CreateFilter();
		}
		else if (elem->TagMatch("egs"))
		{
			elem->GetAttribute("st",  dvals[0]);
			elem->GetAttribute("atk", dvals[1]);
			elem->GetAttribute("pk",  dvals[2]);
			elem->GetAttribute("dec", dvals[3]);
			elem->GetAttribute("sus", dvals[4]);
			elem->GetAttribute("rel", dvals[5]);
			elem->GetAttribute("end", dvals[6]);
			elem->GetAttribute("ty", ival);
			envSig.InitADSR(AmpValue(dvals[0]), 
				FrqValue(dvals[1]), AmpValue(dvals[2]),
				FrqValue(dvals[3]), AmpValue(dvals[4]),
				FrqValue(dvals[5]), AmpValue(dvals[6]),
				(EGSegType)ival);
		}
		else if (elem->TagMatch("egf"))
		{
			elem->GetAttribute("st",  dvals[0]);
			elem->GetAttribute("atk", dvals[1]);
			elem->GetAttribute("pk",  dvals[2]);
			elem->GetAttribute("dec", dvals[3]);
			elem->GetAttribute("sus", dvals[4]);
			elem->GetAttribute("rel", dvals[5]);
			elem->GetAttribute("end", dvals[6]);
			elem->GetAttribute("ty", ival);
			envFlt.InitADSR(AmpValue(dvals[0]), 
				FrqValue(dvals[1]), AmpValue(dvals[2]),
				FrqValue(dvals[3]), AmpValue(dvals[4]),
				FrqValue(dvals[5]), AmpValue(dvals[6]),
				(EGSegType)ival);
		}
		else if (elem->TagMatch("lfo"))
		{
			lfoGen.Load(elem);
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

int SubSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem = parent->AddChild("osc");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("frq",  osc.GetFrequency());
	elem->SetAttribute("wt",  (short) osc.GetWavetable());
	elem->SetAttribute("vol",  vol);
	elem->SetAttribute("mix",  sigMix);
	elem->SetAttribute("fg",   fltGain);
	elem->SetAttribute("ft",   fltType);
	elem->SetAttribute("fr",   fltRes);
	delete elem;

	elem = parent->AddChild("egs");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("st",  envSig.GetStart());
	elem->SetAttribute("atk", envSig.GetAtkRt());
	elem->SetAttribute("pk",  envSig.GetAtkLvl());
	elem->SetAttribute("dec", envSig.GetDecRt());
	elem->SetAttribute("sus", envSig.GetSusLvl());
	elem->SetAttribute("rel", envSig.GetRelRt());
	elem->SetAttribute("end", envSig.GetRelLvl());
	elem->SetAttribute("ty",  (short) envSig.GetType());
	delete elem;

	elem = parent->AddChild("egf");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("st",  envFlt.GetStart());
	elem->SetAttribute("atk", envFlt.GetAtkRt());
	elem->SetAttribute("pk",  envFlt.GetAtkLvl());
	elem->SetAttribute("dec", envFlt.GetDecRt());
	elem->SetAttribute("sus", envFlt.GetSusLvl());
	elem->SetAttribute("rel", envFlt.GetRelRt());
	elem->SetAttribute("end", envFlt.GetRelLvl());
	elem->SetAttribute("ty",  (short) envFlt.GetType());
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

