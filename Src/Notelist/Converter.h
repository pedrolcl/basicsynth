//////////////////////////////////////////////////////////////////////
// This is the base class that provides an interface to the script
// conversion modules and implements output to the BasicSynth sequencer. 
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#if !defined(_CONVERTER_H_)
#define _CONVERTER_H_

#pragma once

// prototype for the error and debug output callbacks
class nlErrOut
{
public:

	virtual void OutputDebug(const char *s)
	{
	}

	virtual void OutputError(const char *s)
	{
	}

	virtual void OutputMessage(const char *s)
	{
	}
};

// nlParamMap holds a set of parameter mappings for an instrument
class nlParamMap : public SynthList<nlParamMap>
{
public:
	int  maxEnt;
	int  mapSiz; // maximum in the map
	int *mapPtr;   // map of index values
	float *mapScl; // map of scaling values
	int  instr;  // instrument number
	int  growBy;

	nlParamMap()
	{
		growBy = 8;
		instr = -1;
		mapPtr = 0;
		mapScl = 0;
		mapSiz = 0;
		maxEnt = 0;
	}

	~nlParamMap()
	{
		delete mapPtr;
		delete mapScl;
	}

	inline void SetInstr(int n)
	{
		instr = n;
	}

	inline int Match(int n)
	{
		return n == instr;
	}

	void Clear()
	{
		int pn;
		for (pn = 0; pn < mapSiz; pn++)
		{
			mapPtr[pn] = 0;
			mapScl[pn] = 1.0;
		}
		maxEnt = 0;
	}

	void AddEntry(int pn, int mn, float scl)
	{
		if (pn >= mapSiz)
		{
			int *newmap = new int[mapSiz+growBy];
			float *newscl = new float[mapSiz+growBy];
			if (newmap == NULL || newscl == NULL)
				return;
			if (mapSiz)
			{
				memcpy(newmap, mapPtr, mapSiz*sizeof(int));
				delete mapPtr;
				memcpy(newscl, mapScl, mapSiz*sizeof(float));
				delete mapScl;
			}
			memset(&newmap[mapSiz], 0, growBy*sizeof(int));
			memset(&newscl[mapSiz], 0, growBy*sizeof(float));
			mapPtr = newmap;
			mapScl = newscl;
			mapSiz += growBy;
		}
		mapPtr[pn] = mn;
		mapScl[pn] = scl;
		if (pn > maxEnt)
			maxEnt = pn;
	}

	int MapParam(int pn)
	{
		if (pn <= maxEnt)
			return mapPtr[pn];
		return pn + P_VOLUME + 1;
	}

	double ScaleValue(int pn, double val)
	{
		if (pn <= maxEnt)
			return mapScl[pn] * val;
		return val;
	}
};

// the convert class. 
// Invoke Convert() to parse the input file then Generate() to produce the sequence.
// Convert() may be called multiple times to process more than one input file
// for the same sequence.
class nlConverter  
{
protected:
	int debugLevel;
	int maxError;
	int ownLexIn;
	nlVoice *curVoice;
	nlLexIn *lexin;
	nlErrOut *eout;
	nlParamMap *mapList;
	nlParamMap *curMap;

	Sequencer *seq;
	InstrManager *mgr;
	nlScriptEngine *eng;

	nlSymbol *symbList;
	nlGenerate gen;
	nlParser parser;

	double sampleRate;
	long evtCount;

	void MakeEvent(int evtType, double start, double dur, double vol, double pit, int pcount, double *params);

public:
	nlConverter();
	virtual ~nlConverter();

	void SetDebugLevel(int n) {	debugLevel = n; }
	int GetDebugLevel() { return debugLevel; }
	void SetMaxError(int n) { maxError = n; }
	int GetMaxError() { return maxError; }

	virtual int Convert(const char *filename, nlLexIn *in = 0);
	virtual int Generate();
	virtual void Reset();

	virtual void SetErrorCallback(nlErrOut *e)
	{
		eout = e;
	}

	void SetSequencer(Sequencer *sp)
	{
		seq = sp;
	}

	void SetInstrManager(InstrManager *ip)
	{
		mgr = ip;
	}

	void SetSampleRate(double sr)
	{
		sampleRate = sr;
	}

	void SetScriptEngine(nlScriptEngine *ep)
	{
		if ((eng = ep) != NULL)
		{
			eng->SetConverter(this);
			eng->SetGenerater(&gen);
		}
	}

	nlScriptEngine *GetScriptEngine()
	{ 
		return eng;
	}

	virtual void DebugNotify(int n, const char *s)
	{
		if (debugLevel >= n && eout)
			eout->OutputDebug(s);
	}

	virtual void ShowError(const char *s)
	{
		if (eout)
			eout->OutputError(s);
	}

	nlSymbol *Lookup(const char *name);
	nlSymbol *AddSymbol(const char *name);

	virtual int  FindInstrNum(const char *name);
	virtual int  GetParamID(int inum, const char *name);
	virtual void InitParamMap(int inum);
	virtual void SetParamMap(int inum, int pn, int mn, double sc);
	virtual void BeginNotelist();
	virtual void EndNotelist();
	virtual void BeginInstr();
	virtual void BeginVoice(nlVoice *vp);
	virtual void EndVoice(nlVoice *vp);
	virtual void BeginNote(double start, double dur, double vol, double pit, int pcount, double *params);
	virtual void RestartNote(double start, double dur, double vol, double pit, int pcount, double *params);
	virtual void ContinueNote(double start, double vol, double pit, int pcount, double *params);
	virtual void Write(char *txt);

};

#endif

