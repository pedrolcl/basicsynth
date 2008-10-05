///////////////////////////////////////////////////////////////
//
// BasicSynth - BiQuad
//
// BiQuad filters.
//
// LowPass, High-pass and Band-pass Butterworth filters
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _BIQUAD_H_
#define _BIQUAD_H_

#define sqr2 1.414213562

class BiQuadFilter : public GenUnit
{
public:
	AmpValue gain;
	AmpValue ampIn0;
	AmpValue ampIn1;
	AmpValue ampIn2;
	AmpValue ampOut1;
	AmpValue ampOut2;
	AmpValue dlyIn1;
	AmpValue dlyIn2;
	AmpValue dlyOut1;
	AmpValue dlyOut2;
	FrqValue cutoff;

	BiQuadFilter()
	{
		cutoff = 0;
		gain = 0;
		ampIn0 = 0;
		ampIn1 = 0;
		ampIn2 = 0;
		ampOut1 = 0;
		ampOut2 = 0;
		dlyIn1 = 0;
		dlyIn2 = 0;
		dlyOut1 = 0;
		dlyOut2 = 0;
	}

	virtual void Copy(BiQuadFilter *filt)
	{
		filt->cutoff = cutoff;
		filt->gain = gain;
		filt->ampIn0 = ampIn0;
		filt->ampIn1 = ampIn1;
		filt->ampIn2 = ampIn2;
		filt->ampOut1 = ampOut1;
		filt->ampOut2 = ampOut2;
		filt->dlyIn1 = dlyIn1;
		filt->dlyIn2 = dlyIn2;
		filt->dlyOut1 = dlyOut1;
		filt->dlyOut2 = dlyOut2;
	}

	virtual void  Init(int n, float *v)
	{
		if (n > 1)
			Init((FrqValue) v[0], (AmpValue) v[1]);
		else if (n > 0)
			Init((FrqValue) v[0], 1);
	}

	virtual void Init(FrqValue cu, AmpValue g)
	{
		cutoff = cu;
		gain = g;
	}

	virtual void Reset(float initPhs)
	{
		dlyIn1 = 0;
		dlyIn2 = 0;
		dlyOut1 = 0;
		dlyOut2 = 0;
	}

	virtual AmpValue Sample(AmpValue vin)
	{
		// Direct Form I
		AmpValue out = (ampIn0 * vin) + (ampIn1 * dlyIn1) + (ampIn2 * dlyIn2)
		             - (ampOut1 * dlyOut1) - (ampOut2 * dlyOut2);
		dlyOut2 = dlyOut1;
		dlyOut1 = out;
		dlyIn2 = dlyIn1;
		dlyIn1 = vin;

		// Direct Form II
		//AmpValue tmp = vin - (ampOut1 * dlyOut1) - (ampOut2 * dlyOut2);
		//AmpValue out = (ampIn0 * tmp) + (ampIn1 * dlyOut1) + (ampIn2 * dlyOut2);
		//dlyOut2 = dlyOut1;
		//dlyOut1 = tmp;
		return out * gain;
	}
};

class FilterLP : public BiQuadFilter
{
public:
	virtual void Init(FrqValue cu, AmpValue g)
	{
		FrqValue old = cutoff;
		BiQuadFilter::Init(cu, g);

		if (old != cutoff)
		{
			double c = 1 / tan((PI / synthParams.sampleRate) * cutoff);
			double c2 = c * c;
			double csqr2 = sqr2 * c;
			double oned = 1.0 / (c2 + csqr2 + 1.0);

			ampIn0 = oned;
			ampIn1 = oned + oned;
			ampIn2 = oned;
			ampOut1 = (2.0 * (1.0 - c2)) * oned;
			ampOut2 = (c2 - csqr2 + 1.0) * oned;
		}
	}
};

class FilterHP : public BiQuadFilter
{
public:
	virtual void Init(FrqValue cu, AmpValue g)
	{
		FrqValue old = cutoff;
		BiQuadFilter::Init(cu, g);

		if (old != cutoff)
		{
			double c = tan(PI / synthParams.sampleRate * cutoff);
			double c2 = c * c;
			double csqr2 = sqr2 * c;
			double oned = 1.0 / (1.0 + c2 + csqr2);

			ampIn0 = (AmpValue) oned;   // 1/d
			ampIn1 = -(ampIn0 + ampIn0);                      // -2/d
			ampIn2 = ampIn0;                                  // 1/d
			ampOut1 = (AmpValue) ((2.0 * (c2 - 1.0)) * oned);
			ampOut2 = (AmpValue) ((1.0 - csqr2 + c2) * oned);
		}
	}
};

class FilterBP : public BiQuadFilter
{
private:
	float bw;
public:
	virtual void Init(FrqValue cu, AmpValue g)
	{
		Init(cu, g, 1000.0);
	}

	virtual void Init(FrqValue cu, AmpValue g, float B)
	{
		FrqValue old = cutoff;
		BiQuadFilter::Init(cu, g);

		if (old != cutoff || bw != B)
		{
			bw = B;
			double r = PI / synthParams.sampleRate;
			double c = 1.0 / tan(r * B);
			//double d = 2.0 * cos(2.0 * r * cutoff);
			double d = 2.0 * cos(synthParams.frqRad * cutoff);
			double oned = 1.0 / (1.0 + c);

			ampIn0 = oned;
			ampIn1 = 0;
			ampIn2 = -oned;
			ampOut1 = -c * d * oned;
			ampOut2 = (c - 1.0) * oned;
		}
	}
};

// Constant gain Resonantor
// See J. Smith, "Introduction to Digital Filters", Appendix B
// and also Perry Cook, "Real Sound Synthesis", Chapter 3
class Reson : public BiQuadFilter
{
private:
	float res;

public:
	Reson()
	{
		res = 1.0;
	}

	virtual void Init(FrqValue cu, AmpValue g)
	{
		InitRes(cu, g, 1);
	}

	virtual void InitRes(FrqValue cu, AmpValue g, float r)
	{
		FrqValue old = cutoff;
		BiQuadFilter::Init(cu, g);

		if (old != cutoff || res != r)
		{
			res = r;
			ampOut1 = -(res + res) * cos(synthParams.frqRad * cutoff);
			ampOut2 = res * res;
			ampIn0 = 0.5 - (0.5 * ampOut2);
			ampIn1 = 0;
			ampIn2 = -ampIn0;
		}
	}
};

#endif
