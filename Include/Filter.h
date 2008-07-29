///////////////////////////////////////////////////////////
// Filter.h
//
// BasicSynth filter classes #1
// This file includes classes for:
//  FilterFIR - 1st order (one-zero) FIR filter
//  FilterIIR - 1st order (one-pole) IIR filter
//  FilterFIRn - n-order FIR filter using convolution
//  FilterAVGn - n-delay running average filter
///////////////////////////////////////////////////////////
#ifndef _FILTER_H_
#define _FILTER_H_


///////////////////////////////////////////////////////////
// One-zero filter: y[n] = a * x[n] + b * x[n-1]
///////////////////////////////////////////////////////////
class FilterFIR : public GenUnit
{
protected:
	AmpValue delay;  // one sample delay
	AmpValue inAmp;  // input coefficient (a)
	AmpValue dlyAmp; // delay coefficient (b)
public:
	FilterFIR()
	{
		delay = 0;
		inAmp = 0;
		dlyAmp = 0;
	}

	void Init(int n, float *f)
	{
		if (n > 1)
			InitFilter(f[0], f[1]);
	}

	void Reset(float initPhs = 0)
	{
		delay = 0;
	}

	void InitFilter(AmpValue in, AmpValue out)
	{
		inAmp = in;
		dlyAmp = out;
	}

	AmpValue Sample(AmpValue val)
	{
		AmpValue out = (val * inAmp) + (delay * dlyAmp);
		delay = val;
		return out;
	}
};

///////////////////////////////////////////////////////////
// One-pole filter: y[n] = a * x[n] + b * y[n-1]
///////////////////////////////////////////////////////////
class FilterIIR : public GenUnit
{
protected:
	AmpValue delay;  // one sample delay
	AmpValue inAmp;  // input coefficient (a)
	AmpValue dlyAmp; // delay coefficient (b)
public:
	FilterIIR()
	{
		delay = 0;
		inAmp = 0;
		dlyAmp = 0;
	}

	// inAmp dlyAmp
	void Init(int n, float *f)
	{
		if (n > 1)
			InitFilter(f[0], f[1]);
	}

	void Reset(float initPhs = 0)
	{
		delay = 0;
	}

	void InitFilter(AmpValue in, AmpValue out)
	{
		inAmp = in;
		dlyAmp = out;
	}

	AmpValue Sample(AmpValue val)
	{
		return delay = (val * inAmp) + (delay * dlyAmp);
	}
};

//
///////////////////////////////////////////////////////////
// FIR impulse response filter:
//   y[n] = h[0] * x[0] + h[1] * x[n-1] ... + h[m] * x[n-m]
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

	// Initialization:
	//  n -> number of impulses
	//  h -> array of impluse values
	void Init(int n, float *h)
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
			val = new AmpValue[n];
			imp = new AmpValue[n];
			for (int i = 0; i < n; i++)
			{
				val[i] = 0;
				imp[i] = (AmpValue) h[i];
			}
		}
	}

	// Reset: Reset history values to 0
	void Reset(float initPhs = 0)
	{
		for (int n = 0; n < length; n++)
			val[n] = 0;
	}

	// Return the next sample,
	// convolution of input with impulse
	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = imp[0] * inval;
		/*
		for (n = length-1; n > 0; n--)
		{
			val[n] = val[n-1];
			out += imp[n] * val[n];
		}
		val[0] = inval;
		*/
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
	}
};

///////////////////////////////////////////////////////////
// Running average filter:
//     y[n] = (x[n] + x[n-1] ... + x[n-M]) / M
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

	void  Init(int n, float *f)
	{
		if (n > 0)
			InitFilter((int)f[0]);
	}

	void Reset(float initPhs = 0)
	{
		if (length > 0)
		{
			AmpValue *p = &prev[length];
			while (--p >= prev)
				*p = 0;
		}
	}

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


#endif
