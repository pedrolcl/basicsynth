// Parser.h: interface for the nlParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_PARSER_H_)
#define _PARSER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

	int Parse();
	int Version();
	int Include();
	int IncludeNotes();
	int Statement();
	int Param1(char *where);
	int Error(char *s, int *skiplist);
	int SkipTo(int *skiplist);
	int SkipBlock();
	int Voice();
	int MixBlock();
	int Tempo();
	int Time();
	int Mark();
	int Sync();
	int Write();
	int Sequence();
	int Instrument();
	int Mixer();
	int MaxParam();
	int Notelist();
	int Note();
	int NoteParam();
	int Play();
	int Crescendo();
	int Accelerando();
	int Integral();
	int Repeat();;
	int Loop();
	int List();
	int Volume();
	int Transpose();
	int Double();
	int Expr();
	int CatOp();
	int LogOp();
	int AddOp();
	int MulOp();
	int Term();
	int InitFn();
	int Artic();
	int MiddleC();
	int Map();
	long PitVal(char *pStr);
	double DurVal(char *pStr, int& dot);
	int FnArgs(int nMax = 0);
};

#endif // !defined(_PARSER_H_)
