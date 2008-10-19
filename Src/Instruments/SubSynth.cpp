//////////////////////////////////////////////////////////////////////
// BasicSynth Subtractive synthesis instrument
//
// See _BasicSynth_ Chapter 21 for a full explanation
//
// This instrument contains an Oscillator, Noise source, low-pass filter,
// and envelope generators for amplitude and filter frequency.
//
// The implementation uses the DynFilterLP which combines the filter
// and envelope generator. To use another filter, un-comment the
// code for envFilt and select the appropriate type of filter for filt.
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
	SubSynthEvent *ep = new SubSynthEvent;
	return (SeqEvent *) ep;
}


SubSynth::SubSynth()
{
	vol = 1.0;
	chnl = 0;
	fltGain = 1.0;
	fltRes = 0.5;
	sigMix = 1.0;
	nzMix = 0.0;
	nzOn = 0;
	pbOn = 0;
}

SubSynth::~SubSynth()
{

}

void SubSynth::Copy(SubSynth *tp)
{
	osc.SetWavetable(tp->osc.GetWavetable());
	sigMix = tp->sigMix;
	nzMix  = 1.0 - sigMix;
	fltGain = tp->fltGain;
	fltRes = tp->fltRes;
	envSig.Copy(&tp->envSig);
	//envFilt.Copy(&tp->envFilt);
	filt.Copy(&tp->filt);
	lfoGen.Copy(&tp->lfoGen);
	nzOn = nzMix > 0;
	pbOn = tp->pbOn;
	pbGen.Copy(&tp->pbGen);
}

void SubSynth::Start(SeqEvent *evt)
{
	SetParams((SubSynthEvent *)evt);
	osc.Reset(0);
	envSig.Reset(0);
	filt.Reset(0);
	lfoGen.Reset(0);
	if (pbOn)
		pbGen.Reset(0);
}

void SubSynth::Param(SeqEvent *evt)
{
	SetParams((SubSynthEvent *)evt);
	osc.Reset(-1);
	filt.Reset(-1);
	lfoGen.Reset(-1);
}

void SubSynth::SetParams(SubSynthEvent *evt)
{
	vol = evt->vol;
	osc.SetFrequency(evt->frq);
	pbGen.SetFrequency(evt->frq);
	chnl = evt->chnl;
	bsInt16 *id = evt->idParam;
	float *valp = evt->valParam;
	float val;
	int n;
	for (n = evt->numParam; n > 0; n--)
	{
		val = *valp;
		switch (*id++)
		{
		case 16: //	Sets the mixture of oscillator output and noise output.
			sigMix = val;
			nzMix  = 1.0 - sigMix;
			nzOn = nzMix > 0;
			break;
		case 17: //Filter gain
			fltGain = val;
			break;
		case 18: //Wave table index.
			osc.SetWavetable((int) val);
			break;
		case 19: //Oscillator envelope start value.
			envSig.SetStart(AmpValue(val));
			break;
		case 20: //Oscillator envelope attack rate
			envSig.SetAtkRt(FrqValue(val));
			break;
		case 21: //Oscillator envelope peak level
			envSig.SetAtkLvl(AmpValue(val));
			break;
		case 22: //	Oscillator envelope decay rate
			envSig.SetDecRt(FrqValue(val));
			break;
		case 23: // Oscillator envelope sustain level
			envSig.SetSusLvl(AmpValue(val));
			break;
		case 24:
			envSig.SetRelRt(FrqValue(val));
			break;
		case 25: //	Oscillator envelope release level
			envSig.SetRelLvl(AmpValue(val));
			break;
		case 26: //Oscillator envelope curve type
			envSig.SetType((EGSegType) (int) val);
			break;
		case 32: //Filter envelope start value.
			filt.SetStart(AmpValue(val));
			break;
		case 33: //Filter envelope attack rate
			filt.SetAtkRt(FrqValue(val));
			break;
		case 34: //Filter envelope peak level
			filt.SetAtkLvl(AmpValue(val));
			break;
		case 35: //Filter envelope decay rate
			filt.SetDecRt(FrqValue(val));
			break;
		case 36: //Filter envelope sustain level
			filt.SetSusLvl(AmpValue(val));
			break;
		case 37: //Filter envelope release rate
			filt.SetRelRt(FrqValue(val));
			break;
		case 38: //Filter envelope final level
			filt.SetRelLvl(AmpValue(val));
			break;
		case 39: //Filter envelope curve type
			filt.SetType((EGSegType) (int) val);
			break;
		case 48: //LFO Frequency
			lfoGen.SetFrequency(FrqValue(val));
			break;
		case 49: //LFO wavetable index
			lfoGen.SetWavetable((int)val);
			break;
		case 50: //LFO envelope attack rate
			lfoGen.SetAttack(FrqValue(val));
			break;
		case 51: //LFO level
			lfoGen.SetLevel(AmpValue(val));
			break;
		case 52: // PB On
			pbOn = (int) val;
			break;
		case 53:
			pbGen.SetRate(0, FrqValue(val));
			break;
		case 54:
			pbGen.SetRate(1, FrqValue(val));
			break;
		case 55:
			pbGen.SetAmount(0, FrqValue(val));
			break;
		case 56:
			pbGen.SetAmount(1, FrqValue(val));
			break;
		case 57:
			pbGen.SetAmount(2, FrqValue(val));
			break;
		}
	}
}

void SubSynth::Stop()
{
	envSig.Release();
	//envFlt.Release();
	filt.Release();
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
	//filt.Init(envFlt.Gen(), fltGain);
	//filt.InitRes(envFlt.Gen(), fltGain, fltRes);
	im->Output(chnl, filt.Sample(sigVal) * envSig.Gen() * vol);
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
			elem->GetAttribute("frq", dvals[0]);
			elem->GetAttribute("vol", dvals[1]);
			elem->GetAttribute("mix", dvals[2]);
			elem->GetAttribute("fg",  dvals[3]);
			elem->GetAttribute("wt", ival);
			osc.InitWT(FrqValue(dvals[0]), (int)ival);
			vol = AmpValue(dvals[1]);
			sigMix = dvals[2];
			nzMix  = 1.0 - sigMix;
			nzOn = nzMix > 0;
			fltGain = dvals[3];
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
			//envFlt.InitADSR(...);
			filt.InitFilter(AmpValue(dvals[0]), 
				FrqValue(dvals[1]), AmpValue(dvals[2]),
				FrqValue(dvals[3]), AmpValue(dvals[4]),
				FrqValue(dvals[5]), AmpValue(dvals[6]),
				(EGSegType)ival, fltGain);
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
	elem->SetAttribute("vol",  vol);
	elem->SetAttribute("mix",  sigMix);
	elem->SetAttribute("fg",   fltGain);
	elem->SetAttribute("fr",   fltRes);
	elem->SetAttribute("wt",  (short) osc.GetWavetable());
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
	elem->SetAttribute("st",  filt.GetStart());
	elem->SetAttribute("atk", filt.GetAtkRt());
	elem->SetAttribute("pk",  filt.GetAtkLvl());
	elem->SetAttribute("dec", filt.GetDecRt());
	elem->SetAttribute("sus", filt.GetSusLvl());
	elem->SetAttribute("rel", filt.GetRelRt());
	elem->SetAttribute("end", filt.GetRelLvl());
	elem->SetAttribute("ty",  (short) filt.GetType());
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

