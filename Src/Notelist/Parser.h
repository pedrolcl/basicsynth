//////////////////////////////////////////////////////////////////////
// Definition of the notelist nlParser class.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#if !defined(_PARSER_H_)
#define _PARSER_H_

#pragma once

class nlParser
{
private:
	nlConverter *cvtPtr;
	nlGenerate *genPtr;
	nlLex *lexPtr;

	double nlVersion;
	int errCount;
	int errFatal;
	int	theToken;
	int lastOct;
	int octMiddleC;
	char *filename;

	void InvalidToken();

public:
	nlParser();
	virtual ~nlParser();

	void SetConverter(nlConverter *p)
	{
		cvtPtr = p;
	}

	void SetGen(nlGenerate *gen)
	{
		genPtr = gen;
	}

	nlGenerate *GetGen()
	{
		return genPtr;
	}

	void SetLex(nlLex *lexer)
	{
		lexPtr = lexer;
	}

	nlLex *GetLex()
	{
		return lexPtr;
	}

	void SetFile(const char *f)
	{
		delete filename;
		filename = StrMakeCopy(f);
	}

	void GetFile(char *f, int maxlen)
	{
		strncpy(f, filename, maxlen);
	}

	void SetMiddleC(int n)
	{
		octMiddleC = n;
	}

	int Param1(char *where);
	int Error(char *s, int *skiplist);
	int SkipTo(int *skiplist);
	int SkipBlock();

	int Parse();
	int Score();
	int Statement();
	int Version();
	int Option();
	int System();
	int Script();
	int Include();
	int Declare();
	int Set();
	int Voice();
	int Mix();
	int Tempo();
	int MiddleC();
	int MaxParam();
	int Map();
	int Mark();
	int Sync();
	int Write();
	int Sequence();
	int Notelist();
	int Time();
	int Artic();
	int Instrument();
	int Channel();
	int Note();
	int Param();
	int Play();
	int Loop();
	int IfStmt();
	int WhileStmt();
	int Valgroup();
	int Volume();
	int Transpose();
	int Double();
	int Call();
	int Expr();
	int Catenate();
	int Logical();
	int Relation();
	int Term();
	int Factor();
	int Value();
	int InitFn();
	long PitVal(const char *pStr);
	double DurVal(const char *pStr, int& dot);
	int FnArgs(int nMax = 0);
};

#endif // !defined(_PARSER_H_)
