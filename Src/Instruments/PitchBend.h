//////////////////////////////////////////////////////////////////////
// BasicSynth pitch bend unit
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _PITCHBEND_H_
#define _PITCHBEND_H_

#define PB_RATES 2
#define PB_AMNTS 3

class PitchBend : public GenUnit
{
private:
	FrqValue frq;
	FrqValue beg;
	FrqValue end;
	FrqValue val;
	FrqValue mul;
	FrqValue rate[PB_RATES];
	FrqValue amnt[PB_AMNTS];
	long count;
	int state;

	void CalcMul();

public:
	PitchBend();

	void Copy(PitchBend *pb);

	void SetFrequency(FrqValue f) { frq = f; }
	FrqValue GetFrequency() { return frq; }

	void SetAmount(int n, FrqValue a)
	{
		if (n < PB_AMNTS)
			amnt[n] = a;
	}

	FrqValue GetAmount(int n)
	{
		if (n < PB_AMNTS)
			return amnt[n];
		return 0;
	}

	void SetRate(int n, FrqValue rt)
	{
		if (n < PB_RATES)
			rate[n] = rt;
	}

	FrqValue GetRate(int n)
	{
		if (n < PB_RATES)
			return rate[n];
		return 0;
	}

	void Init(int n, float *p);
	void Reset(float initPhs = 0);
	void InitPB(FrqValue f, FrqValue a1, FrqValue r1, FrqValue a2, FrqValue r2, FrqValue a3);
	AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}
	AmpValue Gen();

	int IsFinished()
	{
		return state > 1;
	}

	int On() { return amnt[0] != amnt[1] || amnt[1] != amnt[2]; }
	int Load(XmlSynthElem *elem);
	int Save(XmlSynthElem *elem);
};

#endif
