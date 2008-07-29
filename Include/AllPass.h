///////////////////////////////////////////////////////////
// AllPass.h
//
// BasicSynth all-pass filter class
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////
#ifndef _ALLPASS_H_
#define _ALLPASS_H_

class AllPassFilter : public GenUnit
{
private:
	AmpValue gain;
	AmpValue prevX;
	AmpValue prevY;

public:
	AllPassFilter()
	{
		gain = 0.5;
		prevX = 0;
		prevY = 0;
	}

	void Init(int n, float *p)
	{
		if (n > 0)
			InitAP(p[0]);
	}

	void Reset(float initPhs = 0)
	{
		prevX = 0;
		prevY = 0;
	}

	void InitAP(float d)
	{
		gain = (1.0 - d) / (1.0 + d);
	}

	AmpValue Sample(AmpValue val)
	{
		AmpValue out;
		out = (gain * val) + prevX - (gain * prevY);
		prevX = val;
		prevY = out;
		return out;
		// Direct Form II
		//out = prevY;
		//prevY =  val - (prevY * gain);
		//return out + (prevY * gain);
	}
};

#endif
