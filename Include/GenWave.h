///////////////////////////////////////////////////////////////
//
// BasicSynth - GenWave
//
// Various waveform generators using direct calculation
//
// GenWave - sin wave generator using sin() lib function
// GenWaveSaw - sawtooth wave generator
// GenWaveSqr - square wave generator
// GenWaveSqr32 - square wave generator, integers
// GenWaveTri - triangle wave generator
//
// Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _GENWAVE_H_
#define _GENWAVE_H_

// Direct calculation of Sin wave - slower, but as accurate as we can get.
// This class is the base class for all other waveform generators
class GenWave : public GenUnit
{
public:
	PhsAccum indexIncr;
	PhsAccum index;
	FrqValue frq;

	GenWave()
	{
		indexIncr = 0;
		index = 0;
		frq = 440;
	}

	// Fo
	virtual void Init(int n, float *p)
	{
		if (n > 0)
			SetFrequency((FrqValue)p[0]);
		Reset();
	}

	virtual AmpValue Sample(AmpValue in)
	{
		return Gen();
	}

	// store the Frequency, caller must Reset() to apply the new frequency
	inline void SetFrequency(FrqValue f)
	{
		frq = f;
	}

	inline FrqValue GetFrequency()
	{
		return frq;
	}

	virtual void Reset(float initPhs = 0)
	{
		indexIncr = (PhsAccum)frq * synthParams.frqRad;
		if (initPhs >= 0)
		{
			index = (initPhs / twoPI) * indexIncr;
			while (index >= twoPI)
				index -= twoPI;
		}
	}

	// modulate by changing the phase increment, d is in HZ
	virtual void Modulate(FrqValue d)
	{
		indexIncr = (PhsAccum)(frq + d) * synthParams.frqRad;
		if (indexIncr >= twoPI)
			indexIncr -= twoPI;
		else if (indexIncr < 0)
			indexIncr += twoPI;
	}

	// modulate by adding to the phase, phs is in radians
	virtual void PhaseMod(PhsAccum phs)
	{
		if ((index += phs) >= twoPI)
		{
			do
				index -= twoPI;
			while (index >= twoPI);
		}
		else if (index < 0)
		{
			do
				index += twoPI;
			while (index < 0);
		}
	}

	// Generate the next sample
	virtual AmpValue Gen()
	{
		AmpValue v = sinv(index);
		if ((index += indexIncr) >= twoPI)
			index -= twoPI;
		return v;
	}
};

// Direct calculation of Sawtooth wave - fast, but not BW limited
#define oneDivPI (1.0/PI)

class GenWaveSaw : public GenWave
{
public:
	virtual void Modulate(FrqValue d)
	{
		PhsAccum f = (PhsAccum)(frq + d);
		if (f < 0)
			f = -f;
		indexIncr = (2 * f) / synthParams.sampleRate;
	}

	virtual void PhaseMod(PhsAccum phs)
	{
		// phase modulation works, "mostly"
		//index += 2 * (phs / twoPI);
		index += phs * oneDivPI;
		if (index >= 1)
			index -= 2;
		else if (index < -1)
			index += 2;
	}

	virtual void Reset(float initPhs = 0)
	{
		indexIncr = (PhsAccum)((2 * frq) / synthParams.sampleRate);
		if (initPhs >= 0)
		{
			index = (initPhs * oneDivPI) - 1;
			while (index >= 1)
				index -= 2;
		}
	}

	virtual AmpValue Gen()
	{
		AmpValue v = index;
		if ((index += indexIncr) >= 1)
			index = -1;
		return v;
		/* Alternate calculation, slower
		AmpValue v = (index * oneDivPI) - 1;
		if ((index += indexIncr) >= twoPI)
			index -= twoPI;
		return v;
		*/
	}
};

// Direct calculation of Triangle wave - fast, but not BW limited
// Note that phase index varies from [-PI, PI] not [0, 2PI]

#define twoDivPI (2.0/PI)

class GenWaveTri : public GenWave
{
public:
	virtual void Modulate(FrqValue d)
	{
		indexIncr = (PhsAccum)(frq + d) * synthParams.frqRad;
		if (indexIncr >= PI)
			indexIncr -= twoPI;
		else if (indexIncr < -PI)
			indexIncr += twoPI;
	}

	virtual void PhaseMod(PhsAccum phs)
	{
		index += phs;
		if (index >= PI)
			index -= twoPI;
		else if (index < -PI)
			index += twoPI;
	}

	virtual AmpValue Gen()
	{
		//AmpValue triValue = (AmpValue)(1 + (2 * fabs(index - PI) / PI);
		AmpValue triValue = (AmpValue)(index * twoDivPI);
		if (triValue < 0)
			triValue = 1.0 + triValue;
		else
			triValue = 1.0 - triValue;
		if ((index += indexIncr) >= PI)
			index -= twoPI;
		return triValue;
	}
};

// Direct calculation of Square wave - fast, but not BW limited
// This has a settable min/max so that it can toggle
// from 0/1 as well as -1/+1, or any other pair of values
class GenWaveSqr : public GenWave
{
private:
	PhsAccum midPoint;
	PhsAccum dutyCycle;
	AmpValue ampMax;
	AmpValue ampMin;

public:
	GenWaveSqr()
	{
		midPoint = PI;
		dutyCycle = 50.0;
		ampMax = 1.0;
		ampMin = -1.0;
	}

	void inline SetDutyCycle(float d)
	{
		dutyCycle = (PhsAccum)d;
	}

	void inline SetMinMax(AmpValue amin, AmpValue amax)
	{
		ampMin = amin;
		ampMax = amax;
	}

	// Fo, Duty%
	virtual void Init(int n, float *p)
	{
		if (n > 0)
		{
			SetFrequency(p[0]);
			if (n > 1)
				SetDutyCycle(p[1]);
		}
		Reset();
	}

	void InitSqr(FrqValue f, float duty)
	{
		SetDutyCycle(duty);
		SetFrequency(f);
		Reset();
	}

	virtual void Reset(float initPhs = 0)
	{
		GenWave::Reset(initPhs);
		midPoint = twoPI * (dutyCycle / 100.0);
	}

	virtual AmpValue Gen()
	{
		AmpValue v = (index > midPoint) ? ampMin : ampMax;
		if ((index += indexIncr) >= twoPI)
			index -= twoPI;
		return v;
	}
};

// Square waves using integer values
// this discards the fractional part of the phase increment
// and thus avoids phase jitter, but produces slight
// frequency error that gets worse at higher frequencies.
// Modulation doesn't work very well either.
// Makes a very efficient gate signal and not much else.
// This has a settable min/max so that it can toggle
// from 0/1 as well as -1/+1, or any other pair of values
class GenWaveSqr32 : public GenWave
{
private:
	bsInt32 sqPeriod;
	bsInt32 sqMidPoint;
	bsInt32 sqPhase;
	float dutyCycle;
	AmpValue ampMax;
	AmpValue ampMin;

	void inline CalcPeriod(FrqValue f)
	{
		sqPeriod = (bsInt32) ((synthParams.sampleRate / f) + 0.5);
		sqMidPoint = (bsInt32) (((float)sqPeriod * dutyCycle) / 100.0);
	}

public:
	GenWaveSqr32()
	{
		sqPeriod = 2;
		sqMidPoint = 1;
		sqPhase = 0;
		dutyCycle = 50.0;
		ampMax = 1.0;
		ampMin = -1.0;
	}

	void inline SetDutyCycle(float duty)
	{
		dutyCycle = duty;
	}

	void inline SetMinMax(AmpValue amin, AmpValue amax)
	{
		ampMin = amin;
		ampMax = amax;
	}

	// Fo, Duty%
	virtual void Init(int n, float *p)
	{
		if (n > 0)
		{
			SetFrequency((FrqValue)p[0]);
			if (n > 1)
				SetDutyCycle(p[1]);
		}
		Reset();
	}

	void InitSqr(FrqValue f, float duty)
	{
		SetDutyCycle(duty);
		SetFrequency(f);
		Reset();
	}

	virtual void Reset(float initPhs = 0)
	{
		CalcPeriod(frq);
		sqPhase = (bsInt32) ((initPhs / twoPI) * (float)sqPeriod);
	}

	// Modulate is OK at lower values for d
	virtual void Modulate(FrqValue d)
	{
		FrqValue f = frq + d;
		if (f < 0)
			f = -f;
		CalcPeriod(f);
	}

	virtual void PhaseMod(PhsAccum phs)
	{
		// Doesn't really work because calculated phase
		// offset will discard fractional portion.
		sqPhase += (long) ((phs / twoPI) * (float)sqPeriod);
		if (sqPhase >= sqPeriod)
			sqPhase -= sqPeriod;
		else if (sqPhase < 0)
			sqPhase += sqPeriod;
	}

	virtual AmpValue Gen()
	{
		AmpValue v = (sqPhase < sqMidPoint) ? ampMax : ampMin;
		if (++sqPhase >= sqPeriod)
			sqPhase = 0;
		return v;
	}
};

// Normalized phase integrator. 
// output counts up from 0 to 1-incr
class Phasor : public GenUnit
{
protected:
	PhsAccum phase;
	PhsAccum phsIncr;
	FrqValue frq;

public:
	Phasor()
	{
		phase = 0;
		phsIncr = 0;
		frq = 0;
	}

	inline void SetFrequency(FrqValue f)
	{
		frq = f;
	}

	void Init(int n, float *f)
	{
		if (n > 0)
		{
			SetFrequency(f[0]);
			Reset(0);
		}
	}

	void Reset(float initPhs = 0)
	{
		phsIncr = (PhsAccum)frq / synthParams.sampleRate;
		if (initPhs >= 0)
		{
			phase = (initPhs / twoPI) * phsIncr;
			while (phase >= 1)
				phase -= 1;
		}
	}

	// set 'in' to the max value, usually 1
	AmpValue Sample(AmpValue in)
	{
		in *= (AmpValue) phase;
		if ((phase += phsIncr) >= 1)
			phase -= 1;
		return in;
	}
};

// Normalized phase integrator. 
// output counts down from 1 to 0
class PhasorR : public Phasor
{
public:
	void Reset(float initPhs = 0)
	{
		Phasor::Reset(initPhs);
		phase = 1.0 - phase;
	}

	// set 'in' to the max value, usually 1
	AmpValue Sample(AmpValue in)
	{
		in *= (AmpValue) phase;
		if ((phase -= phsIncr) < 0)
			phase += 1;
		return in;
	}
};

#endif

