///////////////////////////////////////////////////////////
// BasicSynth filter classes #1
// This file includes classes for:
//  FilterFIR - 1st order (one-zero) FIR filter
//  FilterIIR - 1st order (one-pole) IIR filter
//  FilterIIR2 - one-pole, two-zero filter
//  FilterFIRn - n-order FIR filter using convolution
//  FilterAVGn - n-delay running average filter
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////
#ifndef _FILTER_H_
#define _FILTER_H_


///////////////////////////////////////////////////////////
// One-zero filter: y[n] = b * x[n] + a * x[n-1]
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

	// ampin, ampdly
	void Init(int n, float *v)
	{
		if (n > 1)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]));
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
// One-pole filter: y[n] = b * x[n] - a * y[n-1]
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

	// inAmp dlyAmp
	void Init(int n, float *v)
	{
		if (n > 1)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]));
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

	AmpValue Sample(AmpValue val)
	{
		return delay = (val * inAmp) - (delay * dlyAmp);
	}
};

///////////////////////////////////////////////////////////
// One-pole, two-zero filter: y[n] = a0 * x[n] + a1 * x[n-1] + b1 * y[n-1]
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

	// inAmp dlyAmp
	void Init(int n, float *v)
	{
		if (n > 2)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]), AmpValue(v[2]));
	}

	void Reset(float initPhs = 0)
	{
		delayX = 0;
		delayY = 0;
	}

	void InitFilter(AmpValue in0, AmpValue in1, AmpValue out)
	{
		inAmp0 = in0;
		inAmp1 = in1;
		dlyAmp = out;
	}

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

	AmpValue Sample(AmpValue val)
	{
		delayY = (val * inAmp0) + (inAmp1 * delayX) + (delayY * dlyAmp);
		delayX = val;
		return delayY;

	}
};

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

	// Initialization:
	//  n -> number of impulses
	//  v -> array of impluse values
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

	void SetCoef(float *v)
	{
		for (int i = 0; i < length; i++)
			imp[i] = AmpValue(v[i]);
	}
	
	// calculate coefficients for a LP/HP filter
	// using windowed sinc (Hamming window)
	void CalcCoef(FrqValue fc, int hp = 0)
	{
		if (!(length & 1))
			return;
		int n2 = length/2;
		int ndx1 = n2 + 1;
		int ndx2 = n2 - 1;
		double g = 1.0;
		int k;
		/**** Direct calculation ****
		double w1 = fc * synthParams.frqRad;
		double w2 = 1.0;
		double w3 = twoPI / (double) (length-1);
		double phs1 = w1;
		double phs2 = w2;
		double phs3 = w3;
		double v;
		for (k = 0; k < n2; k++)
		{
			v = (sin(phs1) / phs2) * (0.54 + (0.46 * cos(phs3)));
			g += v + v;
			imp[ndx1++] = AmpValue(v);
			imp[ndx2--] = AmpValue(v);
			phs1 += w1;
			phs2 += w2;
			phs3 += w3;
		}
		***************/
		/***** Table lookup *********/
		PhsAccum ti1 = fc * synthParams.frqTI;
		PhsAccum ti2 = synthParams.ftableLength / (PhsAccum) (length - 1);
		PhsAccum tph1 = ti1;
		PhsAccum tph2 = ti2 + (synthParams.ftableLength / 4);
		AmpValue divInc = 1.0; // or PI
		AmpValue div = divInc;
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
		/*************************/
		imp[n2] = 1.0;

		// normalize filter gain for unity at DC
		// and optionally convert to high-pass
		for (k = 0; k < length; k++)
		{
			imp[k] /= g;
			if (hp && (k & 1))
				imp[k] = -imp[k];
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
		int n;
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

	void  Init(int n, float *v)
	{
		if (n > 0)
			InitFilter((int)v[0]);
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
