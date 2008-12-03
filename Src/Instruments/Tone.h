//////////////////////////////////////////////////////////////////////
// BasicSynth Tone instruments
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#if !defined(_TONE_H_)
#define _TONE_H_

#include "LFO.h"
#include "PitchBend.h"

class ToneBase : public Instrument  
{
protected:
	int chnl;
	int pbOn;
	int lfoOn;
	AmpValue vol;
	GenWaveWT *osc;
	EnvGenADSR env;
	LFO lfoGen;
	PitchBend pbGen;

	InstrManager *im;

public:
	ToneBase();
	ToneBase(ToneBase *tp);
	virtual ~ToneBase();
	virtual void Copy(ToneBase *tp);
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	virtual int Load(XmlSynthElem *parent);
	virtual int Save(XmlSynthElem *parent);
	virtual int GetParams(VarParamEvent *params);
	virtual int SetParams(VarParamEvent *params);

	virtual int LoadOscil(XmlSynthElem *elem);
	virtual int LoadEnv(XmlSynthElem *elem);
	virtual int SaveOscil(XmlSynthElem *elem);
	virtual int SaveEnv(XmlSynthElem *elem);
	virtual int SetParam(int id, float val);
};

class ToneInstr : public ToneBase
{
public:
	static Instrument *ToneFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *ToneEventFactory(Opaque tmplt);
	static bsInt16     MapParamID(const char *name);

	ToneInstr();
	ToneInstr(ToneInstr *tp);
	virtual ~ToneInstr();
};

class ToneFM : public ToneBase
{
public:
	static Instrument *ToneFMFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *ToneFMEventFactory(Opaque tmplt);
	static bsInt16     MapParamID(const char *name);

	ToneFM();
	ToneFM(ToneFM *tp);
	virtual ~ToneFM();
	virtual void Copy(ToneFM *tp);
	virtual int LoadOscil(XmlSynthElem *elem);
	virtual int SaveOscil(XmlSynthElem *elem);
	virtual int SetParam(int id, float val);

	int GetParams(VarParamEvent *params);
};
#endif
