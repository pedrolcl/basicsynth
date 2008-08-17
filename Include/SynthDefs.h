///////////////////////////////////////////////////////////////
//
// BasicSynth - SynthDefs
//
// Global synthesizer definitions
//
// SynthConfig global parameters, and base class for Generators
//
// Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _SYNTHDEFS_H_
#define _SYNTHDEFS_H_

// This pragma gets rid of the irritating MSVC warning about 
// "double truncation to float" because the little 'f' is not 
// at the end of the constant. You might leave the warning on
// during initial code development.
#ifdef _MSC_VER
#pragma warning(disable : 4305)
#pragma warning(disable : 4244)
#endif


typedef short SampleValue; // sample output type
typedef char  bsInt8;    // select 8 bit data type
typedef short bsInt16;   // select 16 bit data type 
typedef int   bsInt32 ;  // select 32 bit data type 
typedef unsigned char  bsUint8;    // select 8 bit data type
typedef unsigned short bsUint16;   // select 16 bit data type 
typedef unsigned int   bsUint32 ;  // select 32 bit data type 
typedef void* Opaque;

// Choose one of these as the oscillator phase accumulator type.
// double gives less noise with slightly more calculation time
typedef double PhsAccum;
#define sinv sin
//typedef float PhsAccum;
//#define sinv sinf

typedef float AmpValue;
typedef float FrqValue;

#define PI 3.14159265358979
#define twoPI 6.28318530717958

#define PANTBLLEN 4096

class SynthConfig
{
public:
	FrqValue sampleRate;   // Global sample rate as frequency
	bsInt32 isampleRate;     // Sample rate as integer
	AmpValue sampleScale;  // multiplier to convert internal sample values into output values
	PhsAccum frqRad;       // pre-calculated multipler for frequency to radians twoPI/sampleRate
	PhsAccum frqTI;        // pre-calculated multipler for frequency to table index
	PhsAccum radTI;        // pre-calculated multipler for radians to table index
	PhsAccum ftableLength;    // wave table length
	bsInt32  itableLength;    // wave table length as integer, (slight optimization)
	FrqValue tuning[128];     // table to convert pitch class*octave into frequency
	AmpValue sinquad[PANTBLLEN]; // first quadrant of a sine wave
	AmpValue sqrttbl[PANTBLLEN]; // square roots of 0-1
	bsInt32  sqNdx;

	SynthConfig()
	{
		Init();

		size_t sampleBits = (sizeof(SampleValue) * 8) - 1; // -1 because a sample is a signed value.
		sampleScale = (AmpValue) ((1 << sampleBits) - 1);

		int i;
		// Equal tempered tuning system at A5=440 (Western standard)
		// Middle C = C5 = index 60 (MIDI numbering)
		double frq = 6.875 * pow(2.0, 0.25); // C0 = A(-1)*2^(3/12)
		double two12 = pow(2.0, 1.0/12.0); // 2^(1/12) = 1.059463094...
		for (i = 0; i < 128; i++)
		{
			tuning[i] = (FrqValue) frq;
			frq *= two12;
			//printf("%d = %f\n", i, tuning[i]);
		}

		// lookup tables for panning with max amplitude of 0.707...
		sqNdx = PANTBLLEN-1;
		double scl = sqrt(2.0) / 2.0;
		double phs = 0;
		double phsInc = (PI/2) / PANTBLLEN;
		double sqInc = 1.0 / PANTBLLEN;
		double sq = 0.0;
		for (i = 0; i < PANTBLLEN; i++)
		{
			sqrttbl[i] = sqrt(sq) * scl;
			sinquad[i] = sin(phs) * scl;
			phs += phsInc;
			sq += sqInc;
		}
	}

	void Init(bsInt32 sr = 44100, bsInt32 tl = 16384)
	{
		sampleRate = (FrqValue) sr;
		isampleRate = sr;
		itableLength = tl;
		ftableLength = (PhsAccum) tl;
		frqRad = twoPI / (PhsAccum) sampleRate;
		frqTI = ftableLength / (PhsAccum) sampleRate;
		radTI = ftableLength / twoPI;
	}

	FrqValue GetFrequency(int pitch)
	{
		return tuning[pitch & 0x7F];
	}
};

// this global must be defined somewhere in the main code
// the simplest way is to #include "SynthGlobal.cpp" and then
// call InitSynth
extern SynthConfig synthParams;
extern int InitSynthesizer(bsInt32 sampleRate = 44100, bsInt32 wtLen = 16384, bsInt32 wtUsr = 0);

class SampleBlock
{
public:
	int size;
	AmpValue *in;
	AmpValue *out;
	SampleBlock()
	{
		size = 0;
		in = NULL;
		out = NULL;
	}
};

// base class for oscillators and envelopes
class GenUnit
{
public:
	virtual void Init(int count, float *values) = 0;
	virtual void Reset(float initPhs = 0) = 0;
	virtual AmpValue Sample(AmpValue in) = 0;
	virtual void Samples(SampleBlock *block)
	{
		int n = block->size;
		AmpValue *in = block->in;
		AmpValue *out = block->out;
		while (--n >= 0)
			*out++ = Sample(*in++);
	}
	virtual int IsFinished()
	{
		return 1;
	}
};


#endif
