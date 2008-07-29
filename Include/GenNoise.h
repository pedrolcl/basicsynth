///////////////////////////////////////////////////////////////
//
// BasicSynth - GenWaveWT
//
// Noise generators
//
// GenWaveNoise - white noise generator
// GenWavePink1 - pink-ish noise generator using FIR comb filter
// GenWavePink2 - pink-ish noise generator using IIR comb filter
//
// Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _GENNOISE_H_
#define _GENNOISE_H_

// White Noise
class GenWaveNoise : public GenUnit
{
public:
	virtual void Init(int n, float *f) { }
	virtual void Reset(float initPhs = 0) { }

	virtual AmpValue Sample(AmpValue in)
	{
		return Gen();
	}

	virtual AmpValue Gen()
	{
		return ((AmpValue) rand() - (RAND_MAX/2)) / (RAND_MAX/2);
	}

};

// "Pink-ish" noise generators - first order LP filter on White noise output
// choose between FIR (Pink1) and IIR (Pink2)
class GenWavePink1 : public GenWaveNoise
{
private:
	AmpValue prev;
public:
	GenWavePink1()
	{
		prev = 0;
	}

	virtual AmpValue Gen()
	{
		AmpValue val = GenWaveNoise::Gen();
		AmpValue out = (val + prev) / 2;
		prev = val;
		return out;
	}
};

class GenWavePink2 : public GenWaveNoise
{
private:
	AmpValue prev;
public:
	GenWavePink2()
	{
		prev = 0;
	}

	virtual AmpValue Gen()
	{
		//AmpValue val = GenWaveNoise::Gen();
		//AmpValue out = (val + prev) / 2;
		//prev = out;
		//return out;
		return prev = (GenWaveNoise::Gen() + prev) / 2;
	}
};

#endif

