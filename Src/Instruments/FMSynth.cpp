// FMSynth.cpp: implementation of the FMSynth class.
//
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "FMSynth.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

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
	carEnvDef.Alloc(3, 0, 1);
	m1EnvDef.Alloc(3, 0, 1);
	m2EnvDef.Alloc(3, 0, 1);
	nzEnvDef.Alloc(3, 0, 1);
	filtEnvDef.Alloc(3, 0, 1);
	maxPhs = synthParams.ftableLength / 2;
	carMult = 1.0;
	m1Mult = 1.0;
	m2Mult = 2.0;
	fmMix = 1.0;
	fmDly = 0.0;
	nzMix = 0.0;
	nzDly = 0.0;
	nzOn = 0;
	dlyMix = 0.0;
	dlyOn = 0;
	dlyTim = 0.01;
	dlyDec = 0.1;
	dlySamps = 0;
	filtGain = 1.0;
}

FMSynth::~FMSynth()
{
	carEnvDef.Clear();
	m1EnvDef.Clear();
	m2EnvDef.Clear();
	nzEnvDef.Clear();
	filtEnvDef.Clear();
}

void FMSynth::Copy(FMSynth *ip)
{
	carOsc.SetFrequency(ip->carOsc.GetFrequency());
	carOsc.SetWavetable(ip->carOsc.GetWavetable());
	carEnvDef.Copy(&ip->carEnvDef);
	carMult = ip->carMult;
	fmMix = ip->fmMix;
	algorithm = ip->algorithm;

	m1Osc.SetFrequency(ip->m1Osc.GetFrequency());
	m1Osc.SetWavetable(ip->m1Osc.GetWavetable());
	m1EnvDef.Copy(&ip->m1EnvDef);
	m1Mult = ip->m1Mult;

	m2Osc.SetFrequency(ip->m2Osc.GetFrequency());
	m2Osc.SetWavetable(ip->m2Osc.GetWavetable());
	m2EnvDef.Copy(&ip->m2EnvDef);
	m2Mult = ip->m2Mult;

	nzEnvDef.Copy(&ip->nzEnvDef);
	nzMix = ip->nzMix;
	nzOn = ip->nzOn;

	filtEnvDef.Copy(&ip->filtEnvDef);
	
	dlyTim = ip->dlyTim;
	dlyDec = ip->dlyDec;
	dlyMix = ip->dlyMix;
	dlySamps = ip->dlySamps;
	dlyOn = ip->dlyOn;
	
	lfoGen.Copy(&ip->lfoGen);

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
	carOsc.InitWT(frq*carMult, WT_SIN);
	FrqValue mul1 = m1Mult * frq;
	FrqValue mul2 = m2Mult * frq;
	m1Osc.InitWT(mul1, WT_SIN);
	m2Osc.InitWT(mul2, WT_SIN);
	carEG.SetEnvDef(&carEnvDef);
	carEG.Reset(0);
	m1EG.InitADSR(CalcPhaseMod(m1EnvDef.start, mul1),
		m1EnvDef.GetRate(0), CalcPhaseMod(m1EnvDef.GetLevel(0),  mul1),
		m1EnvDef.GetRate(1), CalcPhaseMod(m1EnvDef.GetLevel(1),  mul1),
		m1EnvDef.GetRate(2), CalcPhaseMod(m1EnvDef.GetLevel(2),  mul1),
		m1EnvDef.GetType(0));
	m2EG.InitADSR(CalcPhaseMod(m2EnvDef.start, mul2),
		m2EnvDef.GetRate(0), CalcPhaseMod(m2EnvDef.GetLevel(0),  mul2),
		m2EnvDef.GetRate(1), CalcPhaseMod(m2EnvDef.GetLevel(1),  mul2),
		m2EnvDef.GetRate(2), CalcPhaseMod(m2EnvDef.GetLevel(2),  mul2),
		m2EnvDef.GetType(0));
	filtEG.SetEnvDef(&filtEnvDef);
	filtEG.Reset(0);
	nzOn = nzMix > 0;
	if (nzOn)
	{
		nzEG.SetEnvDef(&nzEnvDef);
		nzEG.Reset(0);
	}
	dlyOn = dlyMix > 0 && (fmDly > 0 || nzDly > 0);
	if (dlyOn)
		apd.InitDLR(dlyTim, dlyDec, 0.001);
	lfoGen.Reset();
}

void FMSynth::Param(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);
	// The only changeable things are the oscillators, i.e. pitch
	// and signal/noise/delay mixture.
	// Envelope rates and levels are not reset while playing.
	// changing the 'algorithm' while playing is an "interseting" idea...
	// most likely will produce unpredictable behavior.
	carOsc.Reset(-1);
	m1Osc.Reset(-1);
	m2Osc.Reset(-1);
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
		val = *valp;
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
		case 19: //mul
			carMult = val;
			break;
		case 20: //st
			carEnvDef.SetStart(AmpValue(val));
			break;
		case 21: //atk
			carEnvDef.SetRate(0, FrqValue(val));
			break;
		case 22: //pk
			carEnvDef.SetLevel(0, AmpValue(val));
			break;
		case 23: //dec
			carEnvDef.SetRate(1, FrqValue(val));
			break;
		case 24: //sus
			carEnvDef.SetLevel(1, AmpValue(val));
			break;
		case 25: //rel
			carEnvDef.SetRate(2, FrqValue(val));
			break;
		case 26: //end
			carEnvDef.SetLevel(2, AmpValue(val));
			break;
		case 27: //ty
			segType = (EGSegType)(int)val;
			carEnvDef.SetType(0, segType);
			carEnvDef.SetType(1, segType);
			carEnvDef.SetType(2, segType);
			break;
		//mod1: 
		case 32: //mul
			m1Mult = val;
			break;
		case 33: //st
			m1EnvDef.SetStart(AmpValue(val));
			break;
		case 34: //atk
			m1EnvDef.SetRate(0, FrqValue(val));
			break;
		case 35: //pk
			m1EnvDef.SetLevel(0, AmpValue(val));
			break;
		case 36: //dec
			m1EnvDef.SetRate(1, FrqValue(val));
			break;
		case 37: //sus
			m1EnvDef.SetLevel(1, AmpValue(val));
			break;
		case 38: //rel
			m1EnvDef.SetRate(2, FrqValue(val));
			break;
		case 39: //end
			m1EnvDef.SetLevel(2, AmpValue(val));
			break;
		case 40: //ty
			segType = (EGSegType)(int)val;
			m1EnvDef.SetType(0, segType);
			m1EnvDef.SetType(1, segType);
			m1EnvDef.SetType(2, segType);
			break;
		// mod2:
		case 48: //mul
			m2Mult = val;
			break;
		case 49: //st
			m2EnvDef.SetStart(AmpValue(val));
			break;
		case 50: //atk
			m2EnvDef.SetRate(0, FrqValue(val));
			break;
		case 51: //pk
			m2EnvDef.SetLevel(0, AmpValue(val));
			break;
		case 52: //dec
			m2EnvDef.SetRate(1, FrqValue(val));
			break;
		case 53: //sus
			m2EnvDef.SetLevel(1, AmpValue(val));
			break;
		case 54: //rel
			m2EnvDef.SetRate(2, FrqValue(val));
			break;
		case 55: //end
			m2EnvDef.SetLevel(2, AmpValue(val));
			break;
		case 56: //ty
			segType = (EGSegType)(int)val;
			m2EnvDef.SetType(0, segType);
			m2EnvDef.SetType(1, segType);
			m2EnvDef.SetType(2, segType);
			break;
		//nz: 
		case 64: //mix
			nzMix = val;
			break;
		case 65: //dly
			nzDly = val;
			break;
		case 66: //st
			nzEnvDef.SetStart(AmpValue(val));
			break;
		case 67: //atk
			nzEnvDef.SetRate(0, FrqValue(val));
			break;
		case 68: //pk
			nzEnvDef.SetLevel(0, AmpValue(val));
			break;
		case 69: //dec
			nzEnvDef.SetRate(1, FrqValue(val));
			break;
		case 70: //sus
			nzEnvDef.SetLevel(1, AmpValue(val));
			break;
		case 71: //rel
			nzEnvDef.SetRate(2, FrqValue(val));
			break;
		case 72: //end
			nzEnvDef.SetLevel(2, AmpValue(val));
			break;
		case 73: //ty
			segType = (EGSegType)(int)val;
			nzEnvDef.SetType(0, segType);
			nzEnvDef.SetType(1, segType);
			nzEnvDef.SetType(2, segType);
			break;
		//filt: 
		case 80: //st
			filtEnvDef.SetStart(AmpValue(val));
			break;
		case 81: //atk
			filtEnvDef.SetRate(0, FrqValue(val));
			break;
		case 82: //pk
			filtEnvDef.SetLevel(0, AmpValue(val));
			break;
		case 83: //dec
			filtEnvDef.SetRate(1, FrqValue(val));
			break;
		case 84: //sus
			filtEnvDef.SetLevel(1, AmpValue(val));
			break;
		case 85: //rel
			filtEnvDef.SetRate(2, FrqValue(val));
			break;
		case 86: //end
			filtEnvDef.SetLevel(2, AmpValue(val));
			break;
		case 87: //ty
			segType = (EGSegType)(int)val;
			filtEnvDef.SetType(0, segType);
			filtEnvDef.SetType(1, segType);
			filtEnvDef.SetType(2, segType);
			break;
		//dlyn: 
		case 96: //mix
			dlyMix = val;
			break;
		case 97: //dly
			dlyTim = val;
			break;
		case 98: //dec
			dlyDec = val;
			break;
		//lfo: 
		case 112: //frq
			lfoGen.SetFrequency(FrqValue(val));
			break;
		case 113: //wt
			lfoGen.SetWavetable((int)val);
			break;
		case 114: //rt
			lfoGen.SetAttack(FrqValue(val));
			break;
		case 115: //amp
			lfoGen.SetLevel(AmpValue(val));
			break;
		default:
			break;
		}
	}
}

void FMSynth::Stop()
{
	carEG.Release();
	m1EG.Release();
	m2EG.Release();
	if (nzOn)
	{
		nzEG.Release();
		filtEG.Release();
	}
}


int FMSynth::IsFinished()
{
	if (carEG.IsFinished())
	{
		if (!dlyOn || --dlySamps <= 0)
			return 1;
	}
	return 0;
}

void FMSynth::Tick()
{
	AmpValue sigOut;
	AmpValue carOut;
	AmpValue m1Out;
	AmpValue m2Out;
	AmpValue nzOut;
	AmpValue dlyOut;
	AmpValue lfoOut;
	AmpValue m1Mod;
	AmpValue carMod;

	int lfoOn = lfoGen.On();
	if (lfoOn)
	{
		lfoOut = lfoGen.Gen() * synthParams.frqTI;
		m1Mod = lfoOut * m1Mult;
		carMod = lfoOut;
	}
	else
	{
		//lfoOut = 0;
		m1Mod = 0;
		carMod = 0;
	}

	if (algorithm != ALG_STACK) 
	{
		if (lfoOn)
			m2Osc.PhaseModWT(lfoOut * m2Mult);
		m2Out = m2EG.Gen() * m2Osc.Gen();
		if (algorithm != ALG_WYE)
			m1Mod += m2Out;
		if (algorithm != ALG_STACK2)
			carMod += m2Out;
	}
	m1Osc.PhaseModWT(m1Mod);
	m1Out = m1Osc.Gen() * m1EG.Gen();

	carOsc.PhaseModWT(carMod + m1Out);
	carOut = carOsc.Gen() * carEG.Gen();
	sigOut = carOut * fmMix;

	if (nzOn) 
	{
		filt.Init(filtEG.Gen(), filtGain);
		nzOut = filt.Sample(nz.Gen()) * nzEG.Gen();
		sigOut += nzOut * nzMix;
	}
	
	if (dlyOn)
	{
		AmpValue dlyIn = carOut * fmDly;
		if (nzOn)
			dlyIn += nzOut *+ nzDly;
		dlyOut = apd.Sample(dlyIn);
		sigOut += dlyOut * dlyMix;
	}

	im->Output(chnl, sigOut * vol);
}

void FMSynth::Destroy()
{
	delete this;
}

void FMSynth::LoadEG(XmlSynthElem *elem, EnvDef& eg)
{
	double rt = 0;
	double lvl = 0;
	long typ = 0;

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
	double dval;
	long ival;

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("car"))
		{
			if (elem->GetAttribute("alg", ival) == 0)
				algorithm = (bsInt16) ival;
			if (elem->GetAttribute("mix", dval) == 0)
				fmMix = dval;
			if (elem->GetAttribute("dly", dval) == 0)
				fmDly = dval;
			if (elem->GetAttribute("mul", dval) == 0)
				carMult = dval;
			LoadEG(elem, carEnvDef);
		}
		else if (elem->TagMatch("mod1"))
		{
			if (elem->GetAttribute("mul", dval) == 0)
				m1Mult = dval;
			LoadEG(elem, m1EnvDef);
		}
		else if (elem->TagMatch("mod2"))
		{
			if (elem->GetAttribute("mul", dval) == 0)
				m2Mult = dval;
			LoadEG(elem, m2EnvDef);
		}
		else if (elem->TagMatch("nz"))
		{
			if (elem->GetAttribute("mix", dval) == 0)
				nzMix = dval;
			if (elem->GetAttribute("dly", dval) == 0)
				nzDly = dval;
			LoadEG(elem, nzEnvDef);
		}
		else if (elem->TagMatch("filt"))
		{
			if (elem->GetAttribute("fg", dval) == 0)
				filtGain = dval;
			LoadEG(elem, filtEnvDef);
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
		elem->SetAttribute("ty", (long) eg.segs[0].type);
	}
	return elem;
}

int FMSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem;
	
	elem = SaveEG(parent, "car", carEnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mix", (double) fmMix);
	elem->SetAttribute("alg", (long) algorithm);
	elem->SetAttribute("mul", (double) carMult);
	delete elem;

	elem = SaveEG(parent, "mod1", m1EnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mul", (double) m1Mult);
	delete elem;

	elem = SaveEG(parent, "mod2", m2EnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mul", (double) m2Mult);
	delete elem;

	elem = SaveEG(parent, "nz", m2EnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mix", (double) nzMix);
	delete elem;

	elem = SaveEG(parent, "filt", m2EnvDef);
	if (elem == NULL)
		return -1;
	//elem->SetAttribute("fg", (double) filtGain);
	delete elem;

	elem = parent->AddChild("dln");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mix", (double) dlyMix);
	elem->SetAttribute("dly", (double) dlyTim);
	elem->SetAttribute("dec", (double) dlyDec);
	delete elem;

	elem = parent->AddChild("lfo");
	if (elem == NULL)
		return -1;
	lfoGen.Save(elem);
	delete elem;

	return 0;
}

