// MatrixSynth.cpp: implementation of the MatrixSynth class.
//
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "MatrixSynth.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
Instrument *MatrixSynth::MatrixSynthFactory(InstrManager *m, Opaque tmplt)
{
	MatrixSynth *ip = new MatrixSynth;
	ip->im = m;
	if (tmplt)
		ip->Copy((MatrixSynth *) tmplt);
	return ip;
}

SeqEvent   *MatrixSynth::MatrixSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 229 + P_VOLUME;
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
	fx1On = 0;
	fx2On = 0;
	fx3On = 0;
	fx4On = 0;
	panOn = 0;
	allFlags = 0;
	envUsed = 0;
	for (int n = 0; n < MATGEN; n++)
		envs[n].SetSegs(4);
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

	lfoOn = (allFlags & TONE_LFOIN) ? 1 : 0;
	fx1On = (allFlags & TONE_FX1OUT) ? 1 : 0;
	fx2On = (allFlags & TONE_FX2OUT) ? 1 : 0;
	fx3On = (allFlags & TONE_FX3OUT) ? 1 : 0;
	fx4On = (allFlags & TONE_FX4OUT) ? 1 : 0;
	panOn = (allFlags & TONE_PAN) ? 1 : 0;
	if (lfoOn)
		lfoGen.Reset(0);
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
}

void MatrixSynth::SetParams(VarParamEvent *evt)
{
	chnl = evt->chnl;
	frq = evt->frq;
	vol = evt->vol;

	bsInt16 *id = evt->idParam;
	float *vp = evt->valParam;
	int n = evt->numParam;
	while (n-- > 0)
	{
		// id = type[2]|gn[4]|sn[2]|vn[6]
		float val = *vp++;
		bsInt16 idval = *id++;
		int gn = (idval >> 8) & (MATGEN-1);
		int vn = idval & 0x3F;
		if (idval & 0x1000)
		{
			// oscillator
			MatrixTone *sig = &gens[gn];
			switch (vn)
			{
			case 0: // output flags
				sig->toneFlags = (sig->toneFlags & TONE_MOD_BITS) | (bsUint32) val;
				break;
			case 1: // modulator flags
				sig->toneFlags = (sig->toneFlags & TONE_OUT_BITS) | (bsUint32) val;
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
				sig->panSet = AmpValue(val);
				sig->panLft = (1 - sig->panSet) / 2;
				sig->panRgt = (1 + sig->panSet) / 2;
				break;
			case 12:
				sig->lfoLvl = AmpValue(val);
				break;
			case 13:
				sig->pbLvl = AmpValue(val);
				break;
			}
		}
		else if (idval & 0x2000)
		{
			// envelope
			EnvGenSegSus *env = &envs[gn];
			int sn = (idval >> 6) & 0x3;
			switch (vn)
			{
			case 0:
				env->SetStart(AmpValue(val));
				break;
			case 1:
				env->SetSuson((int)val);
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
		else
		{
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
		lfoRad = lfoGen.Gen() * synthParams.frqTI;

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
			*out = sig;
			if (flgs & TONE_OUT)
			{
				sig *= tSig->volLvl;
				if (flgs & TONE_PAN)
				{
					sigLft += sig * tSig->panLft;
					sigRgt += sig * tSig->panRgt;
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
					phs = lfoRad;
				else
					phs = 0;
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
		im->FxSend(chnl, 0, fx1Out);
	if (fx2On)
		im->FxSend(chnl, 1, fx2Out);
	if (fx3On)
		im->FxSend(chnl, 2, fx3Out);
	if (fx4On)
		im->FxSend(chnl, 3, fx4Out);

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

	short ival;
	float dval;

	dval = 0;
	elem->GetAttribute("st", dval);
	envs[en].SetStart(dval);

	ival = 1;
	elem->GetAttribute("sus", ival);
	envs[en].SetSuson((int)ival);

	XmlSynthElem *elemEG;
	XmlSynthElem *next = elem->FirstChild();
	while ((elemEG = next) != NULL)
	{
		if (elemEG->TagMatch("seg"))
		{
			elemEG->GetAttribute("sn", ival);
			if (ival >= 0 && ival <= 3)
			{
				int sn = ival;
				if (elemEG->GetAttribute("rt",  dval) == 0)
					envs[en].SetRate(sn, FrqValue(dval));
				if (elemEG->GetAttribute("lv",  dval) == 0)
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
	int nsegs = envs[en].GetSegs();
	elem->SetAttribute("en", (short) en);
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
		delete elem;
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

	for (long n = 0; n < MATGEN; n++)
	{
		elem = parent->AddChild("gen");
		if (elem == NULL)
			return -1;
		err |= elem->SetAttribute("gn", n);
		gens[n].Save(elem);
		delete elem;
	}

	for (long n = 0; n < MATGEN; n++)
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
	panSet = 0;
	panLft = 0.5;
	panRgt = 0.5;
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
	panSet = tp->panSet;
	panLft = tp->panLft;
	panRgt = tp->panRgt;
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
	if (elem->GetAttribute("out", &bits) == 0)
	{
		char *bp = bits;
		bsUint32 mask = 1;
		for (int b = 0; b < 16 && *bp; b++, bp++)
		{
			if (*bp == '1')
				toneFlags |= mask;
			mask <<= 1;
		}
		delete bits;
	}

	if (elem->GetAttribute("mod", &bits) == 0)
	{
		char *bp = bits;
		bsUint32 mask = TONE_MOD1IN;
		for (int b = 0; b < MATGEN && *bp; b++, bp++)
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
	{
		panSet = AmpValue(dval);
		panLft = (1 - panSet) / 2;
		panRgt = (1 + panSet) / 2;
	}

	return 0;
}

int MatrixTone::Save(XmlSynthElem *elem)
{
	int err;
	err  = elem->SetAttribute("wt",  (short) osc.GetWavetable());
	err |= elem->SetAttribute("mul", frqMult);
	err |= elem->SetAttribute("mnx", modLvl);
	err |= elem->SetAttribute("eg",  (short) envIndex);
	char bits[MATGEN+1];
	bsUint32 mask = 1;
	int b;
	for (b = 0; b < 16; b++)
	{
		bits[b] = toneFlags & mask ? '1' : '0';
		mask >>= 1;
	}
	bits[b] = 0;
	err |= elem->SetAttribute("out", bits);

	mask = TONE_MOD1IN;
	for (b = 0; b < MATGEN; b++)
	{
		bits[b] = toneFlags & mask ? '1' : '0';
		mask >>= 1;
	}
	bits[b] = 0;
	err |= elem->SetAttribute("mod", bits);

	err |= elem->SetAttribute("vol", volLvl);
	err |= elem->SetAttribute("fx1", fx1Lvl);
	err |= elem->SetAttribute("fx2", fx1Lvl);
	err |= elem->SetAttribute("fx3", fx3Lvl);
	err |= elem->SetAttribute("fx4", fx4Lvl);
	err |= elem->SetAttribute("pan", panSet);
	err |= elem->SetAttribute("lfo", lfoLvl);
	err |= elem->SetAttribute("pb", pbLvl);

	return err;
}
