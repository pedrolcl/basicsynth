#ifndef _REVERB_H_
#define _REVERB_H_

#include "DelayLine.h"

// Single resonator with one-pole LP filter added to feedback
class Reverb1 : public DelayLineR
{
private:
	AmpValue atten;
	AmpValue prev;
public:
	Reverb1()
	{
		atten = 1.0;
		prev = 0.0;
	}

	void Init(int n, float *p)
	{
		if (n >= 3)
			InitReverb(p[0], p[1], p[2]);
	}

	// a = input attenuation
	// lt = Loop time
	// rt = Reverb time
	void InitReverb(AmpValue a, FrqValue lt, FrqValue rt)
	{
		atten = a;
		DelayLineR::InitDLR(lt, rt, 0.001);
		prev = 0.0;
	}

	AmpValue Sample(AmpValue vin)
	{
		AmpValue out = *delayPos * decayFactor;
		AmpValue vlp = (out + prev) / 2;
		prev = vlp;
		*delayPos = (vin * atten) + vlp;
		if (++delayPos >= delayEnd)
			delayPos = delayBuf;
		return out;
	}
};

// Schroeder reverb - four comb filters + two allpass filters
class Reverb2 : public GenUnit
{
private:
	DelayLineR dlr[4];
	AllPassDelay ap[2];
	AmpValue atten;
public:
	Reverb2()
	{
		atten = 1.0f;
	}

	void Init(int n, float *p)
	{
		if (n > 1)
			InitReverb(p[0], p[1]);
	}

	void Reset(float initPhs = 0)
	{
		for (int i = 0; i < 4; i++)
			dlr[0].Reset(initPhs);
		ap[0].Reset(initPhs);
		ap[1].Reset(initPhs);
	}

	void InitReverb(AmpValue a, FrqValue rt)
	{
		atten = a;
		dlr[0].InitDLR(0.0437, rt, 0.001);
		dlr[1].InitDLR(0.0411, rt, 0.001);
		dlr[2].InitDLR(0.0371, rt, 0.001);
		dlr[3].InitDLR(0.0297, rt, 0.001);
		ap[0].InitDLR(0.0050, 0.09683, 0.001);
		ap[1].InitDLR(0.0017, 0.03292, 0.001);
	}

	AmpValue Sample(AmpValue vin)
	{
		vin *= atten;
		AmpValue out = dlr[0].Sample(vin) + dlr[1].Sample(vin) + dlr[2].Sample(vin) + dlr[3].Sample(vin);
		return ap[1].Sample(ap[0].Sample(out));
	}
};


#endif
