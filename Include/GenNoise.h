///////////////////////////////////////////////////////////////
//
// BasicSynth Noise generators
//
// GenNoise - white noise generator
// GenNoiseH - sampled white noise
// GenNoiseI - sampled/interpolated white noise
// GenPink1 - pink-ish noise generator using FIR comb filter
// GenPink2 - pink-ish noise generator using IIR comb filter
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _GENNOISE_H_
#define _GENNOISE_H_

// White Noise
class GenNoise : public GenUnit
{
public:
	virtual void Init(int n, float *v) { }
	virtual void Reset(float initPhs = 0) { }

	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	virtual AmpValue Gen()
	{
		return ((AmpValue) rand() - (RAND_MAX/2)) / (RAND_MAX/2);
	}

};

class GenNoiseH : public GenNoise
{
private:
	AmpValue lastVal;
	FrqValue freq;
	bsInt32 count;
	bsInt32 hcount;
public:
	GenNoiseH()
	{
		freq = synthParams.sampleRate;
		count = 0;
		hcount = 1;
		lastVal = 0;
	}

	virtual void Init(int n, float *v)
	{
		if (n > 0)
			InitH(*v);
	}

	void InitH(FrqValue f)
	{
		if ((freq = f) <= 0)
			freq = 1;
		Reset(0);
	}

	virtual void Reset(float initPhs = 0)
	{
		hcount = (bsInt32) (synthParams.sampleRate / freq);
		count = 0;
	}

	virtual AmpValue Gen()
	{
		if (--count <= 0)
		{
			count = hcount;
			lastVal = GenNoise::Gen();
		}
		return lastVal;
	}

};

class GenNoiseI : public GenNoise
{
private:
	AmpValue lastVal;
	AmpValue nextVal;
	AmpValue incrVal;
	FrqValue freq;
	bsInt32 count;
	bsInt32 hcount;
public:
	GenNoiseI()
	{
		freq = synthParams.sampleRate;
		hcount = 1;
		count = 0;
		lastVal = 0;
		nextVal = 0;
		incrVal = 0;
	}

	virtual void Init(int n, float *v)
	{
		if (n > 0)
			InitH(FrqValue(*v));
	}

	void InitH(FrqValue f)
	{
		if ((freq = f) <= 0)
			freq = 1;
		Reset(0);
	}

	virtual void Reset(float initPhs = 0)
	{
		hcount = (bsInt32) (synthParams.sampleRate / freq);
		if (hcount < 1)
			hcount = 1;
		count = 0;
		lastVal = GenNoise::Gen();
		nextVal = GenNoise::Gen();
		incrVal = (nextVal - lastVal) / (AmpValue) hcount;
	}

	virtual AmpValue Gen()
	{
		if (--count <= 0)
		{
			count = hcount;
			lastVal = nextVal;
			nextVal = GenNoise::Gen();
			incrVal = (nextVal - lastVal) / (AmpValue) count;
		}
		else
			lastVal += incrVal;
		return lastVal;
	}

};

// "Pink-ish" noise generators - first order LP filter on White noise output
// choose between FIR (Pink1) and IIR (Pink2)
class GenNoisePink1 : public GenNoise
{
private:
	AmpValue prev;
public:
	GenNoisePink1()
	{
		prev = 0;
	}

	virtual AmpValue Gen()
	{
		AmpValue val = GenNoise::Gen();
		AmpValue out = (val + prev) / 2;
		prev = val;
		return out;
	}
};

class GenNoisePink2 : public GenNoise
{
private:
	AmpValue prev;
public:
	GenNoisePink2()
	{
		prev = 0;
	}

	virtual AmpValue Gen()
	{
		//AmpValue val = GenWaveNoise::Gen();
		//AmpValue out = (val + prev) / 2;
		//prev = out;
		//return out;
		return prev = (GenNoise::Gen() + prev) / 2;
	}
};

#endif

