//////////////////////////////////////////////////////////////////////
// The converter class is the container for the interpreter. This is
// the class that aggregates the lexer, parser, generator and other
// relevant bits and pieces, and provides the interace to Notelist. 
// Callers must provide a sequencer, instrument manager, and message
// output class to the converter. After setup, call Convert and then
// Generate. It is not wise to call Generate if Convert returns
// a value > 0. The important stuff happens in MakeEvent.
//
// Copyright 2008, Daniel R. Mitchell
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>
#include "NLConvert.h"

nlConverter::nlConverter()
{
	debugLevel = 0;
	maxError = 5;
	lexin = NULL;
	curVoice = NULL;
	eout = NULL;
	ownLexIn = 0;
	mgr = NULL;
	seq = NULL;
	eng = NULL;
	evtCount = 0;
	sampleRate = 44100.0;
	mapList = 0;
	curMap = 0;
	symbList = NULL;
}

nlConverter::~nlConverter()
{
	if (ownLexIn)
		delete lexin;
	nlSymbol *sym;
	while ((sym = symbList) != NULL)
	{
		symbList = sym->next;
		delete sym;
	}
}

int nlConverter::Convert(const char *filename, nlLexIn *in)
{
	nlLex lexer;
	if (in == NULL)
	{
		lexin = new nlLexFileIn(filename);
		in = lexin;
		ownLexIn = 1;
	}

	lexer.Open(in);

	parser.SetFile(filename);
	parser.SetConverter(this);
	parser.SetGen(&gen);
	parser.SetLex(&lexer);

	int rv = parser.Parse();

	lexer.Close();
	return rv;
}

int nlConverter::Generate()
{
	if (seq == NULL || mgr == NULL)
		return -1;

	gen.SetConverter(this);
	return gen.Run();
}

void nlConverter::Reset()
{
	gen.Reset();
}

void nlConverter::BeginNotelist()
{
	curVoice = NULL;
	curMap = NULL;
}

void nlConverter::EndNotelist()
{
}

void nlConverter::BeginVoice(nlVoice *vp)
{
	if ((curVoice = vp) != NULL)
	{
		curMap = mapList;
		while (curMap)
		{
			if (curMap->Match(vp->instr))
				break;
			curMap = curMap->next;
		}
	}
	else
		curMap = NULL;
}

void nlConverter::EndVoice(nlVoice *vp)
{
	curVoice = NULL;
}

void nlConverter::BeginInstr()
{
	if (curVoice == NULL)
	{
		curMap = NULL;
		return;
	}

	while (curMap)
	{
		if (curMap->Match(curVoice->instr))
			break;
		curMap = curMap->next;
	}
}

void nlConverter::BeginNote(double start, double dur, double amp, double pit, int pcount, double *params)
{
	MakeEvent(SEQEVT_START, start, dur, amp, pit, pcount, params);
}

void nlConverter::RestartNote(double start, double dur, double amp, double pit, int pcount, double *params)
{
	MakeEvent(SEQEVT_RESTART, start, dur, amp, pit, pcount, params);
}

void nlConverter::ContinueNote(double start, double amp, double pit, int pcount, double *params)
{
	MakeEvent(SEQEVT_PARAM, start, 0, amp, pit, pcount, params);
}

void nlConverter::MakeEvent(int evtType, double start, double dur, double amp, double pit, int pcount, double *params)
{
	if (curVoice == NULL)
		return;

	// Insert new event or alter the current note
	SeqEvent *evt = mgr->ManufEvent(curVoice->instr);
	if (evt == NULL)
		return;
	if (evtType == SEQEVT_START)
		evtCount++;
	evt->evid = evtCount;
	evt->type = evtType;
	evt->SetParam(P_INUM, (long) curVoice->instr);
	evt->SetParam(P_CHNL, (long) curVoice->chnl);
	evt->SetParam(P_START, (float) start);
	evt->SetParam(P_DUR, (float) dur);
	if (gen.GetFrequencyMode())
		evt->SetParam(P_FREQ, pit);
	else
		evt->SetParam(P_PITCH, (long) pit);
	if (amp > 0)
	{
		if (gen.GetVoldbMode())
			amp = pow(10, amp / 20.0) / 100000.0;
		else
			amp /= 100.0;
	}
	evt->SetParam(P_VOLUME, (float) amp);

	double val;
	int pn, mn;
	int px = P_VOLUME + 1;
	for (pn = 0; pn < pcount; pn++, px++)
	{
		val = params[pn];
		if (curMap)
		{
			mn = curMap->MapParam(pn);
			val = curMap->ScaleValue(pn, val);
		}
		else
			mn = px;
		evt->SetParam(mn, (float) val);
	}

	for ( ; pn < curVoice->cntParam; pn++, px++)
	{
		val = curVoice->lastParam[pn];
		if (curMap)
		{
			mn = curMap->MapParam(pn);
			val = curMap->ScaleValue(pn, val);
		}
		else
			mn = px;
		evt->SetParam(mn, (float) val);
	}

	seq->AddEvent(evt);

	if (pcount > curVoice->cntParam)
	{
		if (pcount < curVoice->maxParam)
			curVoice->cntParam = pcount;
		else
			curVoice->cntParam = curVoice->maxParam;
	}
}

int nlConverter::FindInstrNum(const char *name)
{
	if (name == NULL || *name == 0)
		return -1;

	InstrMapEntry *ip;
	if ((ip = mgr->FindInstr(name)) != NULL)
		return ip->inum;

	return -1;
}

void nlConverter::SetParamMap(int inum, int pn, int mn, double scl)
{
	nlParamMap *mp = mapList;
	while (mp)
	{
		if (mp->Match(inum))
			break;
		mp = mp->next;
	}

	if (mp == NULL)
	{
		mp = new nlParamMap;
		mp->SetInstr(inum);
		// push_front - so that the new node will be
		// seen first on the next call.
		if (mapList != NULL)
			mapList->InsertBefore(mp);
		mapList = mp;
	}
	mp->AddEntry(pn, mn, (float) scl);
}

nlSymbol *nlConverter::Lookup(const char *name)
{
	nlSymbol *sym = symbList;
	while (sym != NULL)
	{
		if (CompareToken(sym->name, name) == 0)
			return sym;
		sym = sym->next;
	}
	return NULL;
}

nlSymbol *nlConverter::AddSymbol(const char *name)
{
	nlSymbol *sym = new nlSymbol(name);
	sym->next = symbList;
	symbList = sym;
	return sym;
}

void nlConverter::Write(char *txt)
{
	// Output some information for the user and/or to an output file as appropriate
	if (eout)
		eout->OutputMessage(txt);
}
// it appears that strcmpi, strdup, itoa are "deprecated" (pffffttt...)

int CompareToken(const char *s1, const char *s2)
{
	int c1, c2;
	while (*s1 && *s2)
	{
		if ((c1 = *s1++) >= 'a' && c1 <= 'z')
			c1 = 'A' + c1 - 'a';
		if ((c2 = *s2++) >= 'a' && c2 <= 'z')
			c2 = 'A' + c2 - 'a';
		if (c1 != c2)
			return c2 - c1;
	}
	return *s2 - *s1;
}

char *StrMakeCopy(const char *s)
{
	char *snew = NULL;
	if (s)
	{
		if ((snew = new char[strlen(s)+1]) != NULL)
			strcpy(snew, s);
	}
	return snew;
}

char *StrPaste(const char *s1, const char *s2)
{
	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	char *s3 = new char[len1+len2+1];
	if (s3 != NULL)
	{
		strcpy(s3, s1);
		strcpy(s3+len1, s2);
	}
	return s3;
}

char *IntToStr(long val, char *s)
{
	if (val >= 10)
		s = IntToStr(val/10, s);
	*s++ = (val % 10) + '0';
	*s = '\0';
	return s;
}

static char *FltIToStr(double val, char *s, int len)
{
	if (len > 1 && val >= 10.0)
		s = FltIToStr(val/10.0, s, len-1);
	*s++ = (int) fmod(val, 10.0) + '0';
	*s = 0;
	return s;
}

char *FltToStr(double val, char *s, int len)
{
	double fpart = val;
	double ipart = floor(val);
	s = FltIToStr(ipart, s, len);
	int dig = len - (int) strlen(s) - 1;
	*s++ = '.';
	if (dig > 6)
		dig = 6;
	while (dig-- > 0)
	{
		fpart -= ipart;
		fpart *= 10.0;
		ipart = floor(fpart); // 0 <= ipart < 10
		*s++ = ((long)ipart) + '0';
	}
	*s = '\0';
	return s;
}
