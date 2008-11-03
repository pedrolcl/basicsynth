//////////////////////////////////////////////////////////////////////
// BasicSynth Matrix Synthesis instrument
//
// See _BasicSynth_ Chapter 24 for a full description
//
// The Matrix synth instrument combines 8 oscillators with 8 envelope
// generators in a configurable matrix. Any oscillator can function as
// a signal output and/or modulator. Separate amplitude scaling is
// configured for signal and modulation levels. Any envelope generator
// can be applied to any number of oscillators. Single LFO and pitch bend are
// built-in as well and can be applied individually to each oscillator.
// 
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "MatrixSynth.h"

Instrument *MatrixSynth::MatrixSynthFactory(InstrManager *m, Opaque tmplt)
{
	MatrixSynth *ip;
	if (tmplt)
		ip = new MatrixSynth((MatrixSynth*)tmplt);
	else
		ip = new MatrixSynth;
	if (ip)
		ip->im = m;
	return ip;
}

SeqEvent   *MatrixSynth::MatrixSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 16384;
	return (SeqEvent *) ep;
}

static PhsAccum maxPhs;

MatrixSynth::MatrixSynth()
{
	im = NULL;
	frq = 440.0;
	vol = 1.0;
	chnl = 0;
	maxPhs = synthParams.ftableLength/2;
	lfoOn = 0;
	panOn = 0;
	pbOn = 0;
	fx1On = 0;
	fx2On = 0;
	fx3On = 0;
	fx4On = 0;
	panOn = 0;
	allFlags = 0;
	envUsed = 0;
	for (int n = 0; n < MATGEN; n++)
		envs[n].SetSegs(1);
}

MatrixSynth::MatrixSynth(MatrixSynth *tp)
{
	im = NULL;
	Copy(tp);
}

MatrixSynth::~MatrixSynth()
{
}

void MatrixSynth::Copy(MatrixSynth *tp)
{
	chnl = tp->chnl;
	frq = tp->frq;
	vol = tp->vol;

	lfoOn = tp->lfoOn;
	pbOn  = tp->pbOn;
	panOn = tp->panOn;
	fx1On = tp->fx1On;
	fx2On = tp->fx2On;
	fx3On = tp->fx3On;
	fx4On = tp->fx4On;
	panOn = tp->panOn;
	allFlags = tp->allFlags;
	envUsed = tp->envUsed;

	for (int n = 0; n < MATGEN; n++)
	{
		gens[n].Copy(&tp->gens[n]);
		envs[n].Copy(&tp->envs[n]);
	}

	lfoGen.Copy(&tp->lfoGen);
	pbGen.Copy(&tp->pbGen);
}

void MatrixSynth::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);

	allFlags = 0;
	envUsed = 0;
	MatrixTone *tSig = gens;
	MatrixTone *tEnd = &gens[MATGEN];
	while (tSig < tEnd)
	{
		allFlags |= tSig->toneFlags;
		if (tSig->toneFlags & TONE_ON)
		{
			envUsed |= (1 << tSig->envIndex);
			tSig->Start(frq);
		}
		tSig++;
	}
	bsUint16 flgs = envUsed;
	EnvGenSegSus *envPtr = envs;
	EnvGenSegSus *envEnd = &envs[MATGEN];
	while (envPtr < envEnd)
	{
		if (flgs & 1)
			envPtr->Reset(0);
		envPtr++;
		flgs >>= 1;
	}

	lfoOn = (allFlags & (TONE_LFOIN|TONE_TREM)) ? 1 : 0;
	pbOn  = (allFlags & TONE_PBIN) ? 1 : 0;
	fx1On = (allFlags & TONE_FX1OUT) ? 1 : 0;
	fx2On = (allFlags & TONE_FX2OUT) ? 1 : 0;
	fx3On = (allFlags & TONE_FX3OUT) ? 1 : 0;
	fx4On = (allFlags & TONE_FX4OUT) ? 1 : 0;
	panOn = (allFlags & TONE_PAN) ? 1 : 0;
	if (lfoOn)
	{
		lfoGen.SetSigFrq(frq);
		lfoGen.Reset(0);
	}
	if (pbOn)
		pbGen.Reset(0);
}

void MatrixSynth::Param(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);
	MatrixTone *tSig = gens;
	MatrixTone *tEnd = &gens[MATGEN];
	while (tSig < tEnd)
	{
		if (tSig->toneFlags & TONE_ON)
			tSig->AlterFreq(frq);
		tSig++;
	}
	if (lfoOn)
		lfoGen.Reset(-1);
	if (pbOn)
		pbGen.Reset(-1);
}

void MatrixSynth::SetParams(VarParamEvent *evt)
{
	chnl = evt->chnl;
	frq = evt->frq;
	vol = evt->vol;
	pbGen.SetFrequency(frq);

	bsInt16 idval;
	bsInt16 gn, vn, sn, ty;
	bsInt16 *id = evt->idParam;
	float *vp = evt->valParam;
	float val;
	int n = evt->numParam;
	while (n-- > 0)
	{
		// id = 0|xx[3]|vn[8] (generic)
		// id = 1|on[3]|vn[8] (oscillator)
		// id = 2|en[3]|sn[5]|vn[3] (envelope)
		val = *vp++;
		idval = *id++;
		gn = (idval >> 8)  & 7;
		ty = (idval >> 11) & 3;
		if (ty == 0)
		{
			vn = idval & 0xFF;
			switch (vn)
			{
			case 16:
				lfoGen.SetFrequency(FrqValue(val));
				break;
			case 17:
				lfoGen.SetWavetable((int) val);
				break;
			case 18:
				lfoGen.SetAttack(FrqValue(val));
				break;
			case 19:
				lfoGen.SetLevel(AmpValue(val));
				break;
			case 20:
				pbGen.SetRate(0, FrqValue(val));
				break;
			case 21:
				pbGen.SetRate(1, FrqValue(val));
				break;
			case 22:
				pbGen.SetAmount(0, FrqValue(val));
				break;
			case 23:
				pbGen.SetAmount(1, FrqValue(val));
				break;
			case 24:
				pbGen.SetAmount(2, FrqValue(val));
				break;
			}
		}
		else if (ty == 1)
		{
			// oscillator
			vn = idval & 0xFF;
			MatrixTone *sig = &gens[gn];
			switch (vn)
			{
			case 0: // output flags
				sig->toneFlags = (sig->toneFlags & TONE_MOD_BITS) | (((bsUint32) val) & TONE_OUT_BITS);
				break;
			case 1: // modulator flags
				sig->toneFlags = (sig->toneFlags & TONE_OUT_BITS) | (((bsUint32) val) << 16);
				break;
			case 2:
				sig->osc.SetWavetable((int)val);
				break;
			case 3:
				sig->frqMult = FrqValue(val);
				break;
			case 4:
				sig->modLvl = AmpValue(val);
				break;
			case 5:
				sig->volLvl = AmpValue(val);
				break;
			case 6:
				sig->envIndex = (bsUint16) val;
				break;
			case 7:
				sig->fx1Lvl = AmpValue(val);
				break;
			case 8:
				sig->fx2Lvl = AmpValue(val);
				break;
			case 9:
				sig->fx3Lvl = AmpValue(val);
				break;
			case 10:
				sig->fx4Lvl = AmpValue(val);
				break;
			case 11:
				sig->lfoLvl = AmpValue(val);
				break;
			case 12: // vibrato
				if (val)
					sig->toneFlags |= TONE_LFOIN;
				else
					sig->toneFlags &= ~TONE_LFOIN;
				break;
			case 13: // tremolo
				if (val)
					sig->toneFlags |= TONE_TREM;
				else
					sig->toneFlags &= ~TONE_TREM;
				break;
			case 14:
				sig->panSet.Set(panTrig, AmpValue(val));
				break;
			case 15: // pan on/off
				if (val)
					sig->toneFlags |= TONE_PAN;
				else
					sig->toneFlags &= ~TONE_PAN;
				break;
			case 16: // oscil on
				if (val)
					sig->toneFlags |= TONE_ON;
				else
					sig->toneFlags &= ~TONE_ON;
				break;
			case 17: // audio out
				if (val)
					sig->toneFlags |= TONE_OUT;
				else
					sig->toneFlags &= ~TONE_OUT;
				break;
			case 18: // Fx1 out
				if (val)
					sig->toneFlags |= TONE_FX1OUT;
				else
					sig->toneFlags &= ~TONE_FX2OUT;
				break;
			case 19: // Fx2 out
				if (val)
					sig->toneFlags |= TONE_FX2OUT;
				else
					sig->toneFlags &= ~TONE_FX2OUT;
				break;
			case 20: // Fx3 out
				if (val)
					sig->toneFlags |= TONE_FX3OUT;
				else
					sig->toneFlags &= ~TONE_FX3OUT;
				break;
			case 21: // Fx4 out
				if (val)
					sig->toneFlags |= TONE_FX4OUT;
				else
					sig->toneFlags &= ~TONE_FX4OUT;
				break;
			case 22: // pitch bend amount
				sig->pbLvl = AmpValue(val);
				break;
			case 23: // pitch bend on/off
				if (val)
					sig->toneFlags |= TONE_PBIN;
				else
					sig->toneFlags &= ~TONE_PBIN;
				break;
			}
		}
		else if (ty == 2)
		{
			// envelope
			EnvGenSegSus *env = &envs[gn];
			sn = (idval >> 3) & 0x1F;
			vn = idval & 7;
			switch (vn)
			{
			case 0:
				env->SetStart(AmpValue(val));
				break;
			case 1:
				env->SetSusOn((int)val);
				break;
			case 2:
				env->SetRate(sn, FrqValue(val));
				break;
			case 3:
				env->SetLevel(sn, AmpValue(val));
				break;
			case 4:
				env->SetType(sn, (EGSegType)(int)val);
				break;
			}
		}
	}
}

void MatrixSynth::Stop()
{
	bsUint16 flg = envUsed;
	EnvGenSegSus *envPtr = envs;
	EnvGenSegSus *envEnd = &envs[MATGEN];
	while (envPtr < envEnd)
	{
		if (flg & 1)
			envPtr->Release();
		flg >>= 1;
		envPtr++;
	}
}

void MatrixSynth::Tick()
{
	bsUint32 flgs;
	AmpValue sigOut = 0;
	AmpValue sigLft = 0;
	AmpValue sigRgt = 0;
	AmpValue lfoRad = 0;
	AmpValue lfoAmp = 0;
	AmpValue pbRad  = 0;
	AmpValue fx1Out = 0;
	AmpValue fx2Out = 0;
	AmpValue fx3Out = 0;
	AmpValue fx4Out = 0;
	AmpValue outVal[MATGEN];
	AmpValue *out = outVal;
	MatrixTone *tMod;
	MatrixTone *tSig;
	MatrixTone *tEnd;
	EnvGenSegSus *envPtr;
	EnvGenSegSus *envEnd;
	AmpValue egVal[MATGEN];
	AmpValue *eg = egVal;
	bsUint16 envFlgs;

	if (lfoOn)
	{
		lfoAmp = lfoGen.Gen();
		lfoRad = lfoAmp * synthParams.frqTI;
	}
	if (pbOn)
		pbRad = pbGen.Gen() * synthParams.frqTI;

	// Run the envelope generators
	envFlgs = envUsed;
	envEnd = &envs[MATGEN];
	envPtr = envs; 
	while (envPtr < envEnd)
	{
		if (envFlgs & 1)
			*eg = envPtr->Gen();
		else
			*eg = 0;
		eg++;
		envFlgs >>= 1;
		envPtr++;
	}

	// Collect samples from all active generators,
	// and sum the output signals.
	tEnd = &gens[MATGEN];
	for (tSig = gens; tSig < tEnd; tSig++)
	{
		flgs = tSig->toneFlags;
		if (flgs  & TONE_ON)
		{
			AmpValue sig = tSig->osc.Gen() * egVal[tSig->envIndex];
			if (flgs & TONE_TREM)
				sig += lfoAmp;
			*out = sig;
			if (flgs & TONE_OUT)
			{
				sig *= tSig->volLvl;
				if (flgs & TONE_PAN)
				{
					sigLft += sig * tSig->panSet.panlft;
					sigRgt += sig * tSig->panSet.panrgt;
				}
				else
					sigOut += sig;
				if (flgs & TONE_FX1OUT)
					fx1Out += sig * tSig->fx1Lvl;
				if (flgs & TONE_FX2OUT)
					fx2Out += sig * tSig->fx2Lvl;
				if (flgs & TONE_FX3OUT)
					fx3Out += sig * tSig->fx3Lvl;
				if (flgs & TONE_FX4OUT)
					fx4Out += sig * tSig->fx4Lvl;
			}
		}
		else
			*out = 0; // justin case...
		out++;
	}

	// Apply modulators
	if (allFlags & TONE_MODANY)
	{
		PhsAccum phs;
		bsUint32 mask;
		for (tSig = gens; tSig < tEnd; tSig++)
		{
			flgs = tSig->toneFlags;
			if (flgs & TONE_MODANY)
			{
				if (flgs & TONE_LFOIN)
					phs = lfoRad * tSig->lfoLvl;
				else
					phs = 0;
				if (flgs & TONE_PBIN)
					phs += pbRad * tSig->pbLvl;
				out = outVal;
				mask = TONE_MOD1IN;
				for (tMod = gens; tMod < tEnd; tMod++)
				{
					if (flgs & mask)
						phs += *out * tMod->modRad;
					out++;
					mask <<= 1;
				}
				tSig->PhaseModWT(phs);
			}
		}
	}
	if (fx1On)
		im->FxSend(0, fx1Out);
	if (fx2On)
		im->FxSend(1, fx2Out);
	if (fx3On)
		im->FxSend(2, fx3Out);
	if (fx4On)
		im->FxSend(3, fx4Out);

	if (panOn)
		im->Output2(chnl, sigLft * vol, sigRgt * vol);
	im->Output(chnl, sigOut * vol);
}

int  MatrixSynth::IsFinished()
{
	// Test envelope generators on signal outputs...
	MatrixTone *tSig = gens;
	MatrixTone *tEnd = &gens[MATGEN];
	while (tSig < tEnd)
	{
		if ((tSig->toneFlags & (TONE_ON|TONE_OUT)) == (TONE_ON|TONE_OUT))
			if (!envs[tSig->envIndex].IsFinished())
				return 0;
		tSig++;
	}
	return 1;
}

void MatrixSynth::Destroy()
{
	delete this;
}

int MatrixSynth::LoadEnv(XmlSynthElem *elem)
{
	short en = -1;
	if (elem->GetAttribute("en", en) != 0)
		return -1;
	if (en < 0 || en >= MATGEN)
		return -1;

	short segs = 2;
	if (elem->GetAttribute("segs", segs) != 0)
		return -1;
	envs[en].SetSegs(segs);

	short ival;
	float dval;

	dval = 0;
	elem->GetAttribute("st", dval);
	envs[en].SetStart(dval);

	ival = 1;
	elem->GetAttribute("sus", ival);
	envs[en].SetSusOn((int)ival);

	XmlSynthElem *elemEG;
	XmlSynthElem *next = elem->FirstChild();
	while ((elemEG = next) != NULL)
	{
		if (elemEG->TagMatch("seg"))
		{
			elemEG->GetAttribute("sn", ival);
			if (ival >= 0 && ival < segs)
			{
				int sn = ival;
				if (elemEG->GetAttribute("rt",  dval) == 0)
					envs[en].SetRate(sn, FrqValue(dval));
				if (elemEG->GetAttribute("lvl",  dval) == 0)
					envs[en].SetLevel(sn, AmpValue(dval));
				if (elemEG->GetAttribute("ty", ival) == 0)
					envs[en].SetType(sn, (EGSegType)ival);
			}
		}
		next = elemEG->NextSibling();
		delete elemEG;
	}

	return 0;
}

int MatrixSynth::SaveEnv(XmlSynthElem *elem, int en)
{
	short nsegs = envs[en].GetSegs();
	elem->SetAttribute("en", (short) en);
	elem->SetAttribute("segs", nsegs);
	elem->SetAttribute("st", envs[en].GetStart());
	elem->SetAttribute("sus", (short)envs[en].GetSusOn());

	XmlSynthElem *elemEG;
	short sn;
	for (sn = 0; sn < nsegs; sn++)
	{
		elemEG = elem->AddChild("seg");
		if (elemEG == NULL)
			return -1;
		elemEG->SetAttribute("sn", sn);
		elemEG->SetAttribute("rt",  envs[en].GetRate(sn));
		elemEG->SetAttribute("lv",  envs[en].GetLevel(sn));
		elemEG->SetAttribute("ty", (short) envs[en].GetType(sn));
		delete elemEG;
	}

	return 0;
}

int MatrixSynth::Load(XmlSynthElem *parent)
{
	float dval;
	short ival;

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("mat"))
		{
			if (elem->GetAttribute("frq", dval) == 0)
				frq = FrqValue(dval);
			if (elem->GetAttribute("vol", dval) == 0)
				vol = AmpValue(dval);
		}
		else if (elem->TagMatch("gen"))
		{
			if (elem->GetAttribute("gn",  ival) == 0)
			{
				if (ival >= 0 && ival < MATGEN)
					gens[ival].Load(elem);
			}
		}
		else if (elem->TagMatch("env"))
		{
			LoadEnv(elem);
		}
		else if (elem->TagMatch("lfo"))
		{
			lfoGen.Load(elem);
		}
		else if (elem->TagMatch("pb"))
		{
			pbGen.Load(elem);
		}
		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int MatrixSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem = parent->AddChild("mat");
	if (elem == NULL)
		return -1;
	int err;
	err = elem->SetAttribute("frq", frq);
	err |= elem->SetAttribute("vol", vol);
	delete elem;

	long n;
	for (n = 0; n < MATGEN; n++)
	{
		elem = parent->AddChild("gen");
		if (elem == NULL)
			return -1;
		err |= elem->SetAttribute("gn", n);
		gens[n].Save(elem);
		delete elem;
	}

	for (n = 0; n < MATGEN; n++)
	{
		elem = parent->AddChild("env");
		if (elem == NULL)
			return -1;
		SaveEnv(elem, n);
		delete elem;
	}

	elem = parent->AddChild("lfo");
	if (elem == NULL)
		return -1;
	err |= lfoGen.Save(elem);
	delete elem;

	elem = parent->AddChild("pb");
	if (elem == NULL)
		return -1;
	err |= pbGen.Save(elem);
	delete elem;

	return err;
}

/////////////////////////////////////////////////


MatrixTone::MatrixTone()
{
	toneFlags = 0;
	frqMult = 1.0;
	modLvl = 1.0;
	volLvl = 1.0;
	fx1Lvl = 0;
	fx2Lvl = 0;
	fx3Lvl = 0;
	fx4Lvl = 0;
	panSet.Set(panOff, 0);
	lfoLvl = 0;
	pbLvl = 0;
	envIndex = 0;
}

MatrixTone::~MatrixTone()
{
}

void MatrixTone::Copy(MatrixTone *tp)
{
	toneFlags = tp->toneFlags;
	frqMult = tp->frqMult;
	modLvl = tp->modLvl;
	modRad = tp->modRad;
	volLvl = tp->volLvl;
	fx1Lvl = tp->fx1Lvl;
	fx2Lvl = tp->fx2Lvl;
	fx3Lvl = tp->fx3Lvl;
	lfoLvl = tp->lfoLvl;
	pbLvl = tp->pbLvl;
	panSet.panlft = tp->panSet.panlft;
	panSet.panrgt = tp->panSet.panrgt;
	osc.SetFrequency(tp->osc.GetFrequency());
	osc.SetWavetable(tp->osc.GetWavetable());
	envIndex = tp->envIndex;
}

inline void MatrixTone::Start(FrqValue frqBase)
{
	FrqValue f = frqBase * frqMult;
	modRad = f * modLvl * synthParams.frqTI;
	//if (modRad > maxPhs)
	//	modRad = maxPhs;
	osc.SetFrequency(f);
	osc.Reset(0);
}

inline void MatrixTone::AlterFreq(FrqValue frqBase)
{
	FrqValue f = frqBase * frqMult;
	modRad = f * modLvl * synthParams.frqTI;
	osc.SetFrequency(f);
	osc.Reset(-1);
}

inline void MatrixTone::PhaseModWT(PhsAccum phs)
{
	osc.PhaseModWT(phs);
}

int MatrixTone::Load(XmlSynthElem *elem)
{
	float dval;
	short ival;
	toneFlags = 0;

	if (elem->GetAttribute("wt",  ival) == 0)
		osc.SetWavetable(ival);
	if (elem->GetAttribute("mul", dval) == 0)
		frqMult = FrqValue(dval);
	if (elem->GetAttribute("mnx", dval) == 0)
		modLvl = AmpValue(dval);
	char *bits = NULL;
	char *bp;
	bsUint32 mask;
	int b;
	if (elem->GetAttribute("out", &bits) == 0)
	{
		bp = bits;
		mask = 1;
		for (b = 0; b < 16 && *bp; b++, bp++)
		{
			if (*bp == '1')
				toneFlags |= mask;
			mask <<= 1;
		}
		delete bits;
	}

	if (elem->GetAttribute("mod", &bits) == 0)
	{
		bp = bits;
		mask = TONE_MOD1IN;
		for (b = 0; b < MATGEN && *bp; b++, bp++)
		{
			if (*bp == '1')
				toneFlags |= mask;
			mask <<= 1;
		}
		delete bits;
	}

	if (elem->GetAttribute("vol", dval) == 0)
		volLvl = AmpValue(dval);
	if (elem->GetAttribute("eg", ival) == 0)
		envIndex = (bsUint16) ival;
	if (elem->GetAttribute("lfo", dval) == 0)
		lfoLvl = AmpValue(dval);
	if (elem->GetAttribute("fx1", dval) == 0)
		fx1Lvl = AmpValue(dval);
	if (elem->GetAttribute("fx2", dval) == 0)
		fx2Lvl = AmpValue(dval);
	if (elem->GetAttribute("fx3", dval) == 0)
		fx3Lvl = AmpValue(dval);
	if (elem->GetAttribute("fx4", dval) == 0)
		fx4Lvl = AmpValue(dval);
	if (elem->GetAttribute("pan", dval) == 0)
		panSet.Set(panTrig, AmpValue(dval));

	return 0;
}

int MatrixTone::Save(XmlSynthElem *elem)
{
	int err;
	err  = elem->SetAttribute("wt",  (short) osc.GetWavetable());
	err |= elem->SetAttribute("mul", frqMult);
	err |= elem->SetAttribute("mnx", modLvl);
	err |= elem->SetAttribute("eg",  (short) envIndex);
	char bits[33];
	bsUint32 mask = 1;
	int b;
	for (b = 0; b < 16; b++)
	{
		bits[b] = toneFlags & mask ? '1' : '0';
		mask <<= 1;
	}
	bits[b] = 0;
	err |= elem->SetAttribute("out", bits);

	mask = TONE_MOD1IN;
	for (b = 0; b < MATGEN; b++)
	{
		bits[b] = toneFlags & mask ? '1' : '0';
		mask <<= 1;
	}
	bits[b] = 0;
	err |= elem->SetAttribute("mod", bits);

	err |= elem->SetAttribute("vol", volLvl);
	err |= elem->SetAttribute("fx1", fx1Lvl);
	err |= elem->SetAttribute("fx2", fx1Lvl);
	err |= elem->SetAttribute("fx3", fx3Lvl);
	err |= elem->SetAttribute("fx4", fx4Lvl);
	err |= elem->SetAttribute("pan", panSet.panval);
	err |= elem->SetAttribute("lfo", lfoLvl);
	err |= elem->SetAttribute("pb", pbLvl);

	return err;
}
