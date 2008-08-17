// Tone.h: interface for the Toner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TONE_H_)
#define _TONE_H_

#include "LFO.h"

class ToneEvent : public NoteEvent
{
public:
	int waveTable;
	int lfoWaveTable;
	AmpValue startLevel;
	AmpValue atkLevel;
	AmpValue susLevel;
	AmpValue endLevel;
	FrqValue atkRate;
	FrqValue decRate;
	FrqValue relRate;
	EGSegType envType;
	FrqValue lfoFreq;
	FrqValue lfoAtkRate;
	AmpValue lfoAmp;

	// no contsructor - init is done in ToneEventFactory,
	// which should be the only function that instantiates
	// this class!

	virtual void Destroy()
	{
		delete this;
	}

	virtual bsInt16 MaxParam()
	{
		return P_VOLUME+13;
	}

	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case 16:
			waveTable = (int) v;
			break;
		case 17:
			startLevel = v;
			break;
		case 18:
			atkRate = v;
			break;
		case 19:
			atkLevel = v;
			break;
		case 20:
			decRate = v;
			break;
		case 21:
			susLevel = v;
			break;
		case 22:
			relRate = v;
			break;
		case 23:
			endLevel = v;
			break;
		case 24:
			envType = (EGSegType) (int) v;
			break;
		case 25:
			lfoFreq = v;
			break;
		case 26:
			lfoWaveTable = (int) v;
			break;
		case 27:
			lfoAtkRate = v;
			break;
		case 28:
			lfoAmp = v;
			break;
		default:
			NoteEvent::SetParam(id, v);
			break;
		}
	}
};

class ToneInstr : public Instrument  
{
private:
	int chnl;
	AmpValue vol;
	GenWaveWT osc;
	EnvGenADSR env;
	LFO lfoGen;

	InstrManager *im;

	void SetParam(ToneEvent *te);

public:
	static Instrument *ToneFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *ToneEventFactory(Opaque tmplt);
	ToneInstr();
	virtual ~ToneInstr();
	virtual void Copy(ToneInstr *tp);
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
};

class ToneFMEvent : public ToneEvent
{
public:
	AmpValue modIndex;
	FrqValue modMult;

	ToneFMEvent()
	{
		modIndex = 1.0;
		modMult = 1.0;
	}

	virtual bsInt16 MaxParam()
	{
		return P_VOLUME+15;
	}

	void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case 32:
			modIndex = AmpValue(v);
			break;
		case 33:
			modMult = FrqValue(v);
			break;
		default:
			ToneEvent::SetParam(id, v);
			break;
		}
	}
};

class ToneFM : public Instrument  
{
private:
	int chnl;
	AmpValue vol;
	GenWaveFM osc;
	EnvGenADSR env;
	LFO lfoGen;

	InstrManager *im;

	void SetParam(ToneFMEvent *te);

public:
	static Instrument *ToneFMFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *ToneFMEventFactory(Opaque tmplt);
	ToneFM();
	virtual ~ToneFM();
	virtual void Copy(ToneFM *tp);
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
