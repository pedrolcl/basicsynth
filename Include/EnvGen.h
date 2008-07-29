///////////////////////////////////////////////////////////////
//
// BasicSynth - EnvGen
//
// Fixed duration AR type envelope generators, linear, exponential and log
// All classes in this file vary output from 0 - peak - 0
//
// EnvGen - linear attack and decay, base class for other types
// EnvGenExp - exponential attack and decay
// EnvGenLog - log attack and decay
//
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _ENVGEN_H_
#define _ENVGEN_H_

class EnvGen : public GenUnit
{
protected:
	bsUint32  index;
	bsUint32  totalSamples;
	bsUint32  attackTime;
	bsUint32  decayTime;
	bsUint32  sustainTime;
	bsUint32  decayStart;
	AmpValue envInc;
	AmpValue peakAmp;
	AmpValue volume;
	FrqValue attack;
	FrqValue decay;
	FrqValue duration;

public:
	EnvGen()
	{
		index = 0;
		volume = 0;
		attackTime = 1;
		decayTime = 1;
		sustainTime = 1;
		decayStart = 1;
		envInc = 1.0;
		peakAmp = 1.0;
		totalSamples = 0;
		attack = 0;
		decay = 0;
		duration = 0;
	}

	virtual void Init(int n, float *f)
	{
		if (n >= 4)
			InitEG((AmpValue)f[0], f[1], f[2], f[3]);
	}

	virtual void InitEG(AmpValue peak, FrqValue dur, FrqValue atk, FrqValue dec)
	{
		duration = dur;
		attack = atk;
		decay = dec;

		totalSamples = (bsUint32) ((synthParams.sampleRate * duration) + 0.5);
		if (totalSamples < 3)
			totalSamples = 3;
		attackTime = (bsUint32) (attack * synthParams.sampleRate);
		decayTime  = (bsUint32) (decay * synthParams.sampleRate);
		while ((attackTime + decayTime) >= totalSamples)
		{
			if (attackTime > 1)
				attackTime--;
			if (decayTime > 1)
				decayTime--;
		}
		sustainTime = totalSamples - (attackTime + decayTime);
		decayStart = totalSamples - decayTime;
		peakAmp = peak;
		Reset();
	}

	// 0 <= initPhs < 1 will set the volume to the appropriate level
	// and can be used to re-trigger or cycle the envelope.
	// initPhs < 0 will not change the current level or index
	virtual void Reset(float initPhs = 0)
	{
		if (initPhs >= 0)
		{
			index = (bsUint32) (initPhs * duration * synthParams.sampleRate);
			if (index < attackTime)
			{
				envInc = peakAmp / (AmpValue) attackTime;
				volume = (AmpValue) index * envInc;
			}
			else if (index >= decayStart)
			{
				envInc = -peakAmp / (AmpValue) decayTime;
				volume = peakAmp + ((AmpValue) (decayStart - index) * envInc);
			}
			else
				volume = peakAmp;
		}
	}

	virtual AmpValue Sample(AmpValue inval)
	{
		return Gen();
	}

	virtual AmpValue Gen()
	{
		if (index >= totalSamples)
			return 0;
		if (index < attackTime || index > decayStart)
			volume += envInc;
		else if (index == attackTime)
			volume = peakAmp;
		else if (index == decayStart)
			envInc = -volume / (AmpValue) decayTime;
		index++;
		return volume;
	}

	virtual int IsFinished()
	{
		return index >= totalSamples;
	}
};

class EnvGenExp : public EnvGen
{
private:
	AmpValue expMin;
	AmpValue expFactor;
	AmpValue expCurrent;

public:
	EnvGenExp()
	{
		expMin = 0.2;
		expCurrent = 0;
		expFactor = 0;
	}

	virtual void SetBias(AmpValue b)
	{
		expMin = b;
	}

	virtual void Reset(float initPhs = 0)
	{
		EnvGen::Reset(initPhs);
		if (initPhs >= 0)
		{
			if (index < decayStart)
			{
				expFactor = (AmpValue) pow((1+expMin)/expMin, (AmpValue)1.0 / (AmpValue) attackTime);
				if (index > 0)
					expCurrent = expMin * pow(expFactor, (AmpValue) index);
				else
					expCurrent = expMin;
			}
			else
			{
				expCurrent = 1 + expMin;
				expFactor = (AmpValue) pow(expMin/(1+expMin), (AmpValue)1.0 / (AmpValue) decayTime);
				if (index > decayStart)
					expCurrent = (1+expMin) * pow(expFactor, (AmpValue) (index - decayStart));
				else
					expCurrent = 1+expMin;
			}
		}
	}

	virtual AmpValue Gen()
	{
		if (index >= totalSamples)
			return 0;
		if (index < attackTime || index > decayStart)
		{
			volume = (expCurrent - expMin) * peakAmp;
			expCurrent *= expFactor;
		}
		else if (index == attackTime)
			volume = peakAmp;
		else if (index == decayStart)
		{
			expCurrent = 1 + expMin;
			expFactor = (AmpValue) pow(expMin/expCurrent, (AmpValue)1.0 / (AmpValue) (decayTime-1));
		}
		index++;
		return volume;
	}
};

class EnvGenLog : public EnvGen
{
private:
	AmpValue expMin;
	AmpValue expFactor;
	AmpValue expCurrent;

public:
	EnvGenLog()
	{
		expMin = 0.2f;
		expCurrent = 0.0f;
		expFactor = 0.0f;
	}

	virtual void SetBias(AmpValue b)
	{
		expMin = b;
	}

	virtual void Reset(float initPhs = 0)
	{
		EnvGen::Reset(initPhs);
		if (initPhs >= 0)
		{
			if (index < decayStart)
			{
				expFactor = (AmpValue) pow(expMin/(1+expMin), (AmpValue)1.0 / (AmpValue) attackTime);
				if (index > 0)
					expCurrent = (1+expMin) * pow(expFactor, (AmpValue) index);
				else
					expCurrent = 1 + expMin;
			}
			else
			{
				expFactor = (AmpValue) pow((1+expMin)/expMin, (AmpValue) 1.0 / (AmpValue) decayTime);
				if (index > decayStart)
					expCurrent = expMin * pow(expFactor, (AmpValue) (index - decayStart));
				else
					expCurrent = expMin;
			}
		}
	}

	virtual AmpValue Gen()
	{
		if (index >= totalSamples)
			return 0;
		if (index < attackTime || index > decayStart)
		{
			volume = (1.0 - (expCurrent - expMin)) * peakAmp;
			expCurrent *= expFactor;
		}
		else if (index == attackTime)
			volume = peakAmp;
		else if (index == decayStart)
		{
			expCurrent = expMin;
			expFactor = (AmpValue) pow((1+expMin)/expMin, 1.0f / (AmpValue) decayTime);
		}
		index++;
		return volume;
	}
};

#endif
