///////////////////////////////////////////////////////////////
//
// BasicSynth - Fast wavetable generator using Q24.40 index
// Very precise phase increment if you have 64-bit data type.
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////////

#ifndef _GENWAVE64_H_
#define _GENWAVE64_H_

//#define two48 (65563.0 * 65536.0 * 65536.0)
#define two40 (65536.0 * 65536.0 * 256.0)
#define two32 (65536.0 * 65536.0)

#ifdef GCC32
#define round64 0x8000000000LLU
typedef long long int64;
#endif

#ifdef _MSC_VER
#define round64 0x8000000000
typedef __int64 int64;
#endif

class GenWave64 : public GenWaveWT
{
private:
	int64 i64Index;
	int64 i64IndexIncr;
	int64 i64IndexMask;

public:
	GenWave64()
	{
		i64Index = 0;
		i64IndexIncr = 0;
		i64IndexMask = ((int64)synthParams.itableLength << 40) - 1;
	}

	inline void CalcPhase()
	{
		i64IndexIncr = (int64) (indexIncr * two40);
	}

	virtual void Modulate(FrqValue d)
	{
		GenWaveWT::Modulate(d);
		CalcPhase();
	}

	virtual void PhaseModWT(PhsAccum phs)
	{
		if (phs >= synthParams.ftableLength)
			phs -= synthParams.ftableLength;
		else if (phs < 0)
			phs += synthParams.ftableLength;
		i64Index += (int64) ((double)phs * two40); //<-- can overflow with large tables
		//i64Index += (int64) (phs * two32) << 8;  //<-- use this if tables > 16k
		i64Index &= i64IndexMask;
	}

	virtual void Reset(float initPhs = 0)
	{
		GenWaveWT::Reset(initPhs);
		CalcPhase();
		if (initPhs >= 0)
			i64Index = (int64) (index * two40);
	}

	virtual AmpValue Gen()
	{
		// cast to bsInt32 helps 32-bit processors with emulated 64 bits
		// (avoids 64 bit multiply for table index)
		// you can use this on a true 64 bit system, but it doesn't really matter much
		// AmpValue v = waveTable[(i64Index + 0x8000000000) >> 40];
		AmpValue v = waveTable[(bsInt32) ((i64Index + round64) >> 40)];
		i64Index = (i64Index + i64IndexIncr) & i64IndexMask;
		return v;
	}
};

#endif
