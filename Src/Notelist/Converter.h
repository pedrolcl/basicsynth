//////////////////////////////////////////////////////////////////////
// Notelist.h
// Converter.h: interface for the nlConverter class.
// This is the base class that implements output to the BasicSynth
// sequencer (Text files). For other synthesizers, derive a class and override
// the various Begin* and End* methods.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CONVERTER_H_)
#define _CONVERTER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class nlErrOut
{
public:

	virtual void OutputDebug(char *s)
	{
	}

	virtual void OutputError(char *s)
	{
	}

	virtual void OutputMessage(char *s)
	{
	}
};

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

	nlGenerate gen;
	nlParser parser;

	double sampleRate;
	long evtCount;

#ifdef NL_INCLUDE_JS
	JSRuntime *jsRT;
	JSContext *jsCTX;
	JSObject  *jsNotelist;
#endif

	void MakeEvent(int evtType, double start, double dur, double vol, double pit, int pcount, double *params);

public:
	nlConverter();
	virtual ~nlConverter();

	void SetDebugLevel(int n) {	debugLevel = n; }
	int GetDebugLevel() { return debugLevel; }
	void SetMaxError(int n) { maxError = n; }
	int GetMaxError() { return maxError; }

	virtual int Convert(const char *filename, nlLexIn *in);
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

	virtual void DebugNotify(int n, char *s)
	{
		if (debugLevel >= n && eout)
			eout->OutputDebug(s);
	}

	virtual void ShowError(char *s)
	{
		DebugNotify(1, s);
		if (eout)
			eout->OutputError(s);
	}

	virtual int  FindInstrNum(char *name);
	virtual void SetParamMap(int in, int pn, int mn, double sc);
	virtual void BeginNotelist();
	virtual void EndNotelist();
	virtual void BeginInstr();
	virtual void BeginVoice(nlVoice *vp);
	virtual void EndVoice(nlVoice *vp);
	virtual void BeginNote(double start, double dur, double vol, double pit, int pcount, double *params);
	virtual void RestartNote(double start, double dur, double vol, double pit, int pcount, double *params);
	virtual void ContinueNote(double start, double vol, double pit, int pcount, double *params);
	virtual void Write(char *txt);

#ifdef NL_INCLUDE_JS
	// support for JavaScript
	void GetCurTime(jsval *vp)
	{
		double d = 0.0;
		if (curVoice != NULL)
			d = curVoice->curTime;
		JS_NewDoubleValue(jsCTX, d, vp);
	}

	void GetCurPitch(jsval *vp)
	{
		double d = 48.0;
		if (curVoice != NULL)
			d = (double) curVoice->lastPit;
		JS_NewDoubleValue(jsCTX, d, vp);
	}

	void GetCurVol(jsval *vp)
	{
		double d = 100;
		if (curVoice != NULL)
			d = curVoice->lastVol;
		JS_NewDoubleValue(jsCTX, d, vp);
	}

	void GetCurRhythm(jsval *vp)
	{
		double d = 0;
		if (curVoice != NULL)
			d = curVoice->lastDur;
		JS_NewDoubleValue(jsCTX, d, vp);
	}

	void GetCurParams(jsval *vp)
	{
		JSObject *obj = JS_NewArrayObject(jsCTX, 0, NULL);
		jsval parm;
		for (int i = 0; i < 16; i++)
		{
			JS_NewDoubleValue(jsCTX, curVoice->lastParam[i], &parm);
			JS_SetElement(jsCTX, obj, i, &parm);
		}
		*vp = OBJECT_TO_JSVAL(obj);
	}

	void SetCurTime(jsval *vp)
	{
		if (curVoice)
			JS_ValueToNumber(jsCTX, *vp, &curVoice->curTime);
	}

	void SetCurPitch(jsval *vp)
	{
		if (curVoice)
			JS_ValueToNumber(jsCTX, *vp, &curVoice->lastPit);
	}

	void SetCurVol(jsval *vp)
	{
		if (curVoice)
			JS_ValueToNumber(jsCTX, *vp, &curVoice->lastVol);
	}

	void SetCurRhythm(jsval *vp)
	{
		if (curVoice)
			JS_ValueToNumber(jsCTX, *vp, &curVoice->lastDur);
	}

	void SetVolMul(jsval *vp)
	{
		if (curVoice)
			JS_ValueToNumber(jsCTX, *vp, &curVoice->volMul);
	}
//	long   instr;
//	long   chnl;
//	long   articType;
//	long   articParam;
//	long   transpose;
//	long   loopCount;
#endif
};

#endif

