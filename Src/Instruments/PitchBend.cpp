//////////////////////////////////////////////////////////////////////
// BasicSynth pitch bend unit
//
// The pitch bend unit provides a specialized envelope with two segments.
// The segments follow a curve that maps onto equal frequency change
// i.e. 2^(n/12) * f.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "Includes.h"
#include "PitchBend.h"

PitchBend::PitchBend()
{
	state = 2;
	beg = 0;
	end = 0;
	val = 0;
	mul = 1.0;
	frq = 0;
	int n;
	for (n = 0; n < PB_RATES; n++)
		rate[n] = 0;
	for (n = 0; n < PB_AMNTS; n++)
		amnt[n] = 0;
}

void PitchBend::Copy(PitchBend *pb)
{
	pb->frq = frq;
	int n;
	for (n = 0; n < PB_RATES; n++)
		pb->rate[n] = rate[n];
	for (n = 0; n < PB_AMNTS; n++)
		pb->amnt[n] = amnt[n];
}

void PitchBend::CalcMul()
{
	count = (long) (rate[state] * synthParams.sampleRate);
	beg = frq;
	end = frq;
	FrqValue a1 = amnt[state];
	FrqValue a2 = amnt[state+1];
	if (count > 0 && (a1 != a2))
	{
		if (a1 != 0)
			beg *= pow(2.0, a1 / 12.0);
		if (a2 != 0)
			end *= pow(2.0, a2 / 12.0);
		mul = pow((double)end / (double)beg, 1.0 / (double)count);
	}
	else
	{
		mul = 1.0;
	}
	val = beg;
}

void PitchBend::Init(int n, float *p)
{
	if (n >= 6)
		InitPB(FrqValue(p[0]), AmpValue(p[1]),
		       FrqValue(p[2]), AmpValue(p[3]),
			   FrqValue(p[4]), AmpValue(p[5]));
}

void PitchBend::InitPB(FrqValue f, FrqValue a1, FrqValue r1, FrqValue a2, FrqValue r2, FrqValue a3)
{
	frq = f;
	amnt[0] = a1;
	amnt[1] = a2;
	amnt[2] = a3;
	rate[0] = r1;
	rate[1] = r2;
	Reset();
}

void PitchBend::Reset(float initPhs)
{
	state = 0;
	CalcMul();
}

AmpValue PitchBend::Gen()
{
	switch (state)
	{
	case 0:
	case 1:
		if (--count > 0)
			val *= mul;
		else if (++state < 2)
			CalcMul();
		break;
	case 2:
		break;
	}
	return val - frq;
}

int PitchBend::Load(XmlSynthElem *elem)
{
	elem->GetAttribute("frq", frq);
	elem->GetAttribute("r1", rate[0]);
	elem->GetAttribute("r2", rate[1]);
	elem->GetAttribute("a1", amnt[0]);
	elem->GetAttribute("a2", amnt[1]);
	elem->GetAttribute("a3", amnt[2]);
	return 0;
}


int PitchBend::Save(XmlSynthElem *elem)
{
	//elem->SetAttribute("frq", frq);
	elem->SetAttribute("r1", rate[0]);
	elem->SetAttribute("r2", rate[1]);
	elem->SetAttribute("a1", amnt[0]);
	elem->SetAttribute("a2", amnt[1]);
	elem->SetAttribute("a3", amnt[2]);
	return 0;
}
