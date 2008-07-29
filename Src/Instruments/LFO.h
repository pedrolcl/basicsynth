//////////////////////////////////////////////////////////////////////
// LFO: Low Frequency Oscillator
//
// Used for vibrato and similar effects.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LFO_H_)
#define _LFO_H_

class LFO : public GenUnit
{
private:
	GenWave32 osc;
	EnvSegExp atk;
	int lfoOn;

public:
	LFO();
	virtual ~LFO();
	void Copy(LFO *tp);
	
	void SetFrequency(FrqValue frq) { osc.SetFrequency(frq); }
	void SetWavetable(int wt)       { osc.SetWavetable(wt); }
	void SetAttack(FrqValue val)    { atk.SetRate(val); }
	void SetLevel(AmpValue val)     { atk.SetLevel(val); }

	void Init(int n, float *f);
	void InitLFO(FrqValue frq, int wvf, FrqValue rt, AmpValue amp);
	void GetSettings(FrqValue &frq, int &wvf, FrqValue& rt, AmpValue& amp);
	void Reset(float initPhs = 0);
	AmpValue Sample(AmpValue in);
	AmpValue Gen();

	int On() { return lfoOn; }
	int Load(XmlSynthElem *elem);
	int Save(XmlSynthElem *elem);
};

#endif
