///////////////////////////////////////////////////////////////
// BasicSynth -
//
// Segment based envelope generators.
// These have variable segments with release
//
// 1. AR
// 2. ADSR
// 3. A3SR
// 4. Multi-Seg with fixed sustain
// 5. Multi-seg with variable sustain
// 6. Pre-calculated table
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _ENVGENSEG_H_
#define _ENVGENSEG_H_

// These are the curve types for the following generators
enum EGSegType
{
	nulSeg = 0, // undefined
	linSeg = 1, // linear
	expSeg = 2, // exponential
	logSeg = 3, // logarithmic
	susSeg = 4  // sustain - constant value
};

// A Sustain segment. This always returns the same value
class EnvSeg : public GenUnit
{
protected:
	long  count;
	AmpValue value;
	AmpValue start;
	AmpValue end;
	FrqValue rate;

public:
	EnvSeg()
	{
		count = 0;
		value = 0;
		start = 0;
		end = 0;
		rate = 0;
	}

	virtual EGSegType GetType()
	{
		return susSeg;
	}

	// rate, start, end
	virtual void Init(int n, float *v)
	{
		if (n >= 3)
			InitSeg(FrqValue(v[0]), AmpValue(v[1]), AmpValue(v[2]));
	}

	inline void SetStart(AmpValue s) { start = s; }
	inline void SetLevel(AmpValue s) { end = s; }
	inline void SetRate(FrqValue r)  { rate = r; }

	virtual void InitSeg(FrqValue r, AmpValue s, AmpValue e)
	{
		rate = r;
		start = s;
		end = e;
		count = (long) (rate * synthParams.sampleRate);
		Reset();
	}

	inline AmpValue GetStart() { return start; }
	inline AmpValue GetLevel() { return end; }
	inline FrqValue GetRate()  { return rate; }

	virtual void GetSettings(FrqValue& r, AmpValue& s, AmpValue& e)
	{
		r = rate;
		s = start;
		e = end;
	}

	virtual void Copy(EnvSeg *tp)
	{
		rate = tp->rate;
		start = tp->start;
		end = tp->end;
	}

	virtual void InitSegTick(long r, AmpValue s, AmpValue e)
	{
		rate = (FrqValue)r * synthParams.sampleRate;
		count = r;
		start = s;
		end = e;
		Reset(0);
	}

	virtual void Reset(float initPhs = 0)
	{
		value = start;
	}

	virtual AmpValue Value()
	{
		return value;
	}

	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	virtual AmpValue Gen()
	{
		count--;
		return value;
	}

	virtual int IsFinished()
	{
		return count <= 0;
	}
};

// Linear segment: y = ax + b
class EnvSegLin : public EnvSeg
{
protected:
	AmpValue incr;

public:
	EnvSegLin()
	{
		incr = 1;
	}

	virtual EGSegType GetType()
	{ 
		return linSeg;
	}

	virtual void Reset(float initPhs = 0)
	{
		EnvSeg::Reset(initPhs);
		incr = (end - start);
		if (count > 0)
			incr /= (AmpValue) count;
	}

	virtual AmpValue Gen()
	{
		if (--count >= 0)
			return value += incr;
		return end;
	}

};

///////////////////////////////////////////////////////////
// exponential segment y = ab^x + c
///////////////////////////////////////////////////////////
class EnvSegExp : public EnvSeg
{
protected:
	AmpValue range;
	AmpValue bias;
	AmpValue offs;
	AmpValue incr;

public:
	EnvSegExp()
	{
		bias = 0.2;
		range = 1;
		offs = 0;
		incr = 0;
	}

	virtual EGSegType GetType()
	{ 
		return expSeg;
	}

	void SetBias(float b)
	{
		bias = (AmpValue)b;
	}

	virtual void Reset(float initPhs = 0)
	{
		EnvSeg::Reset(initPhs);
		range = fabs(end - start);
		if (count > 0)
		{
			AmpValue ratio;
			if (end >= start)
			{
				value = bias;
				ratio = (1+bias)/bias;
				offs = start;
			}
			else
			{
				value = 1.0+bias;
				ratio = bias/value;
				offs = end;
			}
			incr = (AmpValue) pow(ratio, 1.0f / (float) count);
		}
		else
			incr = 1;
	}

	virtual AmpValue Gen()
	{
		if (--count >= 0)
		{
			value *= incr;
			return ((value - bias) * range) + offs;
		}
		return end;
	}
};

///////////////////////////////////////////////////////////
// "logarithmic" segment y = a(1 - (b^x)) + c
///////////////////////////////////////////////////////////
class EnvSegLog : public EnvSegExp
{
protected:

public:
	virtual EGSegType GetType()
	{ 
		return logSeg; 
	}

	virtual void Reset(float initPhs = 0)
	{
		count = (long) (rate * synthParams.sampleRate);
		value = 0;
		range = fabs(end - start);
		if (count > 0)
		{
			AmpValue ratio;
			if (end >= start)
			{
				value = 1+bias;
				ratio = bias/value;
				offs = start;
			}
			else
			{
				value = bias;
				ratio = (1+bias)/bias;
				offs = end;
			}
			incr = (AmpValue) pow(ratio, 1.0f / (float) count);
		}
		else
			incr = 1;
	}

	virtual AmpValue Gen()
	{
		if (--count >= 0)
		{
			value *= incr;
			return ((1.0 - (value - bias)) * range) + offs;
		}
		return end;
	}
};

struct SegVals
{
	FrqValue rate;
	AmpValue level;
	EGSegType type;
};

///////////////////////////////////////////////////////////
// EnvDef - structure to hold values for an envelope gen.
///////////////////////////////////////////////////////////
struct EnvDef
{
	int nsegs;
	int suson;
	AmpValue start;
	SegVals *segs;

	EnvDef()
	{
		nsegs = 0;
		start = 0;
		suson = 1;
		segs = NULL;
	}

	~EnvDef()
	{
		delete segs;
	}

	void Alloc(int n, AmpValue s, int so = 1)
	{
		Clear();
		nsegs = n;
		start = s;
		suson = 1;
		segs = new SegVals[n];
	}

	int NumSegs() { return nsegs; }

	inline void SetStart(AmpValue st) { start = st; }
	inline void SetRate(int n, FrqValue rt) { segs[n].rate = rt; }
	inline void SetLevel(int n, AmpValue lv) { segs[n].level = lv; }
	inline void SetType(int n, EGSegType ty) { segs[n].type = ty; }

	inline AmpValue GetStart() { return start; }
	inline FrqValue GetRate(int n)  { return segs[n].rate; }
	inline AmpValue GetLevel(int n) { return segs[n].level; }
	inline EGSegType GetType(int n) { return segs[n].type; }

	void Set(int n, FrqValue rt, AmpValue lv, EGSegType ty)
	{
		if (n < nsegs)
		{
			segs[n].rate = rt;
			segs[n].level = lv;
			segs[n].type = ty;
		}
	}

	void Get(int n, FrqValue& rt, AmpValue& lv, EGSegType& ty)
	{
		if (n < nsegs)
		{
			rt = segs[n].rate;
			lv = segs[n].level;
			ty = segs[n].type;
		}
	}

	void Copy(EnvDef *dp)
	{
		Alloc(dp->nsegs, dp->start, dp->suson);
		if (nsegs > 0)
			memcpy(segs, dp->segs, nsegs*sizeof(SegVals));
	}

	void Clear()
	{
		if (segs)
			delete segs;
		nsegs = 0;
		start = 0;
		segs = NULL;
	}
};

///////////////////////////////////////////////////////////
// Interface definition for multi-segment envelope generators
///////////////////////////////////////////////////////////
class EnvGenUnit : public GenUnit
{
public:
	virtual AmpValue Gen() { return 0; }
	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	virtual void Release() { }
	virtual void GetEnvDef(EnvDef *) { }
	virtual void SetEnvDef(EnvDef *) { }
	virtual void Copy(EnvGenUnit *) { }
};

///////////////////////////////////////////////////////////
// EnvGenSeg : Variable multi-segment envelope generator
// This is the base class for multi-segment EG classes.
//
// The array of segment values is dynamic, allowing any
// number of segments. Each segment has a rate, level and
// type, thus allowing a mixture of linear, exponential,
// log curves. The "segObj" array is initialized to 
// the appropriate curve generator (see above) when
// the SetType() method is called.
//
// This class does not implement indeterminate sustain. 
// However, it is possible to create a fixed duration 
// sustain segment by setting the start and end points
// equal for one of the segments and setting the type
// to "susSeg". 
//
// The AR, ADSR, and A3SR classes derive from this class
// and implement a state-machine that stops at the sustain
// point if configured.
///////////////////////////////////////////////////////////
class EnvGenSeg : public EnvGenUnit
{
protected:
	int numSeg;
	int index;
	int susOn;
	SegVals *segRLT;
	EnvSeg **segObj;

	EnvSegLin egsLin;
	EnvSegExp egsExp;
	EnvSegLog egsLog;
	EnvSeg    egsSus;
	EnvSeg *seg;

	AmpValue lastVal;
	AmpValue segStart;

public:
	EnvGenSeg()
	{
		numSeg = 0;
		index = 0;
		lastVal = 0;
		susOn = 0;
		segStart = 0;
		segObj = NULL;
		seg = NULL;
		segRLT = NULL;
	}

	virtual ~EnvGenSeg()
	{
		delete segRLT;
		delete segObj;
	}

	virtual void Copy(EnvGenUnit *tp)
	{
		EnvGenSeg *ap = (EnvGenSeg *)tp;
		SetStart(ap->segStart);
		SetSegs(ap->numSeg);
		for (int n = 0; n < numSeg; n++)
			SetSegN(n, ap->segRLT[n].rate, ap->segRLT[n].level, ap->segRLT[n].type);
	}

	// n -> number of segments
	// v -> array of rate, level, type tuples
	//      L0,{R1,L1.T1}..{Rn,Ln,Tn}
	virtual void Init(int n, float *v)
	{
		if (n > 0)
		{
			segStart = *v++;
			n--;
			SetSegs(n/3);
			for (int i = 0; i < numSeg; i++)
			{
				SetSegN(i, FrqValue(v[0]), AmpValue(v[1]), (EGSegType) (int) v[2]);
				v += 3;
			}
		}
		Reset();
	}

	virtual void SetEnvDef(EnvDef *def)
	{
		SetSegs(def->nsegs);
		SetStart(def->start);
		susOn = def->suson;
		for (int n = 0; n < numSeg; n++)
			SetSegN(n, def->segs[n].rate, def->segs[n].level, def->segs[n].type);
	}

	virtual void GetEnvDef(EnvDef *def)
	{
		def->Alloc(numSeg, segStart, susOn);
		memcpy(def->segs, segRLT, numSeg*sizeof(SegVals));
	}

	int GetSegs() { return numSeg; }

	virtual void SetSegs(int count)
	{
		if (segRLT)
		{
			delete segRLT;
			segRLT = NULL;
		}
		if (segObj)
		{
			delete segObj;
			segObj = NULL;
		}
		if ((numSeg = count) < 1)
			numSeg = 1;
		segRLT = new SegVals[numSeg];
		segObj = new EnvSeg *[numSeg];
		for (int n = 0; n < numSeg; n++)
		{
			segRLT[n].level = 0;
			segRLT[n].rate = 0;
			segRLT[n].type = linSeg;
			segObj[n] = &egsLin;
		}
	}

	inline void SetSuson(int on) { susOn = on; }
	inline void SetStart(AmpValue lvl) { segStart = lvl; }
	inline void SetRate(int segn, FrqValue rt) { segRLT[segn].rate = rt; }
	inline void SetLevel(int segn, AmpValue lvl) { segRLT[segn].level = lvl; }

	inline int GetSusOn()      { return susOn; }
	inline AmpValue GetStart() { return segStart; }
	inline FrqValue GetRate(int segn) { return segRLT[segn].rate; }
	inline AmpValue GetLevel(int segn) { return segRLT[segn].level; }
	inline EGSegType GetType(int segn) { return segRLT[segn].type; }

	void SetType(int segn, EGSegType ty)
	{
		segRLT[segn].type = ty;
		switch(ty)
		{
		default:
		case linSeg:
			segObj[segn] = &egsLin;
			break;
		case expSeg:
			segObj[segn] = &egsExp;
			break;
		case logSeg:
			segObj[segn] = &egsLog;
			break;
		case susSeg:
			segObj[segn] = &egsSus;
			break;
		}
	}

	void GetSegN(int segn, FrqValue& rt, AmpValue& lvl, EGSegType& typ)
	{
		if (segn >= numSeg)
			return;
		rt  = segRLT[segn].rate;
		lvl = segRLT[segn].level;
		typ = segRLT[segn].type;
	}

	void SetSegN(int segn, FrqValue rt, AmpValue lvl, EGSegType typ = linSeg)
	{
		if (segn >= numSeg)
			return;
		SetRate(segn, rt);
		SetLevel(segn, lvl);
		SetType(segn, typ);
	}

	virtual void Reset(float initPhs = 0)
	{
		if (initPhs >= 0)
		{
			index = 0;
			lastVal = segStart;
			NextSeg();
		}
	}

	virtual void Release()
	{
		// NOOP for the base class...
		// derived classes should transition to release state
	}

	virtual void NextSeg()
	{
		if (index < numSeg)
		{
			seg = segObj[index];
			if (seg != NULL)
				seg->InitSeg(segRLT[index].rate, lastVal, segRLT[index].level);
			index++;
		}
		else
			seg = segObj[numSeg-1];
	}

	virtual AmpValue Gen()
	{
		lastVal = seg->Gen();
		if (seg->IsFinished())
			NextSeg();
		return lastVal;
	}

	virtual int IsFinished()
	{
		return (index >= numSeg && seg->IsFinished());
	}
}; 

///////////////////////////////////////////////////////////
// Multi-attack, sustain, single release segments
///////////////////////////////////////////////////////////
class EnvGenSegSus : public EnvGenSeg
{
protected:
	int state;
	int relSeg;

public:
	EnvGenSegSus()
	{
		state = 3;
		relSeg = 0;
	}

	virtual void Reset(float initPhs = 0)
	{
		if (initPhs >= 0)
		{
			state = 0;
			relSeg = numSeg - 1;
			EnvGenSeg::Reset(initPhs);
		}
	}

	virtual AmpValue Gen()
	{
		switch (state)
		{
		case 0:
			lastVal = seg->Gen();
			if (seg->IsFinished())
			{
				if (index == relSeg)
					state = 1;
				else
					NextSeg();
			}
			break;
		case 1:
			if (!susOn)
			{
				NextSeg();
				state = 2;
				lastVal = seg->Gen();
			}
			break;
		case 2:
			lastVal = seg->Gen();
			if (seg->IsFinished())
				state = 3;
			break;
		}
		return lastVal;
	}

	virtual void Release()
	{
		if (state < 2)
		{
			state = 2;
			index = relSeg;
			NextSeg();
		}
	}

	virtual int IsFinished()
	{
		return state == 3;
	}
};

///////////////////////////////////////////////////////////
// AR - attack, [sustain,] and release
///////////////////////////////////////////////////////////
class EnvGenAR : public EnvGenSegSus
{
protected:

public:
	EnvGenAR()
	{
		susOn = 1;
		EnvGenSeg::SetSegs(2);
	}

	inline void SetAtkRt(FrqValue val)  { SetRate(0, val); }
	inline void SetRelRt(FrqValue val)  { SetRate(1, val); }
	inline void SetSus(AmpValue val)    { SetLevel(0, val); }
	inline void SetSusOn(int val)       { susOn = val; }
	inline FrqValue GetAtkRt() { return GetRate(0); }
	inline FrqValue GetRelRt() { return GetRate(1); }
	inline AmpValue GetSus()   { return GetLevel(0); }

	void SetSegs(int count) { }
	int GetSegs() { return 2; }

	virtual void InitAR(FrqValue ar, AmpValue sl, FrqValue rr, int son, EGSegType t)
	{
		SetSuson(son);
		SetStart(0);
		SetSegN(0, ar, sl, t);
		SetSegN(1, rr, 0, t);
		Reset();
	}
};

///////////////////////////////////////////////////////////
// ADSR - attack, decay, sustain, and release
///////////////////////////////////////////////////////////
class EnvGenADSR : public EnvGenSegSus
{
protected:

public:
	EnvGenADSR()
	{
		susOn = 1;
		EnvGenSeg::SetSegs(3);
	}

	void SetAtkRt(FrqValue val)  { SetRate(0, val); }
	void SetAtkLvl(AmpValue val) { SetLevel(0, val); }
	void SetDecRt(FrqValue val)  { SetRate(1, val); }
	void SetSusLvl(AmpValue val) { SetLevel(1, val); }
	void SetRelRt(FrqValue val)  { SetRate(2, val); }
	void SetRelLvl(AmpValue val) { SetLevel(2, val); }
	void SetType(EGSegType ty)   
	{
		EnvGenSeg::SetType(0, ty);
		EnvGenSeg::SetType(1, ty);
		EnvGenSeg::SetType(2, ty);
	}

	FrqValue GetAtkRt() { return GetRate(0); }
	AmpValue GetAtkLvl(){ return GetLevel(0); }
	FrqValue GetDecRt() { return GetRate(1); }
	AmpValue GetSusLvl(){ return GetLevel(1); }
	FrqValue GetRelRt() { return GetRate(2); }
	AmpValue GetRelLvl(){ return GetLevel(2); }
	EGSegType GetType()  { return EnvGenSeg::GetType(0); }

	void SetSegs(int count) { }
	int GetSegs() {	return 3; }

	virtual void InitADSR(AmpValue st, FrqValue ar, AmpValue al, FrqValue dr, 
	                      AmpValue sl, FrqValue rr, AmpValue rl, EGSegType t = logSeg)
	{
		SetStart(st);
		SetSegN(0, ar, al, t);
		SetSegN(1, dr, sl, t);
		SetSegN(2, rr, rl, t);
		Reset();
	}
};

///////////////////////////////////////////////////////////
// Multi-attack, sustain, multi-decay segments
///////////////////////////////////////////////////////////
class EnvGenMulSus : public EnvGenUnit
{
private:
	EnvGenSeg atk;
	EnvGenSeg dec;
	int state;

	AmpValue lastVal;
	AmpValue segStart;

public:
	EnvGenMulSus()
	{
		state = 0;
		lastVal = 0;
		segStart = 0;
	}

	virtual void Copy(EnvGenUnit *tp)
	{
		EnvGenMulSus *ap = (EnvGenMulSus *)tp;
		atk.Copy(&ap->atk);
		dec.Copy(&ap->dec);
		state = ap->state;
		lastVal = ap->lastVal;
		segStart = ap->segStart;
	}

	virtual void Init(int n, float *v)
	{
		if (n > 0)
		{
			AmpValue start = *v++;
			atk.SetStart(start);
			int atksegs = (int) *v++;
			int decsegs = (int) *v++;
			SetSegs(atksegs, decsegs);
			n += 3;
			int i;
			for (i = 0; i < atksegs; i++)
			{
				atk.SetSegN(i, FrqValue(v[0]), AmpValue(v[1]), (EGSegType) (int) v[2]);
				v += 3;
				start = v[1];
			}
			dec.SetStart(start);
			for (i = 0; i < decsegs; i++)
			{
				dec.SetSegN(i, FrqValue(v[0]), AmpValue(v[1]), (EGSegType) (int) v[2]);
				v += 3;
			}
		}
		Reset();
	}

	void SetStart(AmpValue sv)
	{
		atk.SetStart(sv);
	}

	void SetSegs(int atks, int decs)
	{
		atk.SetSegs(atks);
		dec.SetSegs(decs);
	}

	void SetAtkN(int segn, FrqValue rt, AmpValue lvl, EGSegType typ = linSeg)
	{
		atk.SetSegN(segn, rt, lvl, typ);
	}

	void SetDecN(int segn, FrqValue rt, AmpValue lvl, EGSegType typ = linSeg)
	{
		dec.SetSegN(segn, rt, lvl, typ);
	}

	virtual void Reset(float initPhs = 0)
	{
		state = 0;
		atk.Reset(initPhs);
	}

	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	virtual void Release()
	{
		dec.SetStart(lastVal);
		dec.Reset();
		state = 2;
	}

	virtual AmpValue Gen()
	{
		switch (state)
		{
		case 0:
			lastVal = atk.Gen();
			if (atk.IsFinished())
				state = 1;
			break;
		case 1:
			break;
		case 2:
			lastVal = dec.Gen();
			if (dec.IsFinished())
				state = 3;
			break;
		case 3:
			break;
		}
		return lastVal;
	}

	virtual int IsFinished()
	{
		return state == 3;
	}
};

///////////////////////////////////////////////////////////
// Table lookup envelope generator. The entire envelope
// is calculated once and stored in a lookup table. 
// This is more efficient when the envelope rarely changes.
// Note that this derives directly from GenUnit and does
// not include the Release method of EnvGenUnit. 
///////////////////////////////////////////////////////////
class EnvGenTable : public GenUnit
{
private:
	AmpValue *egTable;
	bsInt32 count;
	bsInt32 index;

public:
	EnvGenTable()
	{
		egTable = NULL;
		count = 0;
		index = 0;
	}

	~EnvGenTable()
	{
		delete egTable;
	}

	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	virtual AmpValue Gen()
	{
		if (index < count)
			return egTable[index++];
		return egTable[count];
	}

	virtual int IsFinished()
	{
		return index >= count;
	}

	// segs, start, [rate, level]*
	virtual void Init(int n, float *v)
	{
		if (n >= 2)
		{
			int segs = (int) *v++;
			if (segs < 1)
				segs = 1;
			AmpValue start = AmpValue(*v++);
			FrqValue *rts  = new FrqValue[segs];
			AmpValue *amps = new AmpValue[segs];
			n -= 2;
			int i = 0;
			while (n >= 2)
			{
				rts[i]  = FrqValue(*v++);
				amps[i] = AmpValue(*v++);
				n -= 2;
				i++;
			}

			InitSegs(segs, start, rts, amps, NULL);
			delete rts;
			delete amps;
		}
	}

	virtual void Reset(float initPhs = 0)
	{
		if (initPhs >= 0)
			index = (bsInt32) (initPhs * synthParams.sampleRate);
	}

	// segs  -> number of segments
	// start -> starting value
	// len[] -> array of segment lengths in seconds
	// amp[] -> array of levels at end of segment
	// typ[] -> array of segment types, 1=lin, 2=exp, 3=log, 4=flat, if NULL, all segs are linear
	void InitSegs(int segs, AmpValue start, FrqValue *rt, AmpValue *amp, EGSegType *typ)
	{
		if (egTable)
			delete egTable;

		FrqValue dcount = 0;
		int segn;
		for (segn = 0; segn < segs; segn++)
			dcount += rt[segn];

		count = (long) ((synthParams.sampleRate * dcount) + 0.5);
		egTable = new AmpValue[count+1];

		FrqValue ndxf = 0;
		AmpValue vbeg = start;
		AmpValue vend = 0;

		EnvSeg *segp;
		EnvSegLin egsLin;
		EnvSegExp egsExp;
		EnvSegLog egsLog;
		EnvSeg    egsSus;

		for (segn = 0; segn < segs; segn++)
		{
			vend = amp[segn];
			if (vend == vbeg)
				segp = &egsSus;
			else if (typ == NULL)
				segp = &egsLin;
			else switch(typ[segn])
			{
			case linSeg:
				segp = &egsLin;
				break;
			case expSeg:
				segp = &egsExp;
				break;
			case logSeg:
				segp = &egsLog;
				break;
			default:
				segp = &egsSus;
				break;
			}
			segp->InitSeg(rt[segn], vbeg, vend);
			FrqValue seglen = synthParams.sampleRate * rt[segn];
			FrqValue segend = ndxf + seglen;
			while (ndxf < segend)
			{
				egTable[(int)ndxf] = segp->Gen();
				ndxf += 1;
			}
			vbeg = vend;
		}
		index = (int)ndxf;
		while (index <= count)
			egTable[index++] = vend;

		index = 0;
	}
};
#endif
