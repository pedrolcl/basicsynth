// Parser.cpp: implementation of the nlParser class.
//
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>
#include "NLConvert.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static int skiptoend[] = { T_ENDSTMT, T_END, -1 };


void nlParser::InvalidToken()
{
	if (cvtPtr->GetDebugLevel() > 0)
	{
		static char errmsg[] = "Invalid token: ";
		size_t len = strlen(lexPtr->Tokbuf()) + sizeof(errmsg) + 1;
		char *ebuf = new char[len];
		if (ebuf)
		{
			strcpy(ebuf, errmsg);
			strcat(ebuf, lexPtr->Tokbuf());
			cvtPtr->DebugNotify(0, ebuf);
			delete ebuf;
		}
	}
}

nlParser::nlParser()
{
	genPtr = NULL;
	lexPtr = NULL;
	errFatal = 0;
	theToken = 0;
	errCount = 0;
	lastOct = 4;
	octMiddleC = 0;
	nlVersion = 3.0;
	filename = StrMakeCopy("<unknown>");
}

nlParser::~nlParser()
{
	delete filename;
}

int nlParser::Parse()
{
	theToken = lexPtr->Next();
	while (theToken != T_ENDOF && !errFatal)
	{
		Statement();
	}
	return errCount;
}

int nlParser::Error(char *s, int *skiplist)
{
	char lnstr[80];
	IntToStr((long)lexPtr->Lineno(), lnstr);
	size_t len = strlen(filename) + strlen(lnstr) + strlen(s) + strlen(lexPtr->Tokbuf());
	char *ebuf = new char[len+10];
	if (ebuf)
	{
		strcpy(ebuf, filename);
		strcat(ebuf, "(");
		strcat(ebuf, lnstr);
		strcat(ebuf, ") : ");
		strcat(ebuf, s);
		strcat(ebuf, ": ");
		strcat(ebuf, lexPtr->Tokbuf());
		cvtPtr->ShowError(ebuf);
		delete ebuf;
	}
	else
		cvtPtr->ShowError(s);
	if (skiplist)
		SkipTo(skiplist);

	if (++errCount > cvtPtr->GetMaxError())
		errFatal = -1;

	return -1;
}

int nlParser::SkipTo(int *skiplist)
{
	while (theToken != T_ENDOF)
	{
		for (int *ptok = skiplist; *ptok != -1; ptok++)
		{
			if (theToken == *ptok)
				return 0;
		}
		if (cvtPtr->GetDebugLevel() > 0)
		{
			char num[80];
			IntToStr((long)theToken, num);
			char *ebuf = new char[strlen(lexPtr->Tokbuf()) + strlen(num) + 20];
			strcpy(ebuf, "Skipping: (");
			strcat(ebuf, num);
			strcat(ebuf, ") ");
			strcat(ebuf, lexPtr->Tokbuf());
			cvtPtr->DebugNotify(0, ebuf);
			delete ebuf;
		}
		theToken = lexPtr->Next();
	}
	return -1;
}

int nlParser::SkipBlock()
{
	int deep = 0;
	// should only be called to skip a begin/end block.
	while (theToken != T_ENDOF)
	{
		if (theToken == T_BEGIN)
			deep++;
		else if (theToken == T_END)
		{
			if (--deep <= 0)
			{
				theToken = lexPtr->Next();
				break;
			}
		}
		theToken = lexPtr->Next();
	}
	return 0;
}

int nlParser::Statement()
{
	switch (theToken)
	{
	case T_VOICE:
		return Voice();
	case T_TEMPO:
		return Tempo();
	case T_WRITE:
		return Write();
	case T_SEQ:
		return Sequence();
	case T_INC:
		return Include();
	case T_INIT:
		return InitFn();
	case T_MIX:
		return MixBlock();
	case T_MIDC:
		return MiddleC();
	case T_MAP:
		return Map();
	/*case T_ART:
		return Artic();*/
	case T_VER:
		return Version();
	case T_MAXPARAM:
		return MaxParam();
	}
	InvalidToken();
	Error("Invalid Statement.", 0);
	errFatal = -1;
	return -1;
}

int nlParser::Version()
{
	theToken = lexPtr->Next();
	if (theToken == T_NUM || theToken == T_STRLIT)
	{
		nlVersion = atof(lexPtr->Tokbuf());
		genPtr->nlVersion = nlVersion;
		theToken = lexPtr->Next();
	}
	else
		return Error("Missing or invalid value for VERSION", skiptoend);

	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return 0;
}

int nlParser::Include()
{
	nlLexIn *lexPtrSave = lexPtr->GetLexIn();
	nlLexFileIn *innlLex;

	char *fnameSave = filename;

	theToken = lexPtr->Next();
	if (theToken != T_STRLIT)
	{
		Error("Missing include file name", 0);
		return -1;
	}

	filename = StrMakeCopy(lexPtr->Tokbuf());
	innlLex = new nlLexFileIn(filename);
	if (innlLex == NULL || innlLex->Open())
	{
		Error("Cannot open include file", 0);
		return -1;
	}

	lexPtr->SetLexIn(lexPtrSave);
	theToken = lexPtr->Next();
	while (theToken != T_ENDOF && !errFatal)
		Statement();
	lexPtr->SetLexIn(lexPtrSave);
	delete innlLex;

	delete filename;
	filename = fnameSave;
	theToken = lexPtr->Next();
	if (theToken = T_ENDSTMT)
		theToken = lexPtr->Next();

	return 0;
}

int nlParser::IncludeNotes()
{
	nlLexIn *lexPtrSave = lexPtr->GetLexIn();
	nlLexFileIn *innlLex;

	char *fnameSave = filename;

	theToken = lexPtr->Next();
	if (theToken != T_STRLIT)
	{
		Error("Missing include file name", 0);
		return -1;
	}

	filename = StrMakeCopy(lexPtr->Tokbuf());
	innlLex = new nlLexFileIn(filename);
	if (innlLex == NULL || innlLex->Open())
	{
		Error("Cannot open include file", 0);
		return -1;
	}

	lexPtr->SetLexIn(innlLex);
	theToken = lexPtr->Next();
	while (theToken != T_ENDOF && !errFatal)
		Note();
	lexPtr->SetLexIn(lexPtrSave);
	delete innlLex;

	delete filename;
	filename = fnameSave;
	theToken = lexPtr->Next();
	if (theToken = T_ENDSTMT)
		theToken = lexPtr->Next();

	return 0;
}


int nlParser::Param1(char *whererr)
{
	char msg[80];
	if (cvtPtr->GetDebugLevel() >= 2)
	{
		static char dbgmsg[] = "Parse: Param1 for ";
		strcpy(msg, dbgmsg);
		strcat(msg, whererr);
		cvtPtr->DebugNotify(2, msg);
	}

	theToken = lexPtr->Next();
	if (Expr())
	{
		static char errmsg[] = "Missing or invalid value for ";
		strcpy(msg, errmsg);
		strcat(msg, whererr);
		Error(msg, skiptoend);
		genPtr->AddNode(T_NUM, 0L);
		return -1;
	}
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return 0;
}

int nlParser::Voice()
{
	cvtPtr->DebugNotify(2, "Parse: VOICE");
	genPtr->AddNode(new nlVoiceNode);
	if (Param1("Voice") != 0)
		return -1;
	return Notelist();
}

int nlParser::Tempo()
{
	cvtPtr->DebugNotify(2, "Parse: TEMPO");
	int err;
	genPtr->AddNode(new nlTempoNode);
	theToken = lexPtr->Next();
	err = Expr();
	if (err == 0)
	{
		if (theToken == T_COMMA)
			theToken = lexPtr->Next();
		err = Expr();
		if (theToken == T_ENDSTMT)
			theToken = lexPtr->Next();
	}
	if (err)
		Error("Missing value(s) for TEMPO", skiptoend);
	return err;
}

// TIME n ;
int nlParser::Time()
{
	cvtPtr->DebugNotify(2, "Parse: TIME");
	genPtr->AddNode(new nlTimeNode);
	return Param1("Time");
}

int nlParser::Mark()
{
	cvtPtr->DebugNotify(2, "Parse: MARK");
	genPtr->AddNode(new nlMarkNode);
	return Param1("Mark");
}

int nlParser::Sync()
{
	cvtPtr->DebugNotify(2, "Parse: SYNC");
	genPtr->AddNode(new nlSyncNode);
	return Param1("Sync");
}

// INIT n {LINE|EXP|LOG|RAND} start , end , steps ;
int nlParser::InitFn()
{
	cvtPtr->DebugNotify(2, "Parse: INIT");
	genPtr->AddNode(new nlInitFnNode);
	theToken = lexPtr->Next();
	if (theToken != T_NUM)
		return Error("Expected function number for INIT", skiptoend);
	long fn = atol(lexPtr->Tokbuf());

	theToken = lexPtr->Next();
	switch (theToken)
	{
	case T_LINE:
	case T_EXP:
	case T_LOG:
	case T_RAND:
		genPtr->AddNode(theToken, fn);
		break;
	default:
		Error("Invalid function type for INIT, use {line|exp|log|rand}", skiptoend);
		return -1;
	}

	theToken = lexPtr->Next();
	if (Expr())
		goto badparam;
	if (theToken != T_COMMA)
		goto badparam;

	theToken = lexPtr->Next();
	if (Expr())
		goto badparam;
	if (theToken != T_COMMA)
		goto badparam;

	theToken = lexPtr->Next();
	if (Expr())
		goto badparam;
	if (theToken != T_ENDSTMT)
		goto badparam;

	genPtr->AddNode(T_ENDSTMT, 0L);
	theToken = lexPtr->Next();
	return 0;

badparam:
	return Error("Invalid parameters for INIT", skiptoend);
}

int nlParser::Instrument()
{
	int err = 0;
	cvtPtr->DebugNotify(2, "Parse: INSTRUMENT");
	genPtr->AddNode(new nlInstnumNode);
	return Param1("inst");
/*	theToken = lexPtr->Next();
	err = Expr();
	if (err)
	{
		Error("Missing or invalid value for INST", skiptoend);
		return -1;
	}
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return 0;*/
}

int nlParser::Mixer()
{
	cvtPtr->DebugNotify(2, "Parse: MIXER");
	genPtr->AddNode(new nlMixNode);
	return Param1("Mixer");
}

int nlParser::MaxParam()
{
	cvtPtr->DebugNotify(2, "Parse: MAXPARAM");
	genPtr->AddNode(new nlMaxParamNode);
	return Param1("Maxparam");
}

int nlParser::MixBlock()
{
	cvtPtr->DebugNotify(2, "Parse: MIXER (block)");
//	genPtr->AddNode(new nlMixBlock);
//	return Param1("MixBlock");
	return -1;
}

int nlParser::Volume()
{
	cvtPtr->DebugNotify(2, "Parse: VOLUME");
	genPtr->AddNode(new nlVolumeNode);
	return Param1("Volume");
}

int nlParser::Transpose()
{
	cvtPtr->DebugNotify(2, "Parse: TRANSPOSE");
	genPtr->AddNode(new nlTransposeNode);
	return Param1("Transpose");
}

int nlParser::Double()
{
	cvtPtr->DebugNotify(2, "Parse: DOUBLE");
	genPtr->AddNode(new nlDoubleNode);

	theToken = lexPtr->Next();
	if (theToken == T_OFF)
	{
		genPtr->AddNode(T_OFF, 0L);
		theToken = lexPtr->Next();
	}
	else
	{
		if (Expr())
			return Error("Missing transposition for DOUBLE", skiptoend);
		if (theToken == T_COMMA)
		{
			theToken = lexPtr->Next();
			genPtr->AddNode(T_COMMA, 0L);
			if (Expr())
				return Error("Missing volume for DOUBLE", skiptoend);
		}
	}
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return 0;

}

int nlParser::Notelist()
{
	cvtPtr->DebugNotify(2, "Parse: NOTELIST");
	int err = 0;
	if (theToken == T_BEGIN)
	{
		genPtr->AddNode(new nlBlockNode);
		theToken = lexPtr->Next();
		while (theToken != T_ENDOF && !errFatal)
		{
			if (theToken == T_END)
			{
				theToken = lexPtr->Next();
				break;
			}
			Note();
		}
		genPtr->AddNode(T_END, 0L);
		return 0;
	}
	return Note();
}

int nlParser::Sequence()
{
	static int skiptok[] = { T_BEGIN, T_END, -1 };
	cvtPtr->DebugNotify(2, "Parse: SEQ");
	theToken = lexPtr->Next();
	if (theToken != T_STRLIT)
	{
		Error("Missing name for sequence", skiptok);
		SkipBlock();
		return -1;
	}

	nlSequence *pNew = genPtr->AddSequence(lexPtr->Tokbuf());
	nlSequence *pOld = genPtr->SetCurSeq(pNew);
	theToken = lexPtr->Next();
	int err = Notelist();
	genPtr->SetCurSeq(pOld);
	if (err == 0)
		return 0;
	if (theToken != T_END)
		SkipBlock();
	return -1;
}

int nlParser::Artic()
{
	cvtPtr->DebugNotify(2, "Parse: ARTIC");

	theToken = lexPtr->Next();
	switch (theToken)
	{
	case T_FIXED:
	case T_PCNT:
	case T_ADD:
	case T_OFF:
		break;
	default:
		return Error("Invalid argument to ARTIC", skiptoend);
	}
	nlScriptNode *p = genPtr->AddNode(new nlArticNode);
	p->SetValue((long)theToken);
	if (nlVersion < 3.0)
		return 0;

	int err = 0;
	theToken = lexPtr->Next();
	if (theToken == T_COMMA)
	{
		theToken = lexPtr->Next();
		if (theToken == T_PARAM)
		{
			genPtr->AddNode(T_PARAM, 0L);
			theToken = lexPtr->Next();
		}
		else
		{
			genPtr->AddNode(T_COMMA, 0L);
			err = Expr();
		}
	}
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

int nlParser::Note()
{
	cvtPtr->DebugNotify(2, "Parse: NOTE");

	int bSus = 0;
	int bAdd = 0;

	switch (theToken)
	{
	case T_TEMPO:
		return Tempo();

	case T_TIME:
		return Time();

	case T_MARK:
		return Mark();

	case T_SYNC:
		return Sync();

	case T_WRITE:
		return Write();

	case T_INSTNUM:
		return Instrument();

	case T_CHNL:
	case T_MIX:
		return Mixer();

	case T_CRESC:
	case T_DIM:
		return Crescendo();
	
	case T_ACCEL:
	case T_RIT:
		return Accelerando();
	
	case T_REP:
		return Repeat();

	case T_LOOP:
		return Loop();

	case T_ENDSTMT:		/* null statement */
		theToken = lexPtr->Next();
		return 0;

	case T_SUBOP:
	case T_SUS:
		bSus = 1;
		theToken = lexPtr->Next();
		break;

	case T_TIE:
	case T_ADDOP:
		bAdd = 1;
		theToken = lexPtr->Next();
		break;

	case T_VOL:
		return Volume();

	case T_XPOSE:
		return Transpose();

	case T_DOUBLE:
		return Double();

	case T_PLAY:
		return Play();

	case T_INC:
		return IncludeNotes();

	case T_INIT:
		return InitFn();

	case T_ART:
		return Artic();

	case T_PARAM:
		return NoteParam();

	case T_MAP:
		return Map();
	}

	nlNoteNode *pNote = new nlNoteNode;
	pNote->SetSus(bSus);
	pNote->SetAdd(bAdd);
	genPtr->AddNode(pNote);

	if (List()) // rhythm
	{
		Error("Invalid note", skiptoend);
		return -1;
	}

	while (theToken == T_COMMA)
	{
		genPtr->AddNode(T_COMMA, 0L);
		theToken = lexPtr->Next();
		if (List())
		{
			Error("Invalid parameter list", skiptoend);
			return -1;
		}
	}

	if (theToken != T_ENDSTMT)
		Error("Missing end of statement added.", 0);

	genPtr->AddNode(T_ENDSTMT, 0L);
	theToken = lexPtr->Next();
	return 0;
}

int nlParser::NoteParam()
{
	cvtPtr->DebugNotify(2, "Parse: PARAM [n,v]");

	nlParamNode *pnode = new nlParamNode;
	genPtr->AddNode(pnode);

	theToken = lexPtr->Next();

	if (Expr() != 0)
		return Error("Missing paramter number.", skiptoend);

	if (theToken == T_COMMA)
		theToken = lexPtr->Next();
	if (Expr() != 0)
		return Error("Missing parameter value.", skiptoend);

	return 0;
}

int nlParser::Play()
{
	cvtPtr->DebugNotify(2, "Parse: PLAY");
	theToken = lexPtr->Next();
	if (theToken != T_STRLIT)
		return Error("Missing name of sequence.", 0);

	nlPlayNode *p = new nlPlayNode(lexPtr->Tokbuf());
	genPtr->AddNode(p);

	theToken = lexPtr->Next();
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return 0;
}

int nlParser::Crescendo()
{
	return Error("Crescendo not implemented", 0);
}

int nlParser::Accelerando()
{
	return Error("Accelerando not implemented", 0);
}

int nlParser::Integral()
{
	if (theToken == T_FROM)
	{
		cvtPtr->DebugNotify(2, "Parse: FROM");

		genPtr->AddNode(T_FROM, 0L);
		theToken = lexPtr->Next();
		if (Expr())
		{
			Error("Missing expression for 'FROM'", skiptoend);
			return -1;
		}
	}

	if (theToken == T_TO)
	{
		cvtPtr->DebugNotify(2, "Parse: TO");

		genPtr->AddNode(T_TO, 0L);
		theToken = lexPtr->Next();
		if (Expr())
		{
			Error("Missing expression for 'TO'", skiptoend);
			return -1;
		}
	}

	if (theToken != T_IN)
	{
		cvtPtr->DebugNotify(2, "Parse: IN");

		genPtr->AddNode(T_IN, 0L);
		theToken = lexPtr->Next();
		if (Expr())
		{
			Error("Missing expression for 'IN'", skiptoend);
			return -1;
		}
	}

	if (theToken != T_ENDSTMT)
		Error("Missing end of statement added.", 0);

	genPtr->AddNode(T_ENDSTMT, 0L);
	theToken = lexPtr->Next();

	return 0;
}

int nlParser::Repeat()
{
	return Error("Repeat not implemented", 0);
}

int nlParser::Loop()
{
	cvtPtr->DebugNotify(2, "Parse: LOOP");

	genPtr->AddNode(new nlLoopNode);
	theToken = lexPtr->Next();
	if (Expr())
		Error("Invalid or missing loop count", 0);
	return Notelist();
}

int nlParser::Write()
{
	cvtPtr->DebugNotify(2, "Parse: WRITE");
	theToken = lexPtr->Next();
	if (theToken != T_STRLIT)
		return Error("Missing text for WRITE", skiptoend);

	nlWriteNode *p = new nlWriteNode;
	p->SetValue(lexPtr->Tokbuf());
	genPtr->AddNode(p);
	if ((theToken = lexPtr->Next()) == T_ENDSTMT)
		theToken = lexPtr->Next();
	return 0;
}

int nlParser::List()
{
	cvtPtr->DebugNotify(2, "Parse: {list}");
	int nEndTok = 0;
	int err = 0;
	if (theToken == T_OBRACE)
		nEndTok = T_CBRACE;
	else if (theToken == T_OBRACK)
		nEndTok = T_CBRACK;
	if (nEndTok != 0)
	{
		long count = 0;
		nlScriptNode *pNode = genPtr->AddNode(theToken, 0L);
		theToken = lexPtr->Next();
		while (theToken != nEndTok)
		{
			if ((err = Expr()) != 0)
			{
				static int skiptok[4] = { T_ENDOF, T_ENDSTMT, T_END, -1 };
				skiptok[0] = nEndTok;
				SkipTo(skiptok);
				break;
			}
			if (theToken == T_COMMA)
			{
				genPtr->AddNode(T_COMMA, 0L);
				theToken = lexPtr->Next();
			}
			count++;
		}
		pNode->SetValue(count);
		genPtr->AddNode(nEndTok, 0L);
		theToken = lexPtr->Next();
	}
	else
	{
		err = Expr();
	}
	return err;
}

int nlParser::Expr()
{
	cvtPtr->DebugNotify(3, "Parse: <expr>");
	genPtr->AddNode(new nlExprNode);
	return CatOp();
}

int nlParser::CatOp()
{
	int nSavTok;
	if (LogOp())
		return -1;
	while (theToken == T_CATOP)
	{
		cvtPtr->DebugNotify(3, "Parse: CATOP");
		nSavTok = theToken;
		theToken = lexPtr->Next();
		if (LogOp())
			return -1;
		genPtr->AddNode(nSavTok, 0L);
	}
	return 0;
}

int nlParser::LogOp()
{
	int nSavTok;
	if (AddOp())
		return -1;
	while (theToken == T_LTOP
	    || theToken == T_LEOP
		|| theToken == T_GTOP
		|| theToken == T_GEOP
		|| theToken == T_NEOP
		|| theToken == T_EQOP)
	{
		cvtPtr->DebugNotify(3, "Parse: LOGOP");
		nSavTok = theToken;
		theToken = lexPtr->Next();
		if (AddOp())
			return -1;
		genPtr->AddNode(nSavTok, 0L);
	}
	return 0;
}

int nlParser::AddOp()
{
	int nSavTok;
	if (MulOp())
		return -1;
	while (theToken == T_ADDOP 
		|| theToken == T_SUBOP)
	{
		cvtPtr->DebugNotify(3, "Parse: ADDOP");
		nSavTok = theToken;
		theToken = lexPtr->Next();
		if (MulOp())
			return -1;
		genPtr->AddNode(nSavTok, 0L);
	}
	return 0;
}

int nlParser::MulOp()
{
	int nSavTok;
	if (Term())
		return -1;
	while (theToken == T_MULOP 
		|| theToken == T_DIVOP 
		|| theToken == T_EXPOP)
	{
		cvtPtr->DebugNotify(3, "Parse: MULOP");
		nSavTok = theToken;
		theToken = lexPtr->Next();
		if (Term())
			return -1;
		genPtr->AddNode(nSavTok, 0L);
	}
	return 0;
}

int nlParser::Term()
{
	nlDurNode *pdur;
	double d;
	int dot;

	switch (theToken)
	{
	case T_OPAREN:
		theToken = lexPtr->Next();
		if (AddOp())
			return -1;
		if (theToken != T_CPAREN)
		{
			genPtr->AddNode(T_CPAREN, 0L);
			return Error("Missing close parenthesis", 0);
		}
		break;
	case T_CPAREN:
		break;
	case T_SUBOP:
		theToken = lexPtr->Next();
		if (Term())
			return -1;
		genPtr->AddNode(T_NEG, 0L);
		return 0;
	case T_NUM:
		genPtr->AddNode(T_NUM, atof(lexPtr->Tokbuf()));
		break;
	case T_PIT:
		genPtr->AddNode(T_PIT, PitVal(lexPtr->Tokbuf()));
		break;
	case T_DUR:
		pdur = new nlDurNode;
		d = DurVal(lexPtr->Tokbuf(), dot);
		pdur->SetValue(d);
		pdur->SetDotted(dot);
		genPtr->AddNode(pdur);
		break;
	case T_STRLIT:
		genPtr->AddNode(T_STRLIT, lexPtr->Tokbuf());
		break;
	case T_COUNT:
		genPtr->AddNode(T_COUNT, 0L);
		break;
	case T_FGEN:
	case T_RAND:
		genPtr->AddNode(theToken, 0L);
		theToken = lexPtr->Next();
		if (theToken == T_OPAREN)
		{
			if (FnArgs(2))
				return -1;
		}
		else
		{
			genPtr->AddNode(T_NUM, 0L);
			genPtr->AddNode(T_NUM, 1L);
		}
		break;
	default:
		return -1;
	}
	theToken = lexPtr->Next();
	return 0;
}

int nlParser::FnArgs(int nMax)
{
	static int skiplist[] = { T_CPAREN, T_ENDSTMT, T_END, -1 };
	if (theToken != T_OPAREN)
		return Error("Missing open parenthesis for function.", skiptoend);
	theToken = lexPtr->Next();
	while (theToken != T_ENDOF)
	{
		if (theToken == T_CPAREN || theToken == T_ENDSTMT)
			break;
		if (nMax <= 0)
			return Error("Too many arguments to function", skiplist);
		if (Expr())
			return Error("Invalid function argument", skiplist);
		if (--nMax > 0)
		{
			if (theToken != T_COMMA)
				return Error("Missing , in function arguments", skiplist);
			theToken = lexPtr->Next();
		}
	}

	if (theToken != T_CPAREN)
		return Error("Missing close parenthesis in function arguments", 0);

	if (nMax > 0)
		return Error("Not enough arguments to function", 0);

	return 0;
}

int nlParser::MiddleC()
{
	int neg = 1;
	theToken = lexPtr->Next();
	if (theToken == T_SUBOP)
	{
		neg = -1;
		theToken = lexPtr->Next();
	}
	if (theToken != T_NUM)
		return Error("Missing value for MIDDLEC", skiptoend);

	octMiddleC = atoi(lexPtr->Tokbuf()) * neg;

	theToken = lexPtr->Next();
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return 0;
}

int nlParser::Map()
{
	nlMapNode *mapNode = new nlMapNode;
	genPtr->AddNode(mapNode);

	theToken = lexPtr->Next();
	if (theToken == T_STRLIT)
	{
		// instrument by name
		genPtr->AddNode(T_STRLIT, lexPtr->Tokbuf());
	}
	else if (theToken == T_NUM)
	{
		// instrument by number
		genPtr->AddNode(T_NUM, atol(lexPtr->Tokbuf()));
	}
	else
		return Error("Expected instrument name or number", skiptoend);

	theToken = lexPtr->Next();
	long paramCount = 0;
	while (theToken != EOF)
	{
		if (Expr())
			return Error("Expected map value", skiptoend);
		paramCount++;
		if (theToken == T_EQ)
		{
			genPtr->AddNode(T_EQ, 0L);
			theToken = lexPtr->Next();
			if (Expr())
				return Error("Missing scale value", skiptoend);
		}
		if (theToken == T_COMMA)
		{
			genPtr->AddNode(T_COMMA, 0L);
			theToken = lexPtr->Next();
		}
		else
			break;
	}
	mapNode->SetValue(paramCount);
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();

	return 0;
}

long nlParser::PitVal(char *pStr)
{
	//                        A   B  C  D  E  F  G 
	static long ltrpch[8] = { 9, 11, 0, 2, 4, 5, 7 };
	int c = toupper(*pStr);
	if (c == 'R')
		return -1;

	long pch = ltrpch[c - 'A'];
	c = *++pStr;
	if (c == '#')
	{
		pch++;
		pStr++;
	}
	else if (c == 'b')
	{
		pch--;
		pStr++;
	}
	else if (c == 'x')
	{
		pch += 2;
		pStr++;
	}
	else if (c == 'd')
	{
		pch -= 2;
		pStr++;
	}

	c = *pStr;
	if (isdigit(*pStr))
		lastOct = atol(pStr);

	return pch + ((lastOct + octMiddleC) * 12);
}

double nlParser::DurVal(char *pStr, int& dot)
{
	double dur = 0.0;
	int c = toupper(*pStr);
	switch (c)
	{
	case 'W':
		dur = 1.0;
		break;
	case 'H':
		dur = 2;
		break;
	case 'Q':
		dur = 4;
		break;
	case 'E':
		dur = 8;
		pStr++; // EI
		break;
	case 'S':
		dur = 16;
		break;
	case 'T':
		dur = 32;
		break;
	case '%':
		pStr++;
		dot = 0;
		return atof(pStr);
	}

	dot = (*++pStr == '.');

	return dur;
}
