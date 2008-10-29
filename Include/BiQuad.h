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
/// \addtogroup grpFilter
/*@{*/
#ifndef _BIQUAD_H_
#define _BIQUAD_H_

#define sqr2 1.414213562

///////////////////////////////////////////////////////////
/// Bi-quad filter base class. This class defines the 
/// coefficient and sample history buffer members but
/// does not implement a filter.
///////////////////////////////////////////////////////////
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

	/// Initialize with a copy. Initialize the filter coefficients from
	/// the object filt.
	/// \param filt filter to copy from
	virtual void Copy(BiQuadFilter *filt)
	{
		cutoff = filt->cutoff;
		gain = filt->gain;
		ampIn0 = filt->ampIn0;
		ampIn1 = filt->ampIn1;
		ampIn2 = filt->ampIn2;
		ampOut1 = filt->ampOut1;
		ampOut2 = filt->ampOut2;
		dlyIn1 = filt->dlyIn1;
		dlyIn2 = filt->dlyIn2;
		dlyOut1 = filt->dlyOut1;
		dlyOut2 = filt->dlyOut2;
	}

	/// Initialize the filter. The input array holds the cutoff frequency and gain.
	/// \param n number of values (1 or 2)
	/// \param v values: v[0] = cutoff, v[1] = gain
	virtual void  Init(int n, float *v)
	{
		if (n > 1)
			Init((FrqValue) v[0], (AmpValue) v[1]);
		else if (n > 0)
			Init((FrqValue) v[0], 1);
	}

	/// Initialize cutoff frequency and gain.
	/// \param cu cutoff frequency
	/// \param g overall filter gain
	virtual void Init(FrqValue cu, AmpValue g)
	{
		cutoff = cu;
		gain = g;
	}

	/// Reset the filter. The history buffer is cleared to zero. The phase argument is ignored.
	/// \param initPhs (not used)
	virtual void Reset(float initPhs)
	{
		dlyIn1 = 0;
		dlyIn2 = 0;
		dlyOut1 = 0;
		dlyOut2 = 0;
	}

	/// Process the current sample. The output sample is calculated from
	/// the coefficients and delayed samples.
	/// \param vin current sample amplitude
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

///////////////////////////////////////////////////////////
/// Low-pass filter. This class extends BiQuadFilter adding
/// code to calculate the coefficients for a low-pass filter.
///////////////////////////////////////////////////////////
class FilterLP : public BiQuadFilter
{
public:
	/// Initialize the cutoff frequency and gain. This method calculates the
	/// coefficients if the new frequency does not match the current frequency.
	/// \param cu cutoff frequency
	/// \param g overall filter gain
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

///////////////////////////////////////////////////////////
/// High-pass filter. This class extends BiQuadFilter adding 
/// code to calculate the coefficients for a high-pass filter.
///////////////////////////////////////////////////////////
class FilterHP : public BiQuadFilter
{
public:
	/// Initialize the cutoff frequency and gain. This method calculates the
	/// coefficients if the new frequency does not match the current frequency.
	/// \param cu cutoff frequency
	/// \param g overall filter gain
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

///////////////////////////////////////////////////////////
/// Band-pass filter. This class extends BiQuadFilter adding
/// code to calculate the coefficients for a band-pass filter.
///////////////////////////////////////////////////////////
class FilterBP : public BiQuadFilter
{
private:
	float bw;
public:
	/// Initialize the filter. Sets the cutoff frequency and gain. Bandwidth defaults
	/// to 1K.
	/// \param cu cutoff frequency
	/// \param g overall filter gain
	virtual void Init(FrqValue cu, AmpValue g)
	{
		Init(cu, g, 1000.0);
	}

	/// Initialize cutoff frequency, gain and bandwidth. The coefficients are calculated
	/// for a band-pass filter with a center frequency and bandwidth.
	/// \param cu cutoff frequency
	/// \param g filter gain
	/// \param B bandwidth
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

///////////////////////////////////////////////////////////
/// Constant gain Resonantor. This class extends BiQuadFilter
/// adding code to calculate the coefficients for a resonant band-pass filter.
/// See J. Smith, "Introduction to Digital Filters", Appendix B
/// and also Perry Cook, "Real Sound Synthesis", Chapter 3
///////////////////////////////////////////////////////////
class Reson : public BiQuadFilter
{
private:
	float res;

public:
	Reson()
	{
		res = 1.0;
	}

	/// Initialize the filter. Sets the cutoff frequency and gain. Resonance defaults
	/// to 1.
	/// \param cu cutoff frequency
	/// \param g overall filter gain
	virtual void Init(FrqValue cu, AmpValue g)
	{
		InitRes(cu, g, 1);
	}

	/// Initialize the filter cutoff frequency, gain and resonance. The coefficients are calculated
	/// for a band-pass filter with a center frequency and resonance.
	/// \param cu cutoff frequency
	/// \param g filter gain
	/// \param r bandwidth
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

/*@}*/
#endif
