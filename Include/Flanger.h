/////////////////////////////////////////////////////////////////////
//
// Flanger.h
//
// BasicSynth Flanger/Chorus unit. Uses a variable tap delay line
// to modulate the delay. Allows feedback to provide for allpass filtering. 
//
// See: Jon Dattorro, "Effect Design", Journal of the Audio Engineering
//      Society, Vol 45, No. 10, 1997 October, p. 764
//
// Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////
#ifndef _FLANGER_H_
#define _FLANGER_H_

class Flanger : public GenUnit
{
protected:
	DelayLineV dlv;       // delay line
	GenWave32 wv;         // modulation oscillator
	AmpValue dlyLvl;      // input signal level (0-1)
	AmpValue dlyMix;      // mix (blend) value (0-1)
	AmpValue dlyFeedback; // feedback level (0-1)
	PhsAccum dlyRange;    // delay depth in sec.
	PhsAccum dlyCenter;   // delay center in sec.

public:
	Flanger()
	{
		dlyRange = 0;
		dlyCenter = 0;
		dlyLvl = 0;
		dlyMix = 0;
		dlyFeedback = 0;
	}

	void Clear()
	{
		dlv.Clear();
	}

	void Copy(Flanger *fp)
	{
		dlyLvl = fp->dlyLvl;
		dlyMix = fp->dlyMix;
		dlyFeedback = fp->dlyFeedback;
		dlyRange = fp->dlyRange;
		dlyCenter = fp->dlyCenter;
		wv.SetFrequency(fp->wv.GetFrequency());
		dlv.Copy(&fp->dlv);
	}

	void GetSettings(AmpValue& inlvl, AmpValue& mix, AmpValue& fb, FrqValue& center, FrqValue& depth, FrqValue& sweep)
	{
		inlvl = dlyLvl;
		mix = dlyMix;
		fb = dlyFeedback;
		center = dlyCenter / synthParams.sampleRate;
		depth = (dlyRange * 2) / synthParams.sampleRate;
		sweep = wv.GetFrequency();
	}

	void Init(int n, float *p)
	{
		if (n >= 5)
			InitFlanger(p[0], p[1], p[2], p[3], p[4], p[5]);
	}

	void Reset(float initPhs = 0)
	{
		dlv.Reset(initPhs);
		wv.Reset(initPhs);
	}

	void InitFlanger(AmpValue inlvl, AmpValue mix, AmpValue fb, FrqValue center, FrqValue depth, FrqValue sweep)
	{
		wv.InitWT(sweep, WT_SIN);
		if ((center - depth) < 0)
			center = depth / 2;
		dlv.InitDL(center + (depth/2), mix);
		dlyLvl = inlvl;
		dlyMix = mix;
		dlyFeedback = fb;
		dlyRange = (depth * synthParams.sampleRate) / 2;
		dlyCenter = (center * synthParams.sampleRate);
	}

	AmpValue Sample(AmpValue inval)
	{
		if (dlyFeedback != 0)
			inval -= dlv.Tap(dlyCenter) * dlyFeedback;
		dlv.SetDelay(dlyCenter + (dlyRange * wv.Gen()));
		return (inval * dlyLvl) + dlv.Sample(inval);
	}
};

#endif
