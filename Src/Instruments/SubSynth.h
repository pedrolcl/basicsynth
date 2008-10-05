//////////////////////////////////////////////////////////////////////
// BasicSynth Subtractive synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
//////////////////////////////////////////////////////////////////////

#if !defined(_SUBSYNTHINSTR_H_)
#define _SUBSYNTHINSTR_H_

#include "LFO.h"
#include "PitchBend.h"

class SubSynthEvent : public VarParamEvent
{
public:
	bsInt16 MaxParam() { return 57; }
};

class SubSynth  : public Instrument
{
private:
	GenWaveWT    osc;
	GenNoise nz;
	EnvGenADSR   envSig;
	//FilterLP     filt;
	//Reson        filt;
	//EnvGenADSR   envFlt;
	DynFilterLP filt;
	LFO lfoGen;
	PitchBend pbGen;
	AmpValue vol;
	AmpValue sigMix;
	AmpValue nzMix;
	AmpValue fltGain;
	float fltRes;
	int chnl;
	int nzOn;
	int pbOn;
	InstrManager *im;

	void SetParams(SubSynthEvent *evt);

public:
	SubSynth();
	virtual ~SubSynth();
	static Instrument *SubSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *SubSynthEventFactory(Opaque tmplt);
	void Copy(SubSynth *tp);
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
};

#endif
