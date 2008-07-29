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
	allFlags = 0;
	envUsed = 0;
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
	EnvGenA3SR *envPtr = envs;
	EnvGenA3SR *envEnd = &envs[MATGEN];
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
		float val = *vp++;
		bsInt16 idval = *id++;
		int gn = (idval >> 8) & (MATGEN-1);
		int vn = idval & 0x3F;
		if (idval & 0x01000)
		{
			// oscillator
			MatrixTone *sig = &gens[gn];
			switch (vn)
			{
			case 1: // flags
				sig->toneFlags = (bsUint32) val;
				break;
			case 2:
				sig->osc.SetWavetable((int)val);
				break;
			case 3:
				sig->frqMult = (FrqValue) val;
				break;
			case 4:
				sig->modLvl = (AmpValue) val;
				break;
			case 5:
				sig->volLvl = (AmpValue) val;
				break;
			case 6:
				sig->envIndex = (bsUint16) val;
				break;
			case 7:
				sig->fx1Lvl = (AmpValue) val;
				break;
			case 8:
				sig->fx2Lvl = (AmpValue) val;
				break;
			case 9:
				sig->fx3Lvl = (AmpValue) val;
				break;
			case 10:
				sig->fx4Lvl = (AmpValue) val;
				break;
			}
		}
		else if (idval & 0x2000)
		{
			// envelope
			EnvGenA3SR *env = &envs[gn];
			int sn = (idval >> 6) & 0x3;
			switch (vn)
			{
			case 11:
				env->SetStart((AmpValue)val);
				break;
			case 12:
				env->SetRate(sn, (FrqValue)val);
				break;
			case 13:
				env->SetLevel(sn, (AmpValue)val);
				break;
			case 14:
				env->SetType(sn, (EGSegType)(int)val);
				break;
			}
		}
		else
		{
			switch (vn)
			{
			case 16: // pan;
				panFix = (AmpValue) val;
				break;
			case 17:
				lfoGen.SetFrequency((FrqValue)val);
				break;
			case 18:
				lfoGen.SetWavetable((int) val);
				break;
			case 19:
				lfoGen.SetAttack((FrqValue)val);
				break;
			case 20:
				lfoGen.SetLevel((AmpValue)val);
				break;
			}
		}
	}
}

void MatrixSynth::Stop()
{
	bsUint16 flg = envUsed;
	EnvGenA3SR *envPtr = envs;
	EnvGenA3SR *envEnd = &envs[MATGEN];
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
	EnvGenA3SR *envPtr;
	EnvGenA3SR *envEnd;
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
			AmpValue v = tSig->osc.Gen() * egVal[tSig->envIndex];
			if (flgs & TONE_OUT)
			{
				sigOut += v * tSig->volLvl;
				if (flgs & TONE_FX1OUT)
					fx1Out += v * tSig->fx1Lvl;
				if (flgs & TONE_FX2OUT)
					fx2Out += v * tSig->fx2Lvl;
				if (flgs & TONE_FX3OUT)
					fx3Out += v * tSig->fx3Lvl;
				if (flgs & TONE_FX4OUT)
					fx4Out += v * tSig->fx4Lvl;
			}
			*out = v;
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

	sigOut *= vol;
	if (panOn)
	{
		AmpValue pan = panOsc.Gen();
		AmpValue lft = 1 - pan;
		AmpValue rgt = pan;
		im->Output2(chnl, sigOut * lft, sigOut * rgt);
	}
	else
		im->Output(chnl, sigOut);
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
	long en = -1;
	if (elem->GetAttribute("en", en) != 0)
		return -1;
	if (en < 0 || en >= MATGEN)
		return -1;

	long ival;
	double dval;

	dval = 0;
	elem->GetAttribute("st", dval);
	envs[en].SetStart(dval);

	ival = 1;
	elem->GetAttribute("so", ival);
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
	elem->SetAttribute("en", (long) en);
	elem->SetAttribute("st", (double)envs[en].GetStart());
	elem->SetAttribute("so", (long)envs[en].GetSusOn());

	XmlSynthElem *elemEG;
	int sn;
	for (sn = 0; sn < nsegs; sn++)
	{
		elemEG = elem->AddChild("seg");
		if (elemEG == NULL)
			return -1;
		elemEG->SetAttribute("sn", (long) sn);
		elemEG->SetAttribute("rt",  (double)envs[en].GetRate(sn));
		elemEG->SetAttribute("lv",  (double)envs[en].GetLevel(sn));
		elemEG->SetAttribute("ty", (long) envs[en].GetType(sn));
		delete elem;
	}

	return 0;
}

int MatrixSynth::Load(XmlSynthElem *parent)
{
	double dval;
	long ival;

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
			if (elem->GetAttribute("pan", dval) == 0)
				panFix = AmpValue(dval);
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
	elem->SetAttribute("frq", (double) frq);
	elem->SetAttribute("vol", (double) vol);
	delete elem;

	for (long n = 0; n < MATGEN; n++)
	{
		elem = parent->AddChild("gen");
		if (elem == NULL)
			return -1;
		elem->SetAttribute("gn", n);
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
	lfoGen.Save(elem);
	delete elem;

	return 0;
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

#define MAXBIT 16

int MatrixTone::Load(XmlSynthElem *elem)
{
	double dval;
	long ival;
	toneFlags = 0;
	char *bits = NULL;
	if (elem->GetAttribute("flg", &bits) == 0)
	{
		char *bp = bits;
		bsUint32 mask = 1;
		for (int b = 0; b < MAXBIT && *bp; b++)
		{
			if (*bp == '1')
				toneFlags |= mask;
			mask <<= 1;
			bp++;
		}
		delete bits;
	}

	if (elem->GetAttribute("frq", dval) == 0)
		osc.SetFrequency((FrqValue) dval);
	if (elem->GetAttribute("wt",  ival) == 0)
		osc.SetWavetable(ival);
	if (elem->GetAttribute("mul", dval) == 0)
		frqMult = FrqValue(dval);
	if (elem->GetAttribute("mnx", dval) == 0)
		modLvl = AmpValue(dval);
	if (elem->GetAttribute("vol", dval) == 0)
		volLvl = AmpValue(dval);
	if (elem->GetAttribute("eg", ival) == 0)
		envIndex = (bsUint16) ival;
	if (elem->GetAttribute("fx1", dval) == 0)
		fx1Lvl = AmpValue(dval);
	if (elem->GetAttribute("fx2", dval) == 0)
		fx2Lvl = AmpValue(dval);
	if (elem->GetAttribute("fx3", dval) == 0)
		fx3Lvl = AmpValue(dval);
	if (elem->GetAttribute("fx4", dval) == 0)
		fx4Lvl = AmpValue(dval);

	return 0;
}

int MatrixTone::Save(XmlSynthElem *elem)
{
	char bits[MAXBIT+1];
	bsUint32 mask = 1;
	int b;
	for (b = 0; b < MAXBIT; b++)
	{
		bits[b] = toneFlags & mask ? '1' : '0';
		mask >>= 1;
	}
	bits[b] = 0;
	elem->SetAttribute("flg", bits);
	elem->SetAttribute("frq", (double) osc.GetFrequency());
	elem->SetAttribute("wt",  (long) osc.GetWavetable());
	elem->SetAttribute("mul", (double) frqMult);
	elem->SetAttribute("mnx", (double) modLvl);
	elem->SetAttribute("vol", (double) volLvl);
	elem->SetAttribute("eg",  (long) envIndex);
	elem->SetAttribute("fx1", (double) fx1Lvl);
	elem->SetAttribute("fx2", (double) fx1Lvl);
	elem->SetAttribute("fx3", (double) fx3Lvl);
	elem->SetAttribute("fx4", (double) fx4Lvl);

	return 0;
}
