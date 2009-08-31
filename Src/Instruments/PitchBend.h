//////////////////////////////////////////////////////////////////////
/// @file PitchBend.h BasicSynth pitch bend units
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef _PITCHBEND_H_
#define _PITCHBEND_H_

#define PB_RATES 2
#define PB_AMNTS 3

/// PitchBend generator #1.
/// PitchBend is a three segment generator that produces
/// exponential curves over a range of cents. There are
/// three levels and two rates. Rate 1 is the time to
/// transition from level 1 to level 2. Rate 2 is the time
/// to transition from level 2 to level 3.
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

/// Pitch bend generator #2.
/// PitchBendWT scans a wavetable containing the pitch bend curve.
/// The table is assumed to be normalized [-1,+1] and is multiplied
/// by an amount value. If the signal frequency is set to non-zero,
/// the amount value is taken to indicated pitch cents. This is converted
/// into a frequency range. If the signal frequency is set to zero,
/// the depth value is 
class PitchBendWT : public GenUnit
{
private:
	AmpValue *wave;
	PhsAccum indexIncr;
	PhsAccum index;
	FrqValue durSec;
	AmpValue depth;
	AmpValue ampLvl;
	AmpValue lastVal;
	FrqValue sigFrq;
	bsInt32  count;
	int wtID;
	int pbOn;

public:
	PitchBendWT();
	virtual ~PitchBendWT();
	void Copy(PitchBendWT *tp);
	
	void SetDuration(FrqValue d)    { durSec = d; count = (bsInt32) (d * synthParams.sampleRate); }
	void SetDurationS(bsInt32 d)    { count = d; } // only used at runtime.
	void SetWavetable(int wt)       { wtID = wt; }
	void SetLevel(AmpValue val)     { depth = val; pbOn = depth > 0; }
	void SetSigFrq(FrqValue val)    { sigFrq = val; }
	FrqValue GetDuration() { return durSec; }
	int GetWavetable() { return wtID; }
	AmpValue GetLevel() { return depth; }
	int On() { return pbOn; }

	void Init(int n, float *f);
	void Reset(float initPhs = 0);

	AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	AmpValue Gen();

	int Load(XmlSynthElem *elem);
	int Save(XmlSynthElem *elem);
};

//@}
#endif
