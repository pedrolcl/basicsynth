///////////////////////////////////////////////////////////////
//
// BasicSynth - DelayLine
//
// Various forms of delay lines
//
// 1 - DelayLine, basic delay line with attenuation of the output
// 2 - DelayLineR, recirculating delay line (resonator)
// 3 - DelayLineV, variable delay
// 4 - AllPassDelay, delay line with all pass decay value
// 5 - DelayLineT, multi-tap delay line
//
// Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _DELAYLINE_H_
#define _DELAYLINE_H_

// Basic delay line
class DelayLine : public GenUnit
{
protected:
	AmpValue *delayBuf;    // buffer to store samples
	AmpValue decayFactor;  // output attenuation
	AmpValue *delayPos;    // current in/out position
	AmpValue *delayEnd;    // last writable position
	FrqValue delayTime;    // length of delayBuf in secs.
	int delayLen;          // length of delayBuf in samps.

	virtual void ReleaseBuffers()
	{
		if (delayBuf)
		{
			delete delayBuf;
			delayBuf = NULL;
			delayPos = NULL;
			delayEnd = NULL;
			delayLen = 0;
		}
	}

public:
	DelayLine()
	{
		delayLen = 0;
		delayBuf = NULL;
		delayPos = NULL;
		delayEnd = NULL;
		decayFactor = 1;
	}

	virtual ~DelayLine()
	{
		ReleaseBuffers();
	}

	virtual void Copy(DelayLine *dp)
	{
		InitDL(dp->delayTime, dp->decayFactor);
	}

	void GetSettings(FrqValue& dly, FrqValue& dec)
	{
		dly = delayTime;
		dec = decayFactor;
	}

	virtual void Init(int n, float *p)
	{
		if (n > 1)
			InitDL(p[0], p[1]);
	}

	virtual void Reset(float initPhs = 0)
	{
		delayPos = delayBuf;
	}

	void Clear()
	{
		delayPos = delayBuf;
		while (delayPos < delayEnd)
			*delayPos++ = 0;
		delayPos = delayBuf;
	}

	virtual void InitDL(FrqValue dlyTm, AmpValue decay = 1)
	{
		ReleaseBuffers();
		delayTime = dlyTm;
		decayFactor = decay;
		delayLen = (int) (delayTime * synthParams.sampleRate);
		if (delayLen <= 0)
			delayLen = 1;
		delayBuf = new AmpValue[delayLen];
		delayEnd = delayBuf + delayLen;
		Clear();
	}

	AmpValue TapT(PhsAccum d)
	{
		return Tap(d * synthParams.sampleRate);
	}

	AmpValue Tap(PhsAccum s)
	{
		AmpValue *tp = delayPos - (int) s;
		if (tp < delayBuf)
			tp += delayLen;
		return *tp;
	}

	inline AmpValue GetOut()
	{
		return *delayPos * decayFactor;
	}

	inline void SetIn(AmpValue inval)
	{
		*delayPos = inval;
		if (++delayPos >= delayEnd)
			delayPos = delayBuf;
	}

	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue out = GetOut();
		SetIn(in);
		return out;
	}
};

// Re-circulating delay line, a/k/a IIR comb filter, resonator
class DelayLineR : public DelayLine
{
private:
	AmpValue final;
	AmpValue peak;
	FrqValue decayTime;

public:
	DelayLineR()
	{
		final = 0.001;
		peak = 1.0;
		decayTime = 1.0;
	}

	virtual void Copy(DelayLineR *dp)
	{
		InitDLR(dp->delayTime, dp->decayTime, dp->final, dp->peak);
	}

	void GetSettings(FrqValue& dlyTm, FrqValue& decTm, AmpValue& fin, AmpValue& pk)
	{
		dlyTm = delayTime;
		decTm = decayTime;
		fin = final;
		pk = peak;
	}

	// This method calculates an exponential decay based on delayTime and
	// final amplitude level after decayTime. Use InitDL directly if the
	// caller calculates decayFactor by some other means.
	void InitDLR(FrqValue dlyTm, FrqValue decTm, AmpValue fin, AmpValue pk = 1.0)
	{
		final = fin;
		peak = pk;
		decayTime = decTm;
		DelayLine::InitDL(dlyTm, pow(peak * final, dlyTm/decTm));
	}

	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = GetOut();
		SetIn(inval + out);
		return out;
	}
};

// variable delay tap delay line
class DelayLineV : public DelayLine
{
protected:
	PhsAccum dlyTime;
	PhsAccum rdFract;
	int      rdInt;

public:
	DelayLineV()
	{
		dlyTime = 0;
		rdFract = 0;
		rdInt = 0;
	}

	// set delay in seconds
	void SetDelayT(PhsAccum d)
	{
		SetDelay(d * synthParams.sampleRate);
	}

	// set variable delay in samples
	void SetDelay(PhsAccum d)
	{
		dlyTime = d;
#ifdef _DEBUG
		if (dlyTime < 0)
			dlyTime = 0; // sorry, can't predict the future!
		else if (dlyTime >= delayLen)
			dlyTime = delayLen-1;
#endif
		rdInt = (int) dlyTime;
		rdFract = dlyTime - (PhsAccum) rdInt;
	}

	AmpValue Sample(AmpValue inval)
	{
		AmpValue *rdPos = delayPos - rdInt;
		if (rdPos < delayBuf)
			rdPos += delayLen;
		if (rdInt == 0)
			SetIn(inval);
		// *delayPos now contains the current sample if dlyTime < 1 sample
		// or the maximum delay sample otherwise.
		// assert: 
		//         0 <= delayOffs <= delayLen
		//         0 < dlyTime < 1 will combine previous and current samples
		//         dlyTime = 0 will return current sample
		//         delayLen-1 <= dlyTime < delayLen will combine maxdelay sample with maxdelay-1
		//         dlyTime = delayLen will return maxdelay
		//         all other cases return interpolation of rdPos[0] and rdPos[-1]
		AmpValue out = *rdPos;
		if (--rdPos < delayBuf)
			rdPos += delayLen;
		out += (*rdPos - out) * rdFract;
		if (rdInt > 0) // safe to overwrite maxdelay sample
			SetIn(inval);
		return out * decayFactor;
	}
};

class AllPassDelay : public DelayLineR
{
public:
	AmpValue Sample(AmpValue inval)
	{
		AmpValue vm = *delayPos;
		AmpValue vn = inval - (vm * decayFactor);
		SetIn(vn);
		return vm + (vn * decayFactor);
	}
};

// direct-form I, uses two separate delay lines
class AllPassDelay2 : public GenUnit
{
private:
	DelayLine dlx;
	DelayLine dly;
	AmpValue apg;

public:
	AllPassDelay2()
	{
		apg = 0;
	}

	void Init(int n, float *f)
	{
		if (n > 1)
			InitDL(f[0], f[1]);
	}

	void Reset(float initPhs = 0)
	{
		dlx.Reset(initPhs);
		dly.Reset(initPhs);
	}

	void InitDL(FrqValue delayTime, AmpValue decay)
	{
		apg = decay;
		dlx.InitDL(delayTime, 1);
		dly.InitDL(delayTime, apg);
	}

	void InitDLR(FrqValue delayTime, FrqValue decayTime, float atten, float peak = 1.0)
	{
		InitDL(delayTime, pow(peak * atten, delayTime/decayTime));
	}

	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = (inval * apg) + dlx.GetOut() - dly.GetOut();
		dly.SetIn(out);
		dlx.SetIn(inval);
		return out;
	}
};

// Multi-tap delay line
class DelayLineT : public DelayLine
{
protected:
	int numTaps;          // number of delay taps
	AmpValue **delayTaps; // position of each tap;
	AmpValue *decayTaps;  // decay value for each tap

public:
	DelayLineT()
	{
		numTaps = 0;
		delayTaps = NULL;
		decayTaps = NULL;
	}

	virtual ~DelayLineT()
	{
		ReleaseBuffers();
	}

	virtual void ReleaseBuffers()
	{
		DelayLine::ReleaseBuffers();
		if (delayTaps)
		{
			delete delayTaps;
			delete decayTaps;
			delayTaps = NULL;
			decayTaps = NULL;
		}
		numTaps = 0;
	}

	void Init(int n, float *p)
	{
		if (n > 0)
		{
			int taps = 0;
			if (n > 1)
				taps = (int) p[1];
			InitDLT(p[0], taps);
			int i, t;
			for (i = 2, t = 0; i < n && t < taps; i++, t++)
				SetTap(t, p[i]);
		}
	}

	void InitDLT(FrqValue dlyTm, int taps, AmpValue decay = 1)
	{
		DelayLine::InitDL(dlyTm, decay);

		numTaps = taps;
		if (taps > 0)
		{
			delayTaps = new AmpValue*[taps];
			decayTaps = new AmpValue[taps];
			for (int n = 0; n < taps; n++)
			{
				delayTaps[n] = delayBuf;
				decayTaps[n] = 1.0;
			}
		}
	}

	void SetTap(int n, AmpValue dlyTm, AmpValue decay = 1)
	{
		long position = (int) (dlyTm * synthParams.sampleRate);
		if (position < delayLen)
		{
			delayTaps[n] = delayBuf + (delayLen - position);
			decayTaps[n] = decay;
		}
	}

	AmpValue Tap(int n)
	{
		if (n < numTaps)
			return *(delayTaps[n]) * decayTaps[n];
		return 0;
	}

	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = DelayLine::Sample(inval);
		for (int  n = 0; n < numTaps; n++)
		{
			if (++(delayTaps[n]) >= delayEnd)
				delayTaps[n] = delayBuf;
		}
		return out;
	}
};


#endif
