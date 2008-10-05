//////////////////////////////////////////////////////////////////////
// BasicSynth FM Synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
//////////////////////////////////////////////////////////////////////

#if !defined(_FMSYNTH_H_)
#define _FMSYNTH_H_

#include "LFO.h"
#include "PitchBend.h"

#define ALG_STACK 1
#define ALG_STACK2 2
#define ALG_WYE 3
#define ALG_DELTA 4

class FMSynth : public Instrument
{
private:
	GenWaveWT  gen1Osc;
	EnvGenADSR gen1EG;
	EnvDef     gen1EnvDef;
	FrqValue   gen1Mult;
	AmpValue   fmMix;
	AmpValue   fmDly;
	long algorithm;

	AmpValue   panSet;
	AmpValue   panLft;
	AmpValue   panRgt;

	GenWaveWT  gen2Osc;
	EnvGenADSR gen2EG;
	EnvDef     gen2EnvDef;
	FrqValue   gen2Mult;

	GenWaveWT  gen3Osc;
	EnvGenADSR gen3EG;
	EnvDef     gen3EnvDef;
	FrqValue   gen3Mult;

	GenNoiseI  nzi;
	GenWaveWT  nzo;
	EnvGenADSR nzEG;
	EnvDef     nzEnvDef;
	FrqValue   nzFrqh;
	FrqValue   nzFrqo;
	AmpValue   nzMix;
	AmpValue   nzDly;

	AllPassDelay apd;
	FrqValue   dlyTim;
	FrqValue   dlyDec;
	AmpValue   dlyMix;
	long dlySamps;

	LFO lfoGen;
	PitchBend pbGen;

	int panOn;
	int nzOn;
	int dlyOn;
	int pbOn;

	int chnl;
	FrqValue frq;
	AmpValue vol;
	AmpValue maxPhs;

	InstrManager *im;

	AmpValue CalcPhaseMod(AmpValue amp, FrqValue mult);
	void LoadEG(XmlSynthElem *elem, EnvDef& eg);
	XmlSynthElem *SaveEG(XmlSynthElem *parent, char *tag, EnvDef& eg);

	void SetParams(VarParamEvent *evt);

public:
	FMSynth();
	virtual ~FMSynth();
	static Instrument *FMSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *FMSynthEventFactory(Opaque tmplt);
	void Copy(FMSynth *tp);
	void Start(SeqEvent *evt);
	void Param(SeqEvent *evt);
	void Stop();
	void Tick();
	int  IsFinished();
	void Destroy();

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
};

#endif
