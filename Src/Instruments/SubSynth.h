//////////////////////////////////////////////////////////////////////
// BasicSynth Subtractive synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
//////////////////////////////////////////////////////////////////////

#if !defined(_SUBSYNTHINSTR_H_)
#define _SUBSYNTHINSTR_H_

#include "LFO.h"
#include "PitchBend.h"

class SubFilt
{
protected:
	EnvGenADSR *egFilt;
	AmpValue res;
	AmpValue gain;
public:
	SubFilt()
	{
		egFilt = 0;
		res = 0;
		gain = 0;
	}
	virtual ~SubFilt() { }
	virtual AmpValue Sample(AmpValue in) { return in; }
	virtual void Init(EnvGenADSR *eg, AmpValue g, AmpValue r)
	{
		egFilt = eg;
		gain = g;
		res = r;
	}
	virtual void Reset(float ) { }
	virtual void Copy(SubFilt *tp) { }
};

class SubFiltLP : public SubFilt
{
private:
	FilterLP filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		filt.Init(egFilt->Gen(), gain);
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.Init(egFilt->GetStart(), gain);
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		filt.Copy(&((SubFiltLP*)tp)->filt);
	}
};

class SubFiltHP : public SubFilt
{
private:
	FilterHP filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		filt.Init(egFilt->Gen(), gain);
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.Init(egFilt->GetStart(), gain);
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		filt.Copy(&((SubFiltHP*)tp)->filt);
	}
};

class SubFiltBP : public SubFilt
{
private:
	FilterBP filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		filt.Init(egFilt->Gen(), gain, res);
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.Init(egFilt->GetStart(), gain, res);
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		filt.Copy(&((SubFiltBP*)tp)->filt);
	}
};

class SubFiltRES : public SubFilt
{
private:
	Reson filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		filt.InitRes(egFilt->Gen(), gain, res);
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.InitRes(egFilt->GetStart(), gain, res);
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		filt.Copy(&((SubFiltRES*)tp)->filt);
	}
};

class SubSynth  : public Instrument
{
private:
#ifdef USE_OSCILI
	GenWaveI osc;
#else
	GenWaveWT osc;
#endif
	GenNoise nz;
	EnvGenADSR   envSig;
	SubFilt *filt;
	EnvGenADSR   envFlt;
	LFO lfoGen;
	PitchBend pbGen;
	AmpValue vol;
	AmpValue sigMix;
	AmpValue nzMix;
	AmpValue fltGain;
	AmpValue fltRes; // only if Reson filter
	short fltType; // 0=LP, 1=HP, 2=BP, 3=RES
	int chnl;
	int nzOn;
	int pbOn;
	InstrManager *im;

	void SetParams(VarParamEvent *evt);

public:
	SubSynth();
	virtual ~SubSynth();
	static Instrument *SubSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *SubSynthEventFactory(Opaque tmplt);
	void Copy(SubSynth *tp);
	void CreateFilter();
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
