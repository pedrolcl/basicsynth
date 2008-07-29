//////////////////////////////////////////////////////////////////////
// BasicSynth - Additive Synthesis instrument
//
// Maintains an array of "tone generators" and sums the outputs
//
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
	AddSynth *ap = (AddSynth *)tmplt;
	bsInt16 np = 0;
	if (ap)
	{
		// osci lfo (5) + numParts * (wt + mult + start + suson + env)
		np = 5 + (ap->numParts * 4);
		for (int n = 0; n < ap->numParts; n++)
			np += ap->parts[n].env.GetSegs() * 3;
	}
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = np;
	ep->frq = 440.0;
	ep->vol = 1.0;
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
	// [pn(6)][sn(4)][val(5)]
	// PN = (partial number + 1) * 512
	// SN = (segment number + 1)* 32
	// PN+0	Frequency multiplier. 
	// PN+1	Initial frequency for this oscillator. 
	// PN+2	Wave table index.
	// PN+3	Starting value for the envelope.
	// PN+4 Sustain-on flag, 1 or 0.
	// PN+SN+5	segment rate
	// PN+SN+6	Level at the end of the segment.
	// PN+SN+7	Segment curve type: 1=linear 2=exponential 3=log.

	while (n-- > 0)
	{
		float val = *valp++;
		bsInt16 idval = *id++;
		bsInt16 pn = (idval >> 9) & 0x3F;
		bsInt16 sn = (idval >> 5) & 0x0F;
		bsInt16 vn = idval & 0x1F;
		if (pn == 0)
		{
			switch (vn)
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
			pSig = &parts[pn-1];
			if (sn == 0)
			{
				switch (vn)
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
				}
			}
			else
			{
				sn--;
				switch (vn)
				{
				case 3:
					pSig->env.SetStart(AmpValue(val));
					break;
				case 4:
					pSig->env.SetSuson((int)val);
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
	}

	AddSynthPart *pEnd = &parts[numParts];
	for (pSig = parts; pSig < pEnd; pSig++)
	{
		if (pSig->mul)
			pSig->osc.SetFrequency(pSig->mul * frq);
		pSig->osc.Reset(initPhs);
		pSig->env.Reset(initPhs);
	}
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
 <part pn="n"  mul="n">
   <osc frq="n" wt="n" />
   <env segs="n" st="n" son="n">
    <seg sn="n" rt="n" lvl="n" ty="t" />
   </env>
 </part>
</instr>
***************/

int AddSynth::Load(XmlSynthElem *parent)
{
	double dval;
	double lvl;
	double rt;
	long ival;

	parent->GetAttribute("parts", ival);
	SetNumParts(ival);

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("part"))
		{
			long pno = -1;
			elem->GetAttribute("pn", pno);
			if (pno < numParts)
			{
				AddSynthPart *pn = &parts[pno];
				elem->GetAttribute("mul", dval);
				pn->mul = (FrqValue) dval;
				XmlSynthElem *partElem = elem->FirstChild();
				while (partElem != NULL)
				{
					if (partElem->TagMatch("osc"))
					{
						partElem->GetAttribute("frq", dval);
						partElem->GetAttribute("wt", ival);
						pn->osc.SetFrequency(FrqValue(dval));
						pn->osc.SetWavetable((int) ival);
					}
					else if (partElem->TagMatch("env"))
					{
						EnvGenSeg *pe = &pn->env;
						long son;
						long nseg = 0;
						partElem->GetAttribute("segs", nseg);
						partElem->GetAttribute("st", lvl);
						partElem->GetAttribute("son", son);
						pe->SetSegs((int)nseg);
						pe->SetStart(AmpValue(lvl));
						pe->SetSuson((int)son);
						XmlSynthElem *segElem = partElem->FirstChild();
						while (segElem != NULL)
						{
							if (segElem->TagMatch("seg"))
							{
								segElem->GetAttribute("sn", nseg);
								segElem->GetAttribute("rt", rt);
								segElem->GetAttribute("lvl", lvl);
								segElem->GetAttribute("ty", ival);
								pe->SetRate((int)nseg, FrqValue(rt));
								pe->SetLevel((int)nseg, AmpValue(lvl));
								pe->SetType((int)nseg, (EGSegType)ival);
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

	parent->SetAttribute("parts", (long)numParts);
	AddSynthPart *p = parts;
	for (int n = 0; n < numParts; n++, p++)
	{
		partElem = parent->AddChild("part");
		if (partElem == NULL)
			return -1;
		partElem->SetAttribute("pn", (long) n);
		partElem->SetAttribute("mul", p->mul);
		subElem = partElem->AddChild("osc");
		if (subElem == NULL)
			return -1;
		subElem->SetAttribute("frq", p->osc.GetFrequency());
		subElem->SetAttribute("wt", (long) p->osc.GetWavetable());
		delete subElem;

		subElem = partElem->AddChild("env");
		if (subElem == NULL)
			return -1;
		EnvGenSeg *pe = &p->env;
		int segs = pe->GetSegs();
		subElem->SetAttribute("segs", (long) segs);
		subElem->SetAttribute("st", (double) pe->GetStart());
		subElem->SetAttribute("son", (long) pe->GetSusOn());
		for (int sn = 0; sn < segs; sn++)
		{
			segElem = subElem->AddChild("seg");
			if (segElem == NULL)
				return -1;
			segElem->SetAttribute("sn", (long) sn);
			segElem->SetAttribute("rt", (double) pe->GetRate(sn));
			segElem->SetAttribute("lvl", (double) pe->GetLevel(sn));
			segElem->SetAttribute("ty", (long) pe->GetType(sn));
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
