// LFO.cpp: implementation of the LFO class.
//
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "LFO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LFO::LFO()
{
	osc.InitWT(3.5, WT_SIN);
	atk.InitSeg(0, 0, 0.1);
	lfoOn = 1;
}

LFO::~LFO()
{

}

void LFO::InitLFO(FrqValue frq, int wvf, FrqValue rt, AmpValue amp)
{
	osc.InitWT(frq, wvf);
	atk.InitSeg(rt, 0, amp);
	lfoOn = amp > 0;
}

void LFO::GetSettings(FrqValue &frq, int &wvf, FrqValue& rt, AmpValue& amp)
{
	frq = osc.GetFrequency();
	wvf = osc.GetWavetable();
	AmpValue tmp;
	atk.GetSettings(rt, tmp, amp);
}

void LFO::Copy(LFO *tp)
{
	osc.SetFrequency(tp->osc.GetFrequency());
	osc.SetWavetable(tp->osc.GetWavetable());
	atk.Copy(&tp->atk);
}

void LFO::Init(int n, float *f)
{
	if (n == 4)
		InitLFO(f[0], (int)f[1], f[2], f[3]);
}

void LFO::Reset(float initPhs)
{
	osc.Reset(initPhs);
	atk.Reset(initPhs);
}

AmpValue LFO::Sample(AmpValue in)
{
	return Gen();
}

AmpValue LFO::Gen()
{
	return atk.Gen() * osc.Gen();
}

int LFO::Load(XmlSynthElem *elem)
{
	double dvals[3];
	long ival;

	elem->GetAttribute("frq", dvals[0]);
	elem->GetAttribute("wt", ival);
	elem->GetAttribute("atk", dvals[1]);
	elem->GetAttribute("amp", dvals[2]);
	InitLFO(FrqValue(dvals[0]), (int)ival, FrqValue(dvals[1]), AmpValue(dvals[2]));
	return 0;
}


int LFO::Save(XmlSynthElem *elem)
{
	elem->SetAttribute("frq", (double) osc.GetFrequency());
	elem->SetAttribute("wt",  (long) osc.GetWavetable());
	elem->SetAttribute("atk", (double)atk.GetRate());
	elem->SetAttribute("amp", (double) atk.GetLevel());
	return 0;
}

