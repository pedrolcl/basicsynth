//////////////////////////////////////////////////////////////////////
// BasicSynth - Additive Synthesis instrument 
//
// See _BasicSynth_ Chapter 20 for a full explanation
//
// Maintains an array of "tone generators" and sums the outputs.
// Each tone generator consists of a wavetable oscillator and envelope
// generator. A single LFO generator applies optional vibrato to
// all tone generators.
//
// Copyright 2008, Daniel R. Mitchell
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "AddSynth.h"

Instrument *AddSynth::AddSynthFactory(InstrManager *m, Opaque tmplt)
{
	AddSynth *ip = new AddSynth;
	ip->im = m;
	if (tmplt)
		ip->Copy((AddSynth *) tmplt);
	return ip;
}

SeqEvent *AddSynth::AddSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->frq = 440.0;
	ep->vol = 1.0;
	AddSynth *ap = (AddSynth *)tmplt;
	if (ap)
		ep->maxParam  = ((ap->numParts + 1) * 512) + 8;
	else
		ep->maxParam  = 4616; // default: 8 partials
	return (SeqEvent *) ep;
}


AddSynth::AddSynth()
{
	chnl = 0;
	frq = 440.0;
	vol = 1.0;
	numParts = 0;
	parts = NULL;
}

AddSynth::~AddSynth()
{
	delete[] parts;
}

int AddSynth::SetNumParts(int n)
{
	delete parts;
	numParts = 0;
	parts = new AddSynthPart[n];
	if (parts == NULL)
		return -1;
	numParts = n;
	return 0;
}

int AddSynth::GetNumParts()
{
	return numParts;
}

AddSynthPart *AddSynth::GetPart(int n)
{
	if (n < numParts)
		return &parts[n];
	return NULL;
}

void AddSynth::Copy(AddSynth *tp)
{
	SetNumParts(tp->GetNumParts());
	for (int n = 0; n < numParts; n++)
		parts[n].Copy(&tp->parts[n]);
	lfoGen.Copy(&tp->lfoGen);
}

void AddSynth::Start(SeqEvent *evt)
{
	UpdateParams(evt, 0);
}

void AddSynth::Param(SeqEvent *evt)
{
	UpdateParams(evt, -1);
}

void AddSynth::UpdateParams(SeqEvent *evt, float initPhs)
{
	VarParamEvent *vpe = (VarParamEvent *)evt;
	chnl = vpe->chnl;
	vol = vpe->vol;
	frq = vpe->frq;
	AddSynthPart *pSig;
	bsInt16 *id = vpe->idParam;
	float *valp = vpe->valParam;
	int n = vpe->numParam;
	// [pn(6)][sn(4)][val(4)]
	// PN = (partial number + 1) * 256
	// SN = (segment number + 1) * 16
	// PN+0	Frequency multiplier. 
	// PN+1	Initial frequency for this oscillator. 
	// PN+2	Wave table index.
	// PN+3	Starting value for the envelope.
	// PN+4 Sustain-on flag, 1 or 0.
	// PN+SN+5	segment rate
	// PN+SN+6	Level at the end of the segment.
	// PN+SN+7	Segment curve type: 1=linear 2=exponential 3=log.

	bsInt16 idval, pn, sn;
	float val;

	while (n-- > 0)
	{
		val = *valp++;
		idval = *id++;
		pn = (idval & 0x7F00) >> 8;
		if (pn == 0)
		{
			switch (idval)
			{
			case 16:
				lfoGen.SetFrequency(FrqValue(val));
				break;
			case 17:
				lfoGen.SetWavetable((int)val);
				break;
			case 18:
				lfoGen.SetAttack(FrqValue(val));
				break;
			case 19:
				lfoGen.SetLevel(AmpValue(val));
				break;
			}
		}
		else if (pn <= numParts)
		{
			sn = (idval & 0xF0) >> 4;
			pSig = &parts[pn-1];
			switch (idval & 0x0F)
			{
			case 0:
				pSig->mul = FrqValue(val);
				break;
			case 1:
				pSig->osc.SetFrequency(FrqValue(val));
				break;
			case 2:
				pSig->osc.SetWavetable((int)val);
				break;
			case 3:
				pSig->env.SetStart(AmpValue(val));
				break;
			case 4:
				pSig->env.SetSusOn((int)val);
				break;
			case 5:
				pSig->env.SetRate(sn, FrqValue(val));
				break;
			case 6:
				pSig->env.SetLevel(sn, AmpValue(val));
				break;
			case 7:
				pSig->env.SetType(sn, (EGSegType) (int) val);
				break;
			}
		}
	}

	FrqValue nyquist = synthParams.sampleRate / 2;
	AddSynthPart *pEnd = &parts[numParts];
	for (pSig = parts; pSig < pEnd; pSig++)
	{
		if (pSig->mul > 0)
		{
			FrqValue f = pSig->mul * frq;
			if (f < 0 || f >= nyquist)
				f = 0;
			pSig->osc.SetFrequency(f);
		}
		pSig->osc.Reset(initPhs);
		pSig->env.Reset(initPhs);
	}
	lfoGen.SetSigFrq(frq);
	lfoGen.Reset(initPhs);
}

void AddSynth::Stop()
{
	AddSynthPart *pSig = parts;
	AddSynthPart *pEnd = &parts[numParts];
	while (pSig < pEnd)
	{
		pSig->env.Release();
		pSig++;
	}
}

void AddSynth::Tick()
{
	PhsAccum phs;
	int lfoOn = lfoGen.On();
	if (lfoOn)
		phs = lfoGen.Gen() * synthParams.frqTI;

	AmpValue sigVal = 0;
	AddSynthPart *pSig = parts;
	AddSynthPart *pEnd = &parts[numParts];
	while (pSig < pEnd)
	{
		if (lfoOn)
			pSig->osc.PhaseModWT(phs * pSig->mul);
		sigVal += pSig->env.Gen() * pSig->osc.Gen();
		pSig++;
	}

	im->Output(chnl, sigVal * vol);
}

int  AddSynth::IsFinished()
{
	AddSynthPart *pSig = parts;
	AddSynthPart *pEnd = &parts[numParts];
	while (pSig < pEnd)
	{
		if (!pSig->env.IsFinished())
			return 0;
		pSig++;
	}

	return 1;
}

void AddSynth::Destroy()
{
	delete this;
}

/*************
<instr parts="n">
 <part pn="n"  mul="n" frq="n" wt="n" />
   <env segs="n" st="n" son="n">
    <seg sn="n" rt="n" lvl="n" ty="t" />
   </env>
 </part>
 <lfo frq="" wt="" atk="" lvl="" />
</instr>
***************/

int AddSynth::Load(XmlSynthElem *parent)
{
	float dval;
	float lvl;
	float rt;
	long ival;

	if (parent->GetAttribute("parts", ival) == 0)
		SetNumParts(ival);

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("part"))
		{
			long pno = -1;
			if (elem->GetAttribute("pn", pno) == 0 && pno < numParts)
			{
				AddSynthPart *pn = &parts[pno];
				if (elem->GetAttribute("mul", dval) == 0)
					pn->mul = FrqValue(dval);
				if (elem->GetAttribute("frq", dval) == 0)
					pn->osc.SetFrequency(FrqValue(dval));
				if (elem->GetAttribute("wt", ival) == 0)
					pn->osc.SetWavetable((int) ival);
				XmlSynthElem *partElem = elem->FirstChild();
				while (partElem != NULL)
				{
					if (partElem->TagMatch("env"))
					{
						EnvGenSeg *pe = &pn->env;
						long son;
						long nseg = 0;
						if (partElem->GetAttribute("segs", nseg) == 0)
							pe->SetSegs((int)nseg);
						if (partElem->GetAttribute("st", lvl) == 0)
							pe->SetStart(AmpValue(lvl));
						if (partElem->GetAttribute("sus", son) == 0)
							pe->SetSusOn((int)son);
						XmlSynthElem *segElem = partElem->FirstChild();
						while (segElem != NULL)
						{
							if (segElem->TagMatch("seg"))
							{
								if (segElem->GetAttribute("sn", nseg) == 0)
								{
									if (segElem->GetAttribute("rt", rt) == 0)
										pe->SetRate((int)nseg, FrqValue(rt));
									if (segElem->GetAttribute("lvl", lvl) == 0)
										pe->SetLevel((int)nseg, AmpValue(lvl));
									if (segElem->GetAttribute("ty", ival) == 0)
										pe->SetType((int)nseg, (EGSegType)ival);
								}
							}
							next = segElem->NextSibling();
							delete segElem;
							segElem = next;
						}
					}
					next = partElem->NextSibling();
					delete partElem;
					partElem = next;
				}
			}
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

int AddSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *partElem;
	XmlSynthElem *subElem;
	XmlSynthElem *segElem;

	parent->SetAttribute("parts", (short)numParts);
	AddSynthPart *p = parts;
	for (int n = 0; n < numParts; n++, p++)
	{
		partElem = parent->AddChild("part");
		if (partElem == NULL)
			return -1;
		partElem->SetAttribute("pn", (short) n);
		partElem->SetAttribute("mul", p->mul);
		partElem->SetAttribute("frq", p->osc.GetFrequency());
		partElem->SetAttribute("wt", (short) p->osc.GetWavetable());

		subElem = partElem->AddChild("env");
		if (subElem == NULL)
			return -1;
		EnvGenSeg *pe = &p->env;
		int segs = pe->GetSegs();
		subElem->SetAttribute("segs", (short) segs);
		subElem->SetAttribute("st", pe->GetStart());
		subElem->SetAttribute("sus", (short) pe->GetSusOn());
		for (int sn = 0; sn < segs; sn++)
		{
			segElem = subElem->AddChild("seg");
			if (segElem == NULL)
				return -1;
			segElem->SetAttribute("sn", (short) sn);
			segElem->SetAttribute("rt", pe->GetRate(sn));
			segElem->SetAttribute("lvl", pe->GetLevel(sn));
			segElem->SetAttribute("ty", (short) pe->GetType(sn));
			delete segElem;
		}
		delete subElem;
		delete partElem;
	}

	XmlSynthElem *lfoElem = parent->AddChild("lfo");
	if (lfoElem == NULL)
		return -1;
	lfoGen.Save(lfoElem);
	delete lfoElem;

	return 0;
}
