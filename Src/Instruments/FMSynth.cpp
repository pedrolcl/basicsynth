//////////////////////////////////////////////////////////////////////
// BasicSynth FM Synthesis instrument
//
// See _BasicSynth_ Chapter 22 for a full explanation
//
// The FM Synth instrument implements a three oscillator FM synthesis
// method. The "algorithms" include:
//   - one modulator, one carrier stack
//   - two modulator, one carrier stack
//   - two modulator, one carrier "Y"
//   - one modulator, two carrier "Delta"
// LFO and pitch bend can be optionally applied to the signal
// A noise generator can be summed with the FM signal
// to produce noisy transient sounds.
// A delay line is available to add resonance.
// Panning can be done internally rather than through the mixer
// 
// Copyright 2008, Daniel R. Mitchell
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "FMSynth.h"

Instrument *FMSynth::FMSynthFactory(InstrManager *m, Opaque tmplt)
{
	FMSynth *ip = new FMSynth;
	ip->im = m;
	if (tmplt)
		ip->Copy((FMSynth *) tmplt);
	return ip;
}

SeqEvent *FMSynth::FMSynthEventFactory(Opaque tmplt)
{
	FMSynth *ip = (FMSynth *)tmplt;
	VarParamEvent *vpe = new VarParamEvent;
	vpe->maxParam = 72;
	vpe->pitch = 67;
	vpe->frq = 440.0;
	vpe->vol  = 1.0;
	return (SeqEvent *) vpe;
}

FMSynth::FMSynth()
{
	chnl = 0;
	frq = 440.0;
	vol = 1.0;
	gen1EnvDef.Alloc(3, 0, 1);
	gen2EnvDef.Alloc(3, 0, 1);
	gen3EnvDef.Alloc(3, 0, 1);
	nzEnvDef.Alloc(3, 0, 1);
	int i;
	for (i = 0; i < 3; i++)
	{
		gen1EnvDef.Set(i, 0, 0, linSeg);
		gen2EnvDef.Set(i, 0, 0, linSeg);
		gen3EnvDef.Set(i, 0, 0, linSeg);
		nzEnvDef.Set(i, 0, 0, linSeg);
	}
	maxPhs = synthParams.ftableLength / 2;
	gen1Mult = 1.0;
	gen2Mult = 1.0;
	gen3Mult = 2.0;
	fmMix = 1.0;
	fmDly = 0.0;
	nzMix = 0.0;
	nzDly = 0.0;
	nzFrqh = 400.0;
	nzFrqo = 400.0;
	nzOn = 0;
	dlyMix = 0.0;
	dlyOn = 0;
	dlyTim = 0.01;
	dlyDec = 0.1;
	dlySamps = 0;
	panOn  = 0;
	pbOn = 0;
	panSet = 0.0;
	panLft = 0.5;
	panRgt = 0.5;
}

FMSynth::~FMSynth()
{
	gen1EnvDef.Clear();
	gen2EnvDef.Clear();
	gen3EnvDef.Clear();
	nzEnvDef.Clear();
}

void FMSynth::Copy(FMSynth *ip)
{
	gen1Osc.SetFrequency(ip->gen1Osc.GetFrequency());
	gen1Osc.SetWavetable(ip->gen1Osc.GetWavetable());
	gen1EnvDef.Copy(&ip->gen1EnvDef);
	gen1Mult = ip->gen1Mult;
	fmMix = ip->fmMix;
	algorithm = ip->algorithm;

	gen2Osc.SetFrequency(ip->gen2Osc.GetFrequency());
	gen2Osc.SetWavetable(ip->gen2Osc.GetWavetable());
	gen2EnvDef.Copy(&ip->gen2EnvDef);
	gen2Mult = ip->gen2Mult;

	gen3Osc.SetFrequency(ip->gen3Osc.GetFrequency());
	gen3Osc.SetWavetable(ip->gen3Osc.GetWavetable());
	gen3EnvDef.Copy(&ip->gen3EnvDef);
	gen3Mult = ip->gen3Mult;

	nzEnvDef.Copy(&ip->nzEnvDef);
	nzMix = ip->nzMix;
	nzFrqh = ip->nzFrqh;
	nzFrqo = ip->nzFrqo;
	nzOn = ip->nzOn;

	dlyTim = ip->dlyTim;
	dlyDec = ip->dlyDec;
	dlyMix = ip->dlyMix;
	dlySamps = ip->dlySamps;
	dlyOn = ip->dlyOn;
	
	lfoGen.Copy(&ip->lfoGen);
	pbGen.Copy(&ip->pbGen);

	chnl = ip->chnl;
	frq = ip->frq;
	vol = ip->vol;
}

AmpValue FMSynth::CalcPhaseMod(AmpValue amp, FrqValue mult)
{
	amp = (amp * mult) * synthParams.frqTI;
	if (amp > maxPhs)
		amp = maxPhs;
	return amp;
}

void FMSynth::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);
	FrqValue mul1 = gen2Mult * frq;
	FrqValue mul2 = gen3Mult * frq;
	gen1Osc.InitWT(frq*gen1Mult, WT_SIN);
	gen2Osc.InitWT(mul1, WT_SIN);
	gen3Osc.InitWT(mul2, WT_SIN);
	gen1EG.SetEnvDef(&gen1EnvDef);
	gen1EG.Reset(0);
	if (algorithm != ALG_DELTA)
	{
		gen2EG.InitADSR(CalcPhaseMod(gen2EnvDef.start, mul1),
			gen2EnvDef.GetRate(0), CalcPhaseMod(gen2EnvDef.GetLevel(0),  mul1),
			gen2EnvDef.GetRate(1), CalcPhaseMod(gen2EnvDef.GetLevel(1),  mul1),
			gen2EnvDef.GetRate(2), CalcPhaseMod(gen2EnvDef.GetLevel(2),  mul1),
			gen2EnvDef.GetType(0));
	}
	else
	{
		// double carrier
		gen2EG.SetEnvDef(&gen2EnvDef);
		gen2EG.Reset(0);
	}
	if (algorithm != ALG_STACK)
	{
		gen3EG.InitADSR(CalcPhaseMod(gen3EnvDef.start, mul2),
			gen3EnvDef.GetRate(0), CalcPhaseMod(gen3EnvDef.GetLevel(0),  mul2),
			gen3EnvDef.GetRate(1), CalcPhaseMod(gen3EnvDef.GetLevel(1),  mul2),
			gen3EnvDef.GetRate(2), CalcPhaseMod(gen3EnvDef.GetLevel(2),  mul2),
			gen3EnvDef.GetType(0));
	}

	nzOn = nzMix > 0;
	if (nzOn)
	{
		nzi.InitH(nzFrqh);
		nzo.InitWT(nzFrqo, WT_SIN);
		nzEG.SetEnvDef(&nzEnvDef);
		nzEG.Reset(0);
	}
	dlyOn = dlyMix > 0 && (fmDly > 0 || nzDly > 0);
	if (dlyOn)
		apd.InitDLR(dlyTim, dlyDec, 0.001);
	lfoGen.SetSigFrq(frq);
	lfoGen.Reset();
	if (pbOn)
	{
		pbGen.SetFrequency(frq);
		pbGen.Reset();
	}
}

void FMSynth::Param(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);
	// The only changeable things are the oscillators, i.e. pitch
	// and signal/noise/delay mixture.
	// Envelope rates and levels are not reset while playing.
	// changing the 'algorithm' while playing is an "interseting" idea...
	// most likely will produce unpredictable behavior.
	gen1Osc.SetFrequency(frq*gen1Mult);
	gen2Osc.SetFrequency(frq*gen2Mult);
	gen3Osc.SetFrequency(frq*gen3Mult);
	gen1Osc.Reset(-1);
	gen2Osc.Reset(-1);
	gen3Osc.Reset(-1);
	lfoGen.SetSigFrq(frq);
	lfoGen.Reset(-1);
	if (nzOn)
	{
		nzi.Reset(-1);
		nzo.Reset(-1);
	}
}

void FMSynth::SetParams(VarParamEvent *evt)
{
	vol = evt->vol;
	frq = evt->frq;
	chnl = evt->chnl;
	bsInt16 *id = evt->idParam;
	float *valp = evt->valParam;
	float val;
	EGSegType segType;
	int n;
	for (n = evt->numParam; n > 0; n--)
	{
		val = *valp++;
		switch (*id++)
		{
		case 16: //mix
			fmMix = val;
			break;
		case 17: //dly
			fmDly = val;
			break;
		case 18: //alg
			algorithm = (bsInt16) val;
			break;
		case 19:
			panOn = (bsInt16) val;
			break;
		case 20:
			panSet = AmpValue(val);
			panLft = panSet;
			panRgt = panSet;
			break;
		case 30: //mul
			gen1Mult = val;
			break;
		case 31: //st
			gen1EnvDef.SetStart(AmpValue(val));
			break;
		case 32: //atk
			gen1EnvDef.SetRate(0, FrqValue(val));
			break;
		case 33: //pk
			gen1EnvDef.SetLevel(0, AmpValue(val));
			break;
		case 34: //dec
			gen1EnvDef.SetRate(1, FrqValue(val));
			break;
		case 35: //sus
			gen1EnvDef.SetLevel(1, AmpValue(val));
			break;
		case 36: //rel
			gen1EnvDef.SetRate(2, FrqValue(val));
			break;
		case 37: //end
			gen1EnvDef.SetLevel(2, AmpValue(val));
			break;
		case 38: //ty
			segType = (EGSegType)(int)val;
			gen1EnvDef.SetType(0, segType);
			gen1EnvDef.SetType(1, segType);
			gen1EnvDef.SetType(2, segType);
			break;
		//gen2: 
		case 40: //mul
			gen2Mult = val;
			break;
		case 41: //st
			gen2EnvDef.SetStart(AmpValue(val));
			break;
		case 42: //atk
			gen2EnvDef.SetRate(0, FrqValue(val));
			break;
		case 43: //pk
			gen2EnvDef.SetLevel(0, AmpValue(val));
			break;
		case 44: //dec
			gen2EnvDef.SetRate(1, FrqValue(val));
			break;
		case 45: //sus
			gen2EnvDef.SetLevel(1, AmpValue(val));
			break;
		case 46: //rel
			gen2EnvDef.SetRate(2, FrqValue(val));
			break;
		case 47: //end
			gen2EnvDef.SetLevel(2, AmpValue(val));
			break;
		case 48: //ty
			segType = (EGSegType)(int)val;
			gen2EnvDef.SetType(0, segType);
			gen2EnvDef.SetType(1, segType);
			gen2EnvDef.SetType(2, segType);
			break;
		// gen3:
		case 50: //mul
			gen3Mult = val;
			break;
		case 51: //st
			gen3EnvDef.SetStart(AmpValue(val));
			break;
		case 52: //atk
			gen3EnvDef.SetRate(0, FrqValue(val));
			break;
		case 53: //pk
			gen3EnvDef.SetLevel(0, AmpValue(val));
			break;
		case 54: //dec
			gen3EnvDef.SetRate(1, FrqValue(val));
			break;
		case 55: //sus
			gen3EnvDef.SetLevel(1, AmpValue(val));
			break;
		case 56: //rel
			gen3EnvDef.SetRate(2, FrqValue(val));
			break;
		case 57: //end
			gen3EnvDef.SetLevel(2, AmpValue(val));
			break;
		case 58: //ty
			segType = (EGSegType)(int)val;
			gen3EnvDef.SetType(0, segType);
			gen3EnvDef.SetType(1, segType);
			gen3EnvDef.SetType(2, segType);
			break;
		//nz: 
		case 60: //mix
			nzMix = val;
			break;
		case 61: //dly
			nzDly = val;
			break;
		case 62: //nz frq
			nzFrqh = FrqValue(val);
			break;
		case 63: //osc frq
			nzFrqo = FrqValue(val);
			break;
		case 64: //st
			nzEnvDef.SetStart(AmpValue(val));
			break;
		case 65: //atk
			nzEnvDef.SetRate(0, FrqValue(val));
			break;
		case 66: //pk
			nzEnvDef.SetLevel(0, AmpValue(val));
			break;
		case 67: //dec
			nzEnvDef.SetRate(1, FrqValue(val));
			break;
		case 68: //sus
			nzEnvDef.SetLevel(1, AmpValue(val));
			break;
		case 69: //rel
			nzEnvDef.SetRate(2, FrqValue(val));
			break;
		case 70: //end
			nzEnvDef.SetLevel(2, AmpValue(val));
			break;
		case 71: //ty
			segType = (EGSegType)(int)val;
			nzEnvDef.SetType(0, segType);
			nzEnvDef.SetType(1, segType);
			nzEnvDef.SetType(2, segType);
			break;
		//dlyn: 
		case 80: //mix
			dlyMix = val;
			break;
		case 81: //dly
			dlyTim = val;
			break;
		case 82: //dec
			dlyDec = val;
			break;
		//lfo: 
		case 90: //frq
			lfoGen.SetFrequency(FrqValue(val));
			break;
		case 91: //wt
			lfoGen.SetWavetable((int)val);
			break;
		case 92: //rt
			lfoGen.SetAttack(FrqValue(val));
			break;
		case 93: //amp
			lfoGen.SetLevel(AmpValue(val));
			break;
		// pitchbend
		case 100:
			pbOn = (int) val;
			break;
		case 101:
			pbGen.SetRate(0, FrqValue(val));
			break;
		case 102:
			pbGen.SetRate(1, FrqValue(val));
			break;
		case 103:
			pbGen.SetAmount(0, FrqValue(val));
			break;
		case 104:
			pbGen.SetAmount(1, FrqValue(val));
			break;
		case 105:
			pbGen.SetAmount(2, FrqValue(val));
			break;
		default:
			break;
		}
	}
}

void FMSynth::Stop()
{
	gen1EG.Release();
	gen2EG.Release();
	gen3EG.Release();
	if (nzOn)
		nzEG.Release();
}


int FMSynth::IsFinished()
{
	if (gen1EG.IsFinished())
	{
		if (!dlyOn || --dlySamps <= 0)
			return 1;
	}
	return 0;
}

void FMSynth::Tick()
{
	AmpValue sigOut;
	AmpValue gen1Out;
	AmpValue gen2Out;
	AmpValue gen3Out;
	AmpValue nzOut;
	AmpValue dlyOut;
	AmpValue lfoOut;
	AmpValue gen1Mod;
	AmpValue gen2Mod;
	AmpValue gen3Mod;

	int lfoOn = lfoGen.On();
	if (lfoOn)
	{
		lfoOut = lfoGen.Gen() * synthParams.frqTI;
		if (pbOn)
			lfoOut += pbGen.Gen() * synthParams.frqTI;
		gen3Mod = lfoOut * gen3Mult;
		gen2Mod = lfoOut * gen2Mult;
		gen1Mod = lfoOut * gen1Mult;
	}
	else if (pbOn)
	{
		lfoOut = pbGen.Gen() * synthParams.frqTI;
		gen3Mod = lfoOut * gen3Mult;
		gen2Mod = lfoOut * gen2Mult;
		gen1Mod = lfoOut * gen1Mult;
	}
	else
	{
		gen3Mod = 0;
		gen2Mod = 0;
		gen1Mod = 0;
	}

	gen1Out = gen1Osc.Gen() * gen1EG.Gen();
	gen2Out = gen2Osc.Gen() * gen2EG.Gen();
	gen3Out = gen3Osc.Gen() * gen3EG.Gen();
	switch (algorithm)
	{
	case ALG_STACK2:
		gen2Mod += gen3Out;
		// fallthrough
	case ALG_STACK:
		gen1Mod += gen2Out;
		break;
	case ALG_WYE:
		gen1Mod += gen2Out + gen3Out;
		break;
	case ALG_DELTA:
		gen1Mod += gen3Out;
		gen2Mod += gen3Out;
		gen1Out += gen2Out;
		break;
	}
	gen1Osc.PhaseModWT(gen1Mod);
	gen2Osc.PhaseModWT(gen2Mod);
	gen3Osc.PhaseModWT(gen3Mod);

	sigOut = gen1Out * fmMix;

	if (nzOn) 
	{
		nzOut = nzi.Gen() * nzo.Gen() * nzEG.Gen();
		sigOut += nzOut * nzMix;
	}
	
	if (dlyOn)
	{
		AmpValue dlyIn = gen1Out * fmDly;
		if (nzOn)
			dlyIn += nzOut * nzDly;
		dlyOut = apd.Sample(dlyIn);
		sigOut += dlyOut * dlyMix;
	}

	sigOut *= vol;
	if (panOn)
		im->Output2(chnl, sigOut * panLft, sigOut * panRgt);
	else
		im->Output(chnl, sigOut);
}

void FMSynth::Destroy()
{
	delete this;
}

void FMSynth::LoadEG(XmlSynthElem *elem, EnvDef& eg)
{
	float rt = 0;
	float lvl = 0;
	short typ = 0;

	elem->GetAttribute("st", lvl);
	elem->GetAttribute("ty", typ);
	eg.start = lvl;
	elem->GetAttribute("atk", rt);
	elem->GetAttribute("pk",  lvl);
	eg.Set(0, rt, lvl, (EGSegType)typ);
	elem->GetAttribute("dec", rt);
	elem->GetAttribute("sus", lvl);
	eg.Set(1, rt, lvl, (EGSegType)typ);
	elem->GetAttribute("rel", rt);
	elem->GetAttribute("end", lvl);
	eg.Set(2, rt, lvl, (EGSegType)typ);
}

int FMSynth::Load(XmlSynthElem *parent)
{
	float dval;
	short ival;

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("fm"))
		{
			if (elem->GetAttribute("alg", ival) == 0)
				algorithm = (bsInt16) ival;
			if (elem->GetAttribute("mix", dval) == 0)
				fmMix = dval;
			if (elem->GetAttribute("dly", dval) == 0)
				fmDly = dval;
		}
		if (elem->TagMatch("gen1"))
		{
			if (elem->GetAttribute("mul", dval) == 0)
				gen1Mult = dval;
			LoadEG(elem, gen1EnvDef);
		}
		else if (elem->TagMatch("gen2"))
		{
			if (elem->GetAttribute("mul", dval) == 0)
				gen2Mult = dval;
			LoadEG(elem, gen2EnvDef);
		}
		else if (elem->TagMatch("gen3"))
		{
			if (elem->GetAttribute("mul", dval) == 0)
				gen3Mult = dval;
			LoadEG(elem, gen3EnvDef);
		}
		else if (elem->TagMatch("nz"))
		{
			if (elem->GetAttribute("mix", dval) == 0)
				nzMix = dval;
			if (elem->GetAttribute("dly", dval) == 0)
				nzDly = dval;
			if (elem->GetAttribute("fr", dval) == 0)
				nzFrqh = dval;
			if (elem->GetAttribute("fo", dval) == 0)
				nzFrqo = dval;
			LoadEG(elem, nzEnvDef);
		}
		else if (elem->TagMatch("dln"))
		{
			if (elem->GetAttribute("mix", dval) == 0)
				dlyMix = dval;
			if (elem->GetAttribute("dly", dval) == 0)
				dlyTim = dval;
			if (elem->GetAttribute("dec", dval) == 0)
				dlyDec = dval;
			dlySamps = (long) (dlyDec * synthParams.sampleRate);
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

XmlSynthElem *FMSynth::SaveEG(XmlSynthElem *parent, char *tag, EnvDef& eg)
{
	XmlSynthElem *elem = parent->AddChild(tag);
	if (elem != NULL)
	{
		elem->SetAttribute("st",  eg.start);
		elem->SetAttribute("atk", eg.segs[0].rate);
		elem->SetAttribute("pk",  eg.segs[0].level);
		elem->SetAttribute("dec", eg.segs[1].rate);
		elem->SetAttribute("sus", eg.segs[1].level);
		elem->SetAttribute("rel", eg.segs[2].rate);
		elem->SetAttribute("end", eg.segs[2].level);
		elem->SetAttribute("ty", (short) eg.segs[0].type);
	}
	return elem;
}

int FMSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem;
	
	elem = parent->AddChild("fm");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mix", fmMix);
	elem->SetAttribute("alg", (short) algorithm);
	elem->SetAttribute("dly", fmDly);
	delete elem;

	elem = SaveEG(parent, "gen1", gen1EnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mul", gen1Mult);
	delete elem;

	elem = SaveEG(parent, "gen2", gen2EnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mul", gen2Mult);
	delete elem;

	elem = SaveEG(parent, "gen3", gen3EnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mul", gen3Mult);
	delete elem;

	elem = SaveEG(parent, "nz", nzEnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mix", nzMix);
	elem->SetAttribute("dly", nzDly);
	elem->SetAttribute("fr", nzFrqh);
	elem->SetAttribute("fo", nzFrqo);
	delete elem;

	elem = parent->AddChild("dln");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mix", dlyMix);
	elem->SetAttribute("dly", dlyTim);
	elem->SetAttribute("dec", dlyDec);
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

