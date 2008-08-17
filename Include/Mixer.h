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

		if (pm == panTrig)
		{
			//panlft = sin(panlft * PI/2) * 0.707;
			//panrgt = sin(panrgt * PI/2) * 0.707;
			panlft = synthParams.sinquad[(int)(panlft * synthParams.sqNdx)];
			panrgt = synthParams.sinquad[(int)(panrgt * synthParams.sqNdx)];
		}
		else if (pm == panSqr)
		{
			//panlft = sqrt(panlft) * 0.707;
			//panrgt = sqrt(panrgt) * 0.707;
			panlft = synthParams.sqrttbl[(int)(panlft * synthParams.sqNdx)];
			panrgt = synthParams.sqrttbl[(int)(panrgt * synthParams.sqNdx)];
		}
	}
};

class MixChannel 
{
private:
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
		left  += val * pan.panlft;
		right += val * pan.panrgt;
	}

	void In2(AmpValue lft, AmpValue rgt)
	{
		left += lft * volume;
		right += rgt * volume;
	}

	void Out(AmpValue &lval, AmpValue& rval)
	{
		lval = left;
		rval = right;
		left = 0;
		right = 0;
	}

	void Clear()
	{
		left = 0;
		right = 0;
	}
};


class Mixer
{
private:
	int mixInputs;
	MixChannel *inBuf;
	AmpValue lvol;
	AmpValue rvol;

public:
	Mixer()
	{
		mixInputs = 0;
		lvol = 1.0;
		rvol = 1.0;
		inBuf = 0;
	}

	~Mixer()
	{
		delete[] inBuf;
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

	void MasterVolume(AmpValue lv, AmpValue rv)
	{
		lvol = lv;
		rvol = rv;
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

	void Out(AmpValue *lval, AmpValue *rval)
	{
		AmpValue lvalIn;
		AmpValue rvalIn;
		AmpValue lvalOut = 0;
		AmpValue rvalOut = 0;
		MixChannel *pin = inBuf;
		for (int n = 0; n < mixInputs; n++)
		{
			if (pin->IsOn())
			{
				pin->Out(lvalIn, rvalIn);
				lvalOut += lvalIn;
				rvalOut += rvalIn;
			}
			pin++;
		}

		*lval = lvalOut * lvol;
		*rval = rvalOut * rvol;
	}

	void Reset()
	{
		for (int n = 0; n < mixInputs; n++)
			inBuf[n].Clear();
	}
};

#endif
