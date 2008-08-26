///////////////////////////////////////////////////////////////
//
// BasicSynth - Mixer
//
// Mixer and Mixer channel with panning
//
// Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _MIXER_H_
#define _MIXER_H_

#define panOff 0
#define panLin 1
#define panTrig 2
#define panSqr 3

class Panner
{
public:
	AmpValue panlft;
	AmpValue panrgt;

	Panner()
	{
		panlft = 0.5;
		panrgt = 0.5;
	}

	void Set(int pm, AmpValue p)
	{
		if (pm == panOff)
		{
			panlft = 0.5;
			panrgt = 0.5;
			return;
		}
		panlft = (1 - p) / 2;
		panrgt = (1 + p) / 2;
		// optional: range 0-1
		// panlft = 1 - p;
		// panrgt = p;

		if (pm == panTrig)
		{
			//panlft = sin(panlft * PI/2) * sqrt(2)/2;
			//panrgt = sin(panrgt * PI/2) * sqrt(2)/2;
			panlft = synthParams.sinquad[(int)(panlft * synthParams.sqNdx)];
			panrgt = synthParams.sinquad[(int)(panrgt * synthParams.sqNdx)];
		}
		else if (pm == panSqr)
		{
			//panlft = sqrt(panlft) * sqrt(2)/2;
			//panrgt = sqrt(panrgt) * sqrt(2)/2;
			panlft = synthParams.sqrttbl[(int)(panlft * synthParams.sqNdx)];
			panrgt = synthParams.sqrttbl[(int)(panrgt * synthParams.sqNdx)];
		}
	}
};

class FxChannel
{
public:
	GenUnit *fx;
	AmpValue *fxlvl; // one for each input channel
	AmpValue value;  // total input
	AmpValue fxmix;  // output level
	Panner   pan;
	int init;

	FxChannel()
	{
		init = 0;
		fx = 0;
		fxlvl = 0;
		value = 0;
		fxmix = 0;
	}

	~FxChannel()
	{
		delete[] fxlvl;
	}

	void FxIn(int ch, AmpValue val)
	{
		if (init)
			value += fxlvl[ch] * val;
	}

	void FxIn(AmpValue val)
	{
		value += val;
	}

	void FxOut(AmpValue& lft, AmpValue& rgt)
	{
		if (init)
		{
			AmpValue out = fx->Sample(value) * fxmix;
			lft = out * pan.panlft;
			rgt = out * pan.panrgt;
			value = 0;
		}
		else
		{
			lft = 0;
			rgt = 0;
		}
	}

	void FxInit(GenUnit *p, int ch, AmpValue lvl)
	{
		if (fxlvl)
		{
			delete[] fxlvl;
			fxlvl = 0;
		}
		fx = 0;
		if (p && ch)
		{
			fxlvl = new AmpValue[ch];
			for (int n = 0; n < ch; n++)
				fxlvl[n] = 0;
			fx = p;
			init = 1;
		}
		fxmix = lvl;
	}

	void FxSendSet(int ch, AmpValue lvl)
	{
		if (init)
			fxlvl[ch] = lvl;
	}

	AmpValue FxSendGet(int ch)
	{
		if (init)
			return fxlvl[ch];
		return 0;
	}

	void FxOutSet(AmpValue lvl)
	{
		fxmix = lvl;
	}

	AmpValue FxOutGet()
	{
		return fxmix;
	}

	void FxPanSet(int pm, AmpValue lvl)
	{
		pan.Set(pm, lvl);
	}

	void Clear()
	{
		value = 0;
		fx->Reset();
	}
};

class MixChannel 
{
private:
	AmpValue both;
	AmpValue left;
	AmpValue right;
	AmpValue volume;
	AmpValue panset;
	Panner pan;
	int   method;
	int   on;

public:
	MixChannel()
	{
		volume = 0.5;
		on = false;
		both = 0;
		left = 0;
		right = 0;
		panset = 0;
		method = 0;
	}

	void SetOn(int n)
	{
		on = n;
	}

	int IsOn()
	{
		return on;
	}

	void SetVolume(AmpValue v)
	{
		volume = v;
	}

	AmpValue GetVolume() 
	{
		return volume;
	}

	void SetPan(int pm, AmpValue p)
	{
		panset = p;
		method = pm;
		pan.Set(pm, p);
	}

	AmpValue GetPan()
	{
		return panset;
	}

	void In(AmpValue val)
	{
		val *= volume;
		both  += val;
		left  += val * pan.panlft;
		right += val * pan.panrgt;
	}

	void In2(AmpValue lft, AmpValue rgt)
	{
		// N.B. : bypass panning and effects!
		left += lft * volume;
		right += rgt * volume;
	}

	AmpValue Level()
	{
		return both;
	}

	void Out(AmpValue &lval, AmpValue& rval)
	{
		lval = left;
		rval = right;
		left = 0;
		right = 0;
		both = 0;
	}

	void Clear()
	{
		left = 0;
		right = 0;
		both = 0;
	}
};


class Mixer
{
private:
	int mixInputs;
	int fxUnits;
	MixChannel *inBuf;
	FxChannel *fxBuf;
	AmpValue lvol;
	AmpValue rvol;

public:
	Mixer()
	{
		mixInputs = 0;
		fxUnits = 0;
		lvol = 1.0;
		rvol = 1.0;
		inBuf = 0;
		fxBuf = 0;
	}

	~Mixer()
	{
		if (inBuf)
			delete[] inBuf;
		if (fxBuf)
			delete[] fxBuf;
	}

	void MasterVolume(AmpValue lv, AmpValue rv)
	{
		lvol = lv;
		rvol = rv;
	}

	void SetChannels(int nchnl)
	{
		delete[] inBuf;
		mixInputs = nchnl;
		inBuf = new MixChannel[nchnl];
	}

	int GetChannels()
	{
		return mixInputs;
	}

	void ChannelOn(int ch, int on)
	{
		if (ch < mixInputs)
			inBuf[ch].SetOn(on);
	}

	void ChannelVolume(int ch, AmpValue v)
	{
		if (ch < mixInputs)
			inBuf[ch].SetVolume(v);
	}

	void ChannelPan(int ch, int pm, AmpValue p)
	{
		if (ch < mixInputs)
			inBuf[ch].SetPan(pm, p);
	}

	void ChannelIn(int ch, AmpValue val)
	{
		// warning - no runtime range check here...
		inBuf[ch].In(val);
	}

	void ChannelIn2(int ch, AmpValue lft, AmpValue rgt)
	{
		// warning - no runtime range check here...
		inBuf[ch].In2(lft, rgt);
	}

	void SetFxChannels(int n)
	{
		if (fxBuf)
		{
			delete[] fxBuf;
			fxBuf = 0;
		}
		fxUnits = n;
		if (n > 0)
			fxBuf = new FxChannel[n];
	}

	int GetFxChannels()
	{
		return fxUnits;
	}

	void FxInit(int f, GenUnit *fx, AmpValue lvl)
	{
		// NB: Must set mixInputs first.
		if (mixInputs > 0 && f < fxUnits)
			fxBuf[f].FxInit(fx, mixInputs, lvl);
	}

	void FxLevel(int f, int ch, AmpValue lvl)
	{
		if (f < fxUnits)
			fxBuf[f].FxSendSet(ch, lvl);
	}
	
	void FxPan(int f, int pm, AmpValue lvl)
	{
		if (f < fxUnits)
			fxBuf[f].FxPanSet(pm, lvl);
	}

	// direct effects send, bypass input channel
	void FxIn(int f, AmpValue val)
	{
		fxBuf[f].FxIn(val);
	}

	void Out(AmpValue *lval, AmpValue *rval)
	{
		int n;
		AmpValue lvalIn;
		AmpValue rvalIn;
		AmpValue lvalOut = 0;
		AmpValue rvalOut = 0;
		FxChannel *fx, *fxe;
		MixChannel *pin = inBuf;
		for (n = 0; n < mixInputs; n++)
		{
			if (pin->IsOn())
			{
				if ((fx = fxBuf) != 0)
				{
					fxe = &fxBuf[fxUnits];
					while (fx < fxe)
					{
						fx->FxIn(n, pin->Level());
						fx++;
					}
				}
				pin->Out(lvalIn, rvalIn);
				lvalOut += lvalIn;
				rvalOut += rvalIn;

			}
			pin++;
		}

		if ((fx = fxBuf) != 0)
		{
			fxe = &fxBuf[fxUnits];
			while (fx < fxe)
			{
				fx->FxOut(lvalIn, rvalIn);
				lvalOut += lvalIn;
				rvalOut += rvalIn;
				fx++;
			}
		}

		*lval = lvalOut * lvol;
		*rval = rvalOut * rvol;
	}

	void Reset()
	{
		int n;
		for (n = 0; n < mixInputs; n++)
			inBuf[n].Clear();
		for (n = 0; n < fxUnits; n++)
			fxBuf[n].Clear();
	}
};

#endif
