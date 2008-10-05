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
	AmpValue amp;
	AmpValue prevX;
	AmpValue prevY;

public:
	AllPassFilter()
	{
		amp = 0;
		prevX = 0;
		prevY = 0;
	}

	void Init(int n, float *v)
	{
		if (n > 0)
			InitAP(v[0]);
	}

	void Reset(float initPhs = 0)
	{
		prevX = 0;
		prevY = 0;
	}

	void InitAP(float d)
	{
		amp = (1.0 - d) / (1.0 + d);
	}

	AmpValue Sample(AmpValue val)
	{
		AmpValue out;
		out = (amp * val) + prevX - (amp * prevY);
		prevX = val;
		prevY = out;
		return out;
		// Alternate:
		//out = prevY;
		//prevY = val - (prevY * amp);
		//return out + (prevY * amp);
	}
};

#endif
