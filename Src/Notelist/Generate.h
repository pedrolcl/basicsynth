// Generate.h: interface for the nlGenerate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_GENERATE_H_)
#define _GENERATE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class nlIntegratorData
{
public:
	double beginVal;
	double endVal;
	double range;
	double offset;
	double count;
	double curVal;
	int    fnType;
	int    pos;

// For LINE:
// y = a * x + c
// For EXP or LOG:
// y = a * x^b + c
// x = n * 1/d
// b = flatness of curve, exp or log
// For both:
// a = end - start
// c = start

	nlIntegratorData()
	{
		beginVal = 0;
		endVal = 1;
		range = 1.0;
		offset = 0.0;
		curVal = 0.0;
		count = 0.0;
		fnType = T_LINE;
		pos = 1;
	}

	int IsActive()
	{
		return curVal < count;
	}

	double Iterate(double dDur);

	void Init(double dFrom, double dTo, double dIn, int nFn = T_LINE)
	{
		fnType = nFn;
		beginVal = dFrom;
		endVal = dTo;
		range = dTo - dFrom;
		offset = dFrom;
		count = dIn;
		curVal = 0;
		pos = (dTo >= dFrom) ? 1 : 0;
	}
};

enum vType
{
	vtNull = 0,
	vtText,
	vtNum,
	vtReal
};

class nlVarValue
{
public:
	vType  vt;
	union 
	{
		char *txtVal;
		long  lVal;
		double dblVal;
	};

	nlVarValue()
	{
		vt = vtNull;
		txtVal = 0;
	}
	virtual ~nlVarValue()
	{
		ClearValue();
	}
	virtual void ClearValue();
	virtual void ChangeType(vType vnew);
	virtual void SetValue(char *p);
	virtual void SetValue(long n);
	virtual void SetValue(double d);
	virtual void GetValue(char **p);
	virtual void GetValue(long *n);
	virtual void GetValue(double *d);
	virtual void CopyValue(nlVarValue *p);
	virtual int  Compare(nlVarValue *p);
};

struct nlSyncMark
{
	nlVarValue id;
	double t;
	nlSyncMark *next;
};

class nlScriptNode;

class nlNoteData
{
public:
	long nAlloc;
	long nCount;
	long nIndex;
	int nSimul;
	nlVarValue *pValues;
	//char *name;

	nlNoteData()
	{
		nAlloc = 0;
		nCount = 0;
		nIndex = 0;
		nSimul = 0;
		pValues = NULL;
	}

	virtual ~nlNoteData()
	{
		for (long n = 0; n < nAlloc; n++)
			pValues[n].ClearValue();

		delete[] pValues;
	}

	void InitSingle(long n)
	{
		nSimul = 0;
		nCount = 1;
		nIndex = 0;
		if (nAlloc == 0)
		{
			pValues = new nlVarValue[1];
			nAlloc = 1;
		}
		pValues->SetValue(n);
	}

	void InitSingle(double n)
	{
		nSimul = 0;
		nCount = 1;
		nIndex = 0;
		if (nAlloc == 0)
		{
			pValues = new nlVarValue[1];
			nAlloc = 1;
		}
		pValues->SetValue(n);
	}

	nlScriptNode *Exec(nlScriptNode *p);
	int GetNextValue(double *d);
	int GetNextValue(long *n);
};

class nlVoice
{
public:
	nlVoice();
	virtual ~nlVoice();

	nlVoice *next;
	nlGenerate *genPtr;
	int    voiceNum;
	double curTime;
	double volMul;
	double lastPit;
	double lastDur;
	double lastVol;
	double lastArtic;
	double *lastParam;
	double *paramVal;
	long   maxParam;
	long   cntParam;

	long   instr;
	long   chnl;
	long   articType;
	long   articParam;
	long   transpose;
	long   loopCount;
	char   *instname;

	nlNoteData pitch;
	nlNoteData duration;
	nlNoteData volume;
	nlNoteData artic;
	nlNoteData *params;

	void ClearLast()
	{
		for (long n = 0; n < maxParam; n++)
		{
			lastParam[n] = 0.0;
			paramVal[n] = 0.0;
		}
	}

	void SetMaxParam(int n)
	{
		if (n < 0)
			return;
		if (params)
			delete params;
		if (lastParam)
			delete lastParam;
		if (paramVal)
			delete paramVal;
		params = new nlNoteData[n+1];
		lastParam = new double[n+1];
		paramVal = new double[n+1];
		maxParam = n;
		ClearLast();
	}

	void SetInstName(const char *newname)
	{
		if (instname)
			delete instname;
		instname = StrMakeCopy(newname);
	}
};


class nlScriptNode : public nlVarValue
{
protected:
	int token;
	nlScriptNode *next;
	nlGenerate *genPtr;

public:
	nlScriptNode();
	virtual ~nlScriptNode();

	virtual void SetGen(nlGenerate *p) { genPtr = p; };

	virtual nlScriptNode *Exec()
	{
		return next;
	}

	void Append(nlScriptNode *p)
	{
		p->next = this;
	}

	inline void SetToken(int n)
	{
		token = n;
	}

	inline int GetToken()
	{
		return token;
	}

	inline nlScriptNode *GetNext()
	{
		return next;
	}

};

class nlVoiceNode : public nlScriptNode
{
public:
	nlVoiceNode()
	{
		token = T_VOICE;
	}

	virtual nlScriptNode *Exec();
};

class nlBlockNode : public nlScriptNode
{
public:
	nlBlockNode()
	{
		token = T_BEGIN;
	}
	virtual nlScriptNode *Exec();
};

class nlTempoNode : public nlScriptNode
{
public:
	nlTempoNode()
	{
		token = T_TEMPO;
	}

	virtual nlScriptNode *Exec();
};

class nlTimeNode : public nlScriptNode
{
public:
	nlTimeNode()
	{
		token = T_TIME;
	}
	virtual nlScriptNode *Exec();
};

class nlMarkNode : public nlScriptNode
{
public:
	nlMarkNode()
	{
		token = T_MARK;
	}
	virtual nlScriptNode *Exec();
};

class nlSyncNode : public nlScriptNode
{
public:
	nlSyncNode()
	{
		token = T_SYNC;
	}
	virtual nlScriptNode *Exec();
};

class nlInstnumNode : public nlScriptNode
{
public:
	nlInstnumNode()
	{
		token = T_INSTNUM;
	}
	virtual nlScriptNode *Exec();
};

class nlMixNode : public nlScriptNode
{
public:
	nlMixNode()
	{
		token = T_MIX;
	}
	virtual nlScriptNode *Exec();
};


class nlNoteNode : public nlScriptNode
{
private:
	int    bSus;
	int    bAdd;

public:
	nlNoteNode()
	{
		bSus = 0;
		bAdd = 0;
		token = T_NOTE;
	}
	void SetSus(int n)
	{
		bSus = n;
	}
	void SetAdd(int n)
	{
		bAdd = n;
	}

	virtual nlScriptNode *Exec();
};

class nlExprNode : public nlScriptNode
{
public:
	nlExprNode()
	{
		token = T_EXPR;
	}

	virtual nlScriptNode *Exec();
};

class nlDurNode : public nlScriptNode
{
private:
	int isDotted;
public:
	nlDurNode()
	{
		isDotted = 0;
		token = T_DUR;
	}
	void SetDotted(int d) { isDotted = d; }
	virtual void CopyValue(nlVarValue *p);
	virtual void GetValue(long *n);
	virtual void GetValue(double *d);
	void GetBeatValue(double *n)
	{
		nlVarValue::GetValue(n);
	}
};

class nlLoopNode : public nlScriptNode
{
public:
	nlLoopNode()
	{
		token = T_LOOP;
	}
	virtual nlScriptNode *Exec();
};

class nlWriteNode: public nlScriptNode
{
public:
	nlWriteNode()
	{
		token = T_WRITE;
	}
	virtual nlScriptNode *Exec();
};

class nlVolumeNode : public nlScriptNode
{
public:
	nlVolumeNode()
	{
		token = T_VOL;
	}
	virtual nlScriptNode *Exec();
};

class nlTransposeNode : public nlScriptNode
{
public:
	nlTransposeNode()
	{
		token = T_XPOSE;
	}
	virtual nlScriptNode *Exec();
};

class nlPlayNode : public nlScriptNode
{
private:
	char *name;

public:
	nlPlayNode(char *p)
	{
		name = new char[strlen(p)+1];
		strcpy(name, p);
		token = T_PLAY;
	}
	virtual nlScriptNode *Exec();
};

class nlInitFnNode : public nlScriptNode
{
public:
	nlInitFnNode()
	{
		token = T_INIT;
	}
	virtual nlScriptNode *Exec();
};

class nlArticNode : public nlScriptNode
{
public:
	nlArticNode()
	{
		token = T_ART;
	}
	virtual nlScriptNode *Exec();
};

class nlParamNode : public nlScriptNode
{
public:
	nlParamNode()
	{
		token = T_PARAM;
	}
	virtual nlScriptNode *Exec();
};

class nlMapNode : public nlScriptNode
{
public:
	nlMapNode()
	{
		token = T_MAP;
	}
	virtual nlScriptNode *Exec();
};

class nlMaxParamNode : public nlScriptNode
{
public:
	nlMaxParamNode()
	{
		token = T_MAXPARAM;
	}
	virtual nlScriptNode *Exec();
};


class nlSequence
{
	char *name;
	nlScriptNode *iHead;
	nlScriptNode *iTail;
	nlScriptNode *iCur;

	nlSequence *pNext;

public:
	nlSequence(char *name);
	~nlSequence();
	void Play();
	void Append(nlSequence **pList);
	nlScriptNode *AddNode(nlScriptNode *p);
	nlSequence *FindSequence(char *pfind);
};

class nlGenerate  
{
private:
	nlSequence *curSeq;
	nlConverter *cvtPtr;
	nlVoice *curVoice;
	nlVoice *voiceList;
	nlSequence *pMain;

	nlVarValue *vStack;
	nlVarValue *spEnd;
	nlVarValue *spCur;
	nlSyncMark *synclist;

	long maxParam;

	void Clear();

public:
	nlGenerate();
	virtual ~nlGenerate();


	void SetConverter(nlConverter *p) {cvtPtr = p; }
	nlConverter *GetConverter() { return cvtPtr; }
	nlVoice *GetCurVoice() { return curVoice; }
	nlVoice *SetCurVoice(int n);

	void SyncTo(nlVarValue *v);
	void MarkTo(nlVarValue *v);
	void SetMaxParam(long n);

	double nlVersion;
	double beat;
	double secBeat;
	nlIntegratorData *iFnGen[MAXFGEN];

	nlSequence *FindSequence(char *name);
	void Reset();
	int Run();
	void InitStack();
	void PushStack(long n);
	void PushStack(double d);
	void PushStack(char *s);
	void PopStack(long *n);
	void PopStack(double *d);
	void PopStack(char **s);
	void PopStack(nlVarValue *v);
	nlSequence *AddSequence(char *name);
	nlSequence *SetCurSeq(nlSequence *p);
	nlScriptNode *AddNode(nlScriptNode *pn);
	nlScriptNode *AddNode(int token, char *text);
	nlScriptNode *AddNode(int token, long val);
	nlScriptNode *AddNode(int token, short v1, short v2);
	nlScriptNode *AddNode(int token, double val);
};

#endif // !defined(_GENERATE_H_)
