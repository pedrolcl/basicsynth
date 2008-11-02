///////////////////////////////////////////////////////////
// BasicSynth filter classes #1
//
/// @file Filter.h FIR and IIR filter classes.
//
// This file includes classes for:
//  FilterFIR - 1st order (one-zero) FIR filter
//  FilterIIR - 1st order (one-pole) IIR filter
//  FilterIIR2 - one-pole, two-zero filter
//  FilterFIRn - n-order FIR filter using convolution
//  FilterAVGn - n-delay running average filter
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////
// @addtogroup grpFilter
//@{
#ifndef _FILTER_H_
#define _FILTER_H_


///////////////////////////////////////////////////////////
/// FIR, one-zero filter. This filter implements the equation:
/// @code
/// y[n] = b * x[n] + a * x[n-1]
/// @endcode
///////////////////////////////////////////////////////////
class FilterFIR : public GenUnit
{
protected:
	AmpValue delay;  // one sample delay
	AmpValue inAmp;  // input coefficient (b)
	AmpValue dlyAmp; // delay coefficient (a)
public:
	FilterFIR()
	{
		delay = 0;
		inAmp = 0;
		dlyAmp = 0;
	}

	/// Initialize the filter. Set the two coefficients from the 
	/// value array.
	/// @param n number of values (2)
	/// @param v array of values, v[0] = b, v[1] = a
	void Init(int n, float *v)
	{
		if (n > 1)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]));
	}

	/// Reset the filter. This merely clears the delay buffer. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		delay = 0;
	}

	/// Initialize the filter. The two arguments are the coefficients. 
	/// @param in input sample coefficient (a)
	/// @param out delayed sample coefficient (b)
	void InitFilter(AmpValue in, AmpValue out)
	{
		inAmp = in;
		dlyAmp = out;
	}

	/// Process the current sample. The input sample is stored in the delay
	/// buffer and the filtered sample is calculated and returned.
	/// @param val current sample
	AmpValue Sample(AmpValue val)
	{
		AmpValue out = (val * inAmp) + (delay * dlyAmp);
		delay = val;
		return out;
	}
};

///////////////////////////////////////////////////////////
/// IIR, one-zero filter. This filter implements the equation:
/// @code
/// y[n] = b * x[n] - a * y[n-1]
/// @endcode
///////////////////////////////////////////////////////////
class FilterIIR : public GenUnit
{
protected:
	AmpValue delay;  // one sample delay
	AmpValue inAmp;  // input coefficient (b)
	AmpValue dlyAmp; // delay coefficient (a)
public:
	FilterIIR()
	{
		delay = 0;
		inAmp = 0;
		dlyAmp = 0;
	}

	/// Initialize the filter. Set the two coefficients from the 
	/// value array.
	/// @param n number of values (2)
	/// @param v array of values, v[0] = b, v[1] = a
	void Init(int n, float *v)
	{
		if (n > 1)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]));
	}

	/// Reset the filter. This merely clears the delay buffer. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		delay = 0;
	}

	/// Initialize the filter. The two arguments are the coefficients 
	/// @param in input sample coefficient (a)
	/// @param out delayed sample coefficient (b)
	void InitFilter(AmpValue in, AmpValue out)
	{
		inAmp = in;
		dlyAmp = out;
	}

	/// Calculate coefficients. The coefficients are calculate to produce the indicated
	/// cutoff frequency for either a low-pass or high-pass frequency response.
	/// @param fc cutoff frequency
	/// @param hp when true, produce a high-passs
	void CalcCoef(FrqValue fc, int hp = 0)
	{
		double x = exp(-twoPI * (fc/synthParams.sampleRate));
		if (hp)
		{
			inAmp = x;
			dlyAmp = AmpValue(1.0 - x);
		}
		else
		{
			inAmp = AmpValue(1.0 - x);
			dlyAmp = AmpValue(-x);
		}
	}

	/// Process the current sample. The input sample is stored in the delay
	/// buffer and the filtered sample is calculated and returned.
	/// @param val current sample value
	AmpValue Sample(AmpValue val)
	{
		return delay = (val * inAmp) - (delay * dlyAmp);
	}
};

///////////////////////////////////////////////////////////
/// One-pole, two-zero filter. This filter implements the equation
/// @code
/// y[n] = a0 * x[n] + a1 * x[n-1] + b1 * y[n-1]
/// @endcode
///////////////////////////////////////////////////////////
class FilterIIR2 : public GenUnit
{
protected:
	AmpValue delayY;  // one sample delay
	AmpValue delayX;
	AmpValue inAmp0; // a0
	AmpValue inAmp1; // a1
	AmpValue dlyAmp; // b1
public:
	FilterIIR2()
	{
		delayX = 0;
		delayY = 0;
		inAmp0 = 0;
		inAmp1 = 0;
		dlyAmp = 0;
	}

	/// Initialize the filter. Set the three coefficients from the 
	/// value array.
	/// @param n number of values (3)
	/// @param v values, v[0] = a0, v[1] = a1, v[2] = b
	void Init(int n, float *v)
	{
		if (n > 2)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]), AmpValue(v[2]));
	}

	/// Reset the filter. This merely clears the delay buffer. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		delayX = 0;
		delayY = 0;
	}

	/// Initialize the filter. The three arguments are the coefficients 
	/// @param in0 input sample coefficient (a0)
	/// @param in1 input sample coefficient (a1)
	/// @param out delayed sample coefficient (b)
	void InitFilter(AmpValue in0, AmpValue in1, AmpValue out)
	{
		inAmp0 = in0;
		inAmp1 = in1;
		dlyAmp = out;
	}

	/// Calculate coefficients. The coefficients are calculate to produce the indicated
	/// cutoff frequency for either a low-pass or high-pass frequency response.
	/// @param fc cutoff frequency
	/// @param hp when true, produce a high-passs
	void CalcCoef(FrqValue fc, int hp = 0)
	{
		double x = exp(-twoPI * (fc/synthParams.sampleRate));
		if (hp)
		{
			inAmp0 = AmpValue((1.0 + x) / 2.0);
			inAmp1 = -inAmp0;
		}
		else
		{
			inAmp0 = 1.0 - x;
			inAmp1 = 0.0;
		}
		dlyAmp = AmpValue(x);
	}

	/// Process the current sample. The input sample is stored in the delay
	/// buffer and the filtered sample is calculated and returned.
	/// @param val current sample value
	AmpValue Sample(AmpValue val)
	{
		delayY = (val * inAmp0) + (inAmp1 * delayX) + (delayY * dlyAmp);
		delayX = val;
		return delayY;

	}
};

///////////////////////////////////////////////////////////
/// FIR impulse response filter. This filter implements
/// convolution of the input with an impulse response:
/// @code
/// y[n] = h[0] * x[0] + h[1] * x[n-1] ... + h[m] * x[n-m]
/// @endcode
///////////////////////////////////////////////////////////
class FilterFIRn : public GenUnit
{
protected:
	AmpValue *val; // input sample values
	AmpValue *imp; // impulse response
	int length;
public:
	FilterFIRn()
	{
		val = NULL;
		imp = NULL;
		length = 0;
	}

	~FilterFIRn()
	{
		delete val;
		delete imp;
	}

	/// Allocate impulse response. The impulse responce array holds the coefficients
	/// for convolution. This array is allocated automatically when Init is called.
	/// If coefficients are to be set individulally using SetCoef() this function
	/// must be called first to create the buffer.
	/// @param n number of coefficients
	void AllocImpResp(int n)
	{
		if (val)
		{
			delete val;
			val = NULL;
		}
		if (imp)
		{
			delete imp;
			imp = NULL;
		}

		if ((length = n) > 0)
		{
			val = new AmpValue[length];
			imp = new AmpValue[length];
		}
	}

	/// Initialize the filter. The first member of the value array
	/// contains the number of impulses. The remaining values
	/// contain the amplitudes for each impulse.
	/// @param n number of values
	/// @param v values, v[0] = number of coefficients (n), v[1..n] = array of coefficient values
	void Init(int n, float *v)
	{
		AllocImpResp(n);
		for (int i = 0; i < n; i++)
		{
			val[i] = 0;
			if (v)
				imp[i] = AmpValue(v[i]);
			else
				imp[i] = 0;
		}
	}

	/// Set the impulse coefficients. The array must be of the same
	/// length set with AllocImpResp()
	/// @param v impulse response
	void SetCoef(float *v)
	{
		for (int i = 0; i < length; i++)
			imp[i] = AmpValue(v[i]);
	}
	
	/// Calculate coefficients. The impulse responce coefficients are calculated
	/// for a low-pass or high-pass filter using the windowed sinc equation with
	/// a Hamming window.
	/// @param fc cutoff frequency
	/// @param hp 0 = low-pass, 1 = high-pass
	void CalcCoef(FrqValue fc, int hp = 0)
	{
		if (!(length & 1))
			return;
		PhsAccum ti1 = fc * synthParams.frqTI;
		PhsAccum ti2 = synthParams.ftableLength / (PhsAccum) (length - 1);
		PhsAccum tph1 = ti1;
		PhsAccum tph2 = ti2 + (synthParams.ftableLength / 4);
		AmpValue divInc = PI;
		AmpValue div = divInc;
		int n2 = length/2;
		int ndx1 = n2 + 1;
		int ndx2 = n2 - 1;
		double g = 2 * (fc / synthParams.sampleRate);
		imp[n2] = g;
		int k;
		AmpValue v;
		for (k = 0; k < n2; k++)
		{
			v = (wtSet.wavSin[(int)(tph1+0.5)] / div) * (0.54 + (0.46 * wtSet.wavSin[(int)(tph2+0.5)]));
			g += v + v;
			imp[ndx1++] = v;
			imp[ndx2--] = v;
			if ((tph1 += ti1) >= synthParams.ftableLength)
				tph1 -= synthParams.ftableLength;
			if ((tph2 += ti2) >= synthParams.ftableLength)
				tph2 -= synthParams.ftableLength;
			div += divInc;
		}
		// normalize filter gain for unity at DC
		// and optionally convert to high-pass
		for (k = 0; k < length; k++)
		{
			imp[k] /= g;
			if (hp)
				imp[k] = -imp[k];
		}
		if (hp)
			imp[n2] += 1.0;

		/**** Direct calculation (for reference) ****
		int z = (length - 1) / 2;
		double m = (double) length - 1;
		double f = fc / synthParams.sampleRate;
		for (k = 0; k < length; k++)
		{
			double n = (double)k - (m / 2);
			if (n == 0)
				imp[k] = 2.0 * f;
			else
				imp[k] = sin(twoPI*f*n) / (PI*n);
			imp[k] *= 0.54 + (0.46 * cos(twoPI * n / m));
		}
		**********************************************/
	}

	/// Reset the filter. This clears the history values to 0. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		AmpValue *v = val;
		for (int n = length; n > 0; n--)
			*v++ = 0;
	}

	/// Process the current sample. The current sample is pushed into the
	/// history buffer and then convolved with the impulse response.
	/// @param inval current sample value.
	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = imp[0] * inval;
		int m = length-1;
		AmpValue *vp = &val[m];
		AmpValue *ip = &imp[m];
		if (m > 0)
		{
			AmpValue tmp;
			do
			{
				tmp = *(vp-1);
				*vp-- = tmp;
				out += *ip-- * tmp;
			} while (vp > val);
		}
		*vp = inval;
		return out;
		/***** indexing (for ref) *****
		for (n = length-1; n > 0; n--)
		{
			val[n] = val[n-1];
			out += imp[n] * val[n];
		}
		val[0] = inval;
		*******************************/
	}
};

///////////////////////////////////////////////////////////
/// Running average filter. This filter sums a series of
/// samples using the equation:
/// @code
/// y[n] = (x[n] + x[n-1] ... + x[n-M]) / M
/// @endcode
///////////////////////////////////////////////////////////
class FilterAvgN : public GenUnit
{
protected:
	AmpValue *prev;
	int length;
public:
	FilterAvgN()
	{
		prev = NULL;
		length = 0;
	}

	~FilterAvgN()
	{
		delete prev;
	}

	/// Initialize the filter. The first value in the array is the number of samples to average.
	/// @param n number of values (1)
	/// @param v values, v[0] = number of samples to average
	void  Init(int n, float *v)
	{
		if (n > 0)
			InitFilter((int)v[0]);
	}

	/// Reset the filter. This clears the previous values. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		if (length > 0)
		{
			AmpValue *p = &prev[length];
			while (--p >= prev)
				*p = 0;
		}
	}

	/// Initialize the filter. A history buffer is allocated to the indicated length
	/// and set to zero. 
	/// @param n number of samples to average.
	void InitFilter(int n)
	{
		if (prev)
		{
			delete prev;
			prev = NULL;
		}
		if ((length = n) < 2)
			length = 2;
		prev = new AmpValue[length];
		Reset();
	}

	/// Process the current sample. The sample is added to the history buffer
	/// and then the average is returned.
	/// @param inval current sample value
	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = inval;
		AmpValue tmp;
		AmpValue *p = &prev[length-1];
		do
		{
			tmp = *(p-1);
			*p-- = tmp;
			out += tmp;
		} while (p > prev);
		*p = inval;

		return out / (AmpValue) length;
	}
};
//@}

#endif
