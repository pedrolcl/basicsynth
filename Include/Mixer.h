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
#define panLog 2
#define panAtn 3

class MixChannel 
{
private:
	AmpValue left;
	AmpValue right;
	AmpValue volume;
	AmpValue panset;
	AmpValue panlft;
	AmpValue panrgt;
	int   method;
	int   on;

public:
	MixChannel()
	{
		volume = 0.5;
		panset = 0;
		panlft = 1;
		panrgt = 1;
		method = 0;
		on = false;
		left = 0;
		right = 0;
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
		method = pm;
		panset = p;
		if (pm == panLin)
		{
			panlft = (1 - p) / 2;
			panrgt = (1 + p) / 2;
		}
		else if (pm == panLog)
		{
			float r = (p + 45.0) * (twoPI / 360.0);
			panlft = cos(r);
			panrgt = sin(r);
		}
		else if (pm == panAtn)
		{
			if (p > 0)
			{
				panlft = 1 - p;
				panrgt = 1;
			}
			else if (p < 0)
			{
				panlft = 1;
				panrgt = 1 + p;
			} 
			else
			{
				panlft = 1;
				panrgt = 1;
			}
		}
		else
		{
			panlft = 1;
			panrgt = 1;
		}
	}

	AmpValue GetPan()
	{
		return panset;
	}

	void In(AmpValue val)
	{
		val *= volume;
		left  += val * panlft;
		right += val * panrgt;
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
