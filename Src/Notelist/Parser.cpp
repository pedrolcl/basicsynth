//////////////////////////////////////////////////////////////////////
// Implementation of the nlParser class. This is a recursive descent
// parser. It receives input tokes from an nlLex object and adds the
// appropriate nodes to the generator object.
//
// Error recovery is primitive, usually we just scan to the next EOS.
// Error messages are formatted inline to avoid dragging in the whole
// of sprintf().
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>
#include "NLConvert.h"

static int skiptoend[] = { T_ENDSTMT, T_END, -1 };

//////////////////////////////////////////////////////////////////////
// Print a message about invalid token. (used for debuggin scripts)
//////////////////////////////////////////////////////////////////////
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
	cvtPtr = NULL;
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

//////////////////////////////////////////////////////////////////////
// Common syntax error printout. We send error messages out through
// the Converter which should have an error reporting object set.
// The skiplist argument indicates how much of the input we should
// skip. This is an array of token IDs terminated with -1. See SkipTo
// below.
//////////////////////////////////////////////////////////////////////

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

int nlParser::Parse()
{
	if (lexPtr == NULL 
	 || genPtr == NULL 
	 || cvtPtr == NULL)
	{
		// you screwed up!
		return -1;
	}

	Score();
	genPtr->AddNode(T_ENDOF, 0L);
	return errCount;
}

// score = statement*
int nlParser::Score()
{
	theToken = lexPtr->Next();
	while (theToken != T_ENDOF && !errFatal)
		Statement();
	return errCount;
}

// statement = include | system | script | write
//           | version | tempo | middlec | initfn
//           | maxparam | map | sequence | voice
//           | var
int nlParser::Statement()
{
	switch (theToken)
	{
	case T_INC:
		return Include();
	case T_SYSTEM:
		return System();
	case T_SCRIPT:
		return Script();
	case T_WRITE:
		return Write();
	case T_VER:
		return Version();
	case T_TEMPO:
		return Tempo();
	case T_MIDC:
		return MiddleC();
	case T_INIT:
		return InitFn();
	case T_MIX:
		return Mix();
	case T_MAP:
		return Map();
	case T_MAXPARAM:
		return MaxParam();
	case T_SET:
		return Set();

	case T_VOICE:
		return Voice();
	case T_SEQ:
		return Sequence();

	case T_DECLARE:
		return Declare();
	}
	InvalidToken();
	Error("Invalid Statement.", 0);
	errFatal = -1;
	return -1;
}

// include ::= 'include' strlit ';'
int nlParser::Include()
{
	int err = 0;
	nlLexIn *lexPtrSave = lexPtr->GetLexIn();
	nlLexFileIn *innlLex;

	char *fnameSave = filename;

	theToken = lexPtr->Next();
	if (theToken == T_STRLIT)
	{
		filename = StrMakeCopy(lexPtr->Tokbuf());
		innlLex = new nlLexFileIn(filename);
		if (innlLex == NULL || !innlLex->Open())
			err = Error("Cannot open include file", 0);
		else
		{
			lexPtr->SetLexIn(innlLex);
			Score();
			lexPtr->SetLexIn(lexPtrSave);
		}
		delete innlLex;
		delete filename;
		filename = fnameSave;

		theToken = lexPtr->Next();
	}
	else
		err = Error("Missing include file name", skiptoend);

	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();

	return err;
}

// system = 'system' strlit ';'
int nlParser::System()
{
	int err = 0;
	theToken = lexPtr->Next();
	if (theToken == T_STRLIT)
	{
		if (system(lexPtr->Tokbuf()) == -1)
			err = Error("Cannot exec", 0);
		theToken = lexPtr->Next();
	}
	else
		err = Error("Missing exec command", skiptoend);

	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();

	return err;
}

// script ::= 'script' strlit ';'
int nlParser::Script()
{
	int err = 0;
	theToken = lexPtr->Next();
	if (theToken == T_STRLIT)
	{
		nlScriptEngine *eng = cvtPtr->GetScriptEngine();
		if (eng != NULL)
			err = eng->LoadScript(lexPtr->Tokbuf());
		else
			err = Error("No script engine was specified.", 0);
		theToken = lexPtr->Next();
	}
	else
		err = Error("Missing script name", skiptoend);

	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();

	return err;
}

// write = 'write' expr ';'
int nlParser::Write()
{
	cvtPtr->DebugNotify(2, "Parse: WRITE");
	genPtr->AddNode(new nlWriteNode);
	return Param1("Write");
}

// declare = 'var' VAR (, VAR)* ';'
int nlParser::Declare()
{
	int err = 0;
	cvtPtr->DebugNotify(2, "Parse: VAR");
	nlSymbol *symb;
	theToken = lexPtr->Next();
	while (theToken == T_VAR)
	{
		symb = cvtPtr->Lookup(lexPtr->Tokbuf());
		if (symb != NULL)
			err = Error("Symbol redefined", skiptoend);
		else
			symb = cvtPtr->AddSymbol(lexPtr->Tokbuf());
		theToken = lexPtr->Next();
		if (theToken == T_COMMA)
			theToken = lexPtr->Next();
	}
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

// set = 'set' VAR '=' expr ';'
int nlParser::Set()
{
	int err = 0;
	cvtPtr->DebugNotify(2, "Parse: SET");
	theToken = lexPtr->Next();
	if (theToken == T_VAR)
	{
		nlSymbol *symb = cvtPtr->Lookup(lexPtr->Tokbuf());
		if (symb != NULL)
		{
			nlSetNode *sp = new nlSetNode;
			sp->SetSymbol(symb);
			genPtr->AddNode(sp);
			theToken = lexPtr->Next();
			if (theToken == T_EQ)
				theToken = lexPtr->Next();
			err = Expr();
		}
		else
			err = Error("Undefined variable", skiptoend);
	}
	else
		err = Error("Expected variable name", skiptoend);
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

// option = 'option' optname ('on'|'off')
// optname = 'frequency' | 'voldb'
int nlParser::Option()
{
	int err = 0;
	int opt = lexPtr->Next();
	if (opt == T_FREQ || opt == T_VOLDB)
	{
		theToken = lexPtr->Next();
		if (theToken == T_ON || theToken == T_OFF)
		{
			nlOptNode *node = new nlOptNode;
			node->SetToken(opt);
			node->SetValue(theToken == T_ON ? 1L : 0L);
			genPtr->AddNode(node);
		}
		else
			err = Error("Expected ON|OFF", skiptoend);
	}
	else
		err = Error("Unknown option", skiptoend);
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

// version = 'version' numstr ';'
// numstr = number | strlit
int nlParser::Version()
{
	int err = 0;
	theToken = lexPtr->Next();
	if (theToken == T_NUM || theToken == T_STRLIT)
	{
		nlVersion = atof(lexPtr->Tokbuf());
		genPtr->SetVersion(nlVersion);
		theToken = lexPtr->Next();
	}
	else
		err = Error("Missing or invalid value for VERSION", skiptoend);
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

// common method for parsing a single parameter: expr ';'
int nlParser::Param1(char *whererr)
{
	int err;
	char msg[80];
	if (cvtPtr->GetDebugLevel() >= 2)
	{
		static char dbgmsg[] = "Parse: Param1 for ";
		strcpy(msg, dbgmsg);
		strcat(msg, whererr);
		cvtPtr->DebugNotify(2, msg);
	}

	theToken = lexPtr->Next();
	if ((err = Expr()) != 0)
	{
		static char errmsg[] = "Missing or invalid value for ";
		strcpy(msg, errmsg);
		strcat(msg, whererr);
		Error(msg, skiptoend);
		genPtr->AddNode(T_NUM, 0L);
	}
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}
// middlec = 'middlec' num ';'
int nlParser::MiddleC()
{
	int err = 0;
	theToken = lexPtr->Next();
	if (theToken == T_NUM)
	{
		octMiddleC = atoi(lexPtr->Tokbuf());
		theToken = lexPtr->Next();
	}
	else
		err = Error("Invalid value for MIDDLEC", skiptoend);
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

// tempo = 'tempo' expr ',' expr ';'
int nlParser::Tempo()
{
	cvtPtr->DebugNotify(2, "Parse: TEMPO");
	int err;
	genPtr->AddNode(new nlTempoNode);
	theToken = lexPtr->Next();
	if ((err = Expr()) == 0)
	{
		if (theToken == T_COMMA)
		{
			theToken = lexPtr->Next();
			err = Expr();
		}
		else
			err = -1;
	}
	if (err)
		Error("Missing value(s) for TEMPO", skiptoend);
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

// For future use...
int nlParser::Mix()
{
	return Error("Mix not implemented", skiptoend);
}

// maxparam = 'maxparam' expr ';'
int nlParser::MaxParam()
{
	cvtPtr->DebugNotify(2, "Parse: MAXPARAM");
	genPtr->AddNode(new nlMaxParamNode);
	return Param1("Maxparam");
}


// map = 'map' id mapval (',' mapval)* ';'
// mapval = expr ('=' expr)?
// id = NUMBER | STRING
int nlParser::Map()
{
	int err = 0;

	nlMapNode *mapNode = new nlMapNode;
	genPtr->AddNode(mapNode);

	theToken = lexPtr->Next();
	// instrument by name or number
	if (theToken == T_STRLIT)
		genPtr->AddNode(T_STRLIT, lexPtr->Tokbuf());
	else if (theToken == T_NUM)
		genPtr->AddNode(T_NUM, atol(lexPtr->Tokbuf()));
	else
		err = Error("Expected instrument id", skiptoend);
	if (err == 0)
	{
		theToken = lexPtr->Next();
		long paramCount = 0;
		while (theToken != EOF)
		{
			if (Expr())
			{
				err = Error("Invalid parameter ID", skiptoend);
				break;
			}
			paramCount++;
			if (theToken == T_COL)
			{
				genPtr->AddNode(T_COL, 0L);
				theToken = lexPtr->Next();
				if (Expr())
				{
					err = Error("Invalid scale value", skiptoend);
					break;
				}
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
	}

	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();

	return err;
}

// sequence = 'seq' id notelist
// id = NUMBER | STRLIT
int nlParser::Sequence()
{
	static int skiptok[] = { T_BEGIN, T_END, -1 };
	cvtPtr->DebugNotify(2, "Parse: SEQ");
	nlSequence *seq;
	theToken = lexPtr->Next();
	if (theToken == T_STRLIT)
		seq = genPtr->AddSequence(lexPtr->Tokbuf());
	else if (theToken == T_NUM)
		seq = genPtr->AddSequence(atoi(lexPtr->Tokbuf()));
	else
	{
		Error("Missing name for sequence", skiptok);
		SkipBlock();
		return -1;
	}

	nlSequence *sav = genPtr->SetCurSeq(seq);
	theToken = lexPtr->Next();
	int err = Notelist();
	genPtr->SetCurSeq(sav);
	return err;
}

// voice = 'voice' NUMBER notelist
int nlParser::Voice()
{
	cvtPtr->DebugNotify(2, "Parse: VOICE");
	theToken = lexPtr->Next();
	if (theToken != T_NUM)
	{
		Error("Missing or invalid voice number", 0);
		errFatal = -1;
		return -1;
	}

	genPtr->AddNode(new nlVoiceNode);
	genPtr->AddNode(T_NUM, atol(lexPtr->Tokbuf()));
	theToken = lexPtr->Next();
	return Notelist();
}


// time = 'time' expr ';'
int nlParser::Time()
{
	cvtPtr->DebugNotify(2, "Parse: TIME");
	genPtr->AddNode(new nlTimeNode);
	return Param1("Time");
}

// mark = 'mark' expr ';'
int nlParser::Mark()
{
	cvtPtr->DebugNotify(2, "Parse: MARK");
	genPtr->AddNode(new nlMarkNode);
	return Param1("Mark");
}

// sync = 'sync' expr ';'
int nlParser::Sync()
{
	cvtPtr->DebugNotify(2, "Parse: SYNC");
	genPtr->AddNode(new nlSyncNode);
	return Param1("Sync");
}

// initfn = 'init' num fntype expr ',' expr ',' expr ';'
// fntype = 'line' | 'exp' | 'log' | 'rand'
int nlParser::InitFn()
{
	cvtPtr->DebugNotify(2, "Parse: INIT");
	genPtr->AddNode(new nlInitFnNode);
	theToken = lexPtr->Next();
	if (theToken != T_NUM)
	{
		Error("Expected function number for INIT", skiptoend);
		if (theToken == T_ENDSTMT)
			theToken = lexPtr->Next();
	}

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
		if (theToken == T_ENDSTMT)
			theToken = lexPtr->Next();
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

	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return 0;

badparam:
	Error("Invalid parameters for INIT", skiptoend);
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return -1;
}

// instrument = 'instr' expr ';'
int nlParser::Instrument()
{
	int err = 0;
	cvtPtr->DebugNotify(2, "Parse: INSTRUMENT");
	genPtr->AddNode(new nlInstnumNode);
	return Param1("inst");
}

// chnl = 'channel' expr ';'
int nlParser::Channel()
{
	cvtPtr->DebugNotify(2, "Parse: CHANNEL");
	genPtr->AddNode(new nlChnlNode);
	return Param1("Channel");
}

// volume = 'vol' expr ';'
int nlParser::Volume()
{
	cvtPtr->DebugNotify(2, "Parse: VOLUME");
	genPtr->AddNode(new nlVolumeNode);
	return Param1("Volume");
}

// transpose = 'transpose' expr ';'
int nlParser::Transpose()
{
	cvtPtr->DebugNotify(2, "Parse: TRANSPOSE");
	genPtr->AddNode(new nlTransposeNode);
	return Param1("Transpose");
}

// double = 'double' dblparam ';'
// dblparam = 'OFF' | expr | expr ',' expr
int nlParser::Double()
{
	int err = 0;
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
			err = Error("Missing transposition for DOUBLE", skiptoend);
		else
		{
			if (theToken == T_COMMA)
			{
				genPtr->AddNode(T_COMMA, 0L);
				theToken = lexPtr->Next();
				if (Expr())
					err = Error("Missing volume for DOUBLE", skiptoend);
			}
		}
	}
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

// call = 'call' expr ';'
int nlParser::Call()
{
	cvtPtr->DebugNotify(2, "Parse: CALL");
	genPtr->AddNode(new nlCallNode);
	return Param1("Call");
}

// notelist = 'begin' note* 'end' | note 
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
			err = Note();
		}
		genPtr->AddNode(T_END, 0L);
		return 0;
	}
	return Note();
}

// artic = 'artic' arttype ';'
// arttype = 'fixed' artexpr
//         | 'pcnt' artexpr
//         | 'add' artexpr
//         | 'off'
// artexpr = 'param' | expr
int nlParser::Artic()
{
	cvtPtr->DebugNotify(2, "Parse: ARTIC");

	int err = 0;
	int hasExpr = 1;
	theToken = lexPtr->Next();
	switch (theToken)
	{
	case T_FIXED:
	case T_PCNT:
	case T_ADD:
		break;
	case T_OFF:
		hasExpr = 0;
		break;
	default:
		Error("Invalid argument to ARTIC", skiptoend);
		if (theToken == T_ENDSTMT)
			theToken = lexPtr->Next();
		return -1;
	}
	nlScriptNode *p = genPtr->AddNode(new nlArticNode);
	p->SetValue((long)theToken);

	theToken = lexPtr->Next();
	if (nlVersion < 3.0)
		genPtr->AddNode(T_PARAM, 0L);
	else if (hasExpr)
	{
		if (theToken == T_COMMA) // backward compatibility
			theToken = lexPtr->Next();
		if (theToken == T_PARAM)
		{
			genPtr->AddNode(T_PARAM, 0L);
			theToken = lexPtr->Next();
		}
		else
		{
			if (Expr())
				err = Error("Invalid parameter to ARTIC", skiptoend);
		}
	}
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

// note = include | system | script | write
//      | tempo | middlec | maxparam | map
//      | inst | vol | chnl | time | mark | sync 
//      | transpose | double | play | initfn
//      | artic | param | loop | sus | tie
//      | notespec | ';'
// sus = 'sus' notespec
// tie = 'tie' notespec
// notespec = valgroup (',' valgroup)* ';'
// valgroup = '{' exprs '}' | '[' exprs ']' | expr
// exprs = expr (',' expr)*
int nlParser::Note()
{
	cvtPtr->DebugNotify(2, "Parse: NOTE");

	int err = 0;
	int bSus = 0;
	int bAdd = 0;

	switch (theToken)
	{
	case T_INC:
		return Include();
	case T_SYSTEM:
		return System();
	case T_SCRIPT:
		return Script();
	case T_WRITE:
		return Write();
	case T_VER:
		return Version();
	case T_TEMPO:
		return Tempo();
	case T_MIDC:
		return MiddleC();
	case T_MAP:
		return Map();
	case T_MAXPARAM:
		return MaxParam();
	case T_DECLARE:
		return Declare();
	case T_SET:
		return Set();

	case T_INSTNUM:
		return Instrument();
	case T_VOL:
		return Volume();
	case T_CHNL:
		return Channel();
	case T_CALL:
		return Call();
	case T_TIME:
		return Time();
	case T_MARK:
		return Mark();
	case T_SYNC:
		return Sync();
	case T_PLAY:
		return Play();
	case T_XPOSE:
		return Transpose();
	case T_DOUBLE:
		return Double();
	case T_ART:
		return Artic();
	case T_PARAM:
		return Param();
	case T_INIT:
		return InitFn();
	case T_LOOP:
		return Loop();
	case T_IF:
		return IfStmt();
	case T_WHILE:
		return WhileStmt();

	case T_NOTE:
		// syntactic sugar...
		theToken = lexPtr->Next();
		break;
	case T_SUS:
		bSus = 1;
		theToken = lexPtr->Next();
		break;
	case T_TIE:
		bAdd = 1;
		theToken = lexPtr->Next();
		break;

	case T_ENDSTMT:
		// null statement
		theToken = lexPtr->Next();
		return 0;
	}

	nlNoteNode *pNote = new nlNoteNode;
	pNote->SetSus(bSus);
	pNote->SetAdd(bAdd);
	genPtr->AddNode(pNote);

	if (Valgroup())
		err = Error("Invalid note", skiptoend);
	else
	{
		while (theToken == T_COMMA)
		{
			genPtr->AddNode(T_COMMA, 0L);
			theToken = lexPtr->Next();
			if (Valgroup())
			{	
				err = Error("Invalid parameter list", skiptoend);
				break;
			}
		}
	}
	genPtr->AddNode(T_ENDSTMT, 0L);
	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();
	return err;
}

// param = 'param' expr ',' expr ';'
int nlParser::Param()
{
	int err = 0;
	cvtPtr->DebugNotify(2, "Parse: PARAM [n,v]");

	nlParamNode *pnode = new nlParamNode;
	genPtr->AddNode(pnode);

	theToken = lexPtr->Next();
	if (Expr() != 0)
		err = Error("Missing or invalid paramter number.", skiptoend);
	else
	{
		if (theToken == T_COMMA)
		{
			theToken = lexPtr->Next();
			err = Expr();
		}
		else
			err = -1;
		if (err)
			Error("Missing or invalid parameter value.", skiptoend);
	}

	if (theToken == T_ENDSTMT)
		theToken = lexPtr->Next();

	return err;
}

// play = 'play' expr ';'
int nlParser::Play()
{
	cvtPtr->DebugNotify(2, "Parse: PLAY");
	genPtr->AddNode(new nlPlayNode);
	return Param1("Play");
}

// loop = 'loop' '(' expr ')' notelist
// Notice that the parentheses around 'expr' are treated
// as optional. This allows backward compatibility with
// an earlier version of Notelist that did not require them.
// However, without the parentheses, there is a conflict
// in the formal grammar in the case of something like:
//     loop 5 -(5);
// As a practical matter the negation op would likely never
// be seen on the first value of a note statement.
int nlParser::Loop()
{
	int err = 0;
	cvtPtr->DebugNotify(2, "Parse: LOOP");

	nlLoopNode *lnode = new nlLoopNode;
	genPtr->AddNode(lnode);

	nlSequence *lseq = new nlSequence;
	lnode->SetSequence(lseq);

	theToken = lexPtr->Next();
	if (theToken == T_OPAREN)
		theToken = lexPtr->Next();
	err = Expr();
	if (theToken == T_CPAREN)
		theToken = lexPtr->Next();
	if (err == 0)
	{
		nlSequence *sav = genPtr->SetCurSeq(lseq);
		err = Notelist();
		genPtr->AddNode(T_ENDOF, 0L);
		genPtr->SetCurSeq(sav);
	}
	else
		Error("Invalid or missing loop count", 0);
	return err;
}

// ifstmt = 'if' expr 'then' notelist ('else' notelist)?
int nlParser::IfStmt()
{
	int err = 0;
	cvtPtr->DebugNotify(2, "Parse: IF...THEN...ELSE");

	nlIfNode *in = new nlIfNode;
	genPtr->AddNode(in);

	nlSequence *iseq = new nlSequence;
	nlSequence *eseq = new nlSequence;
	in->SetIfSequence(iseq);
	in->SetElseSequence(eseq);

	theToken = lexPtr->Next();
	err = Expr();
	if (theToken == T_THEN)
		theToken = lexPtr->Next();
	if (!err)
	{
		nlSequence *sav = genPtr->SetCurSeq(iseq);
		err = Notelist();
		genPtr->AddNode(T_ENDOF, 0L);
		if (theToken == T_ELSE)
		{
			theToken = lexPtr->Next();
			genPtr->SetCurSeq(eseq);
			err = Notelist();
			genPtr->AddNode(T_ENDOF, 0L);
		}
		genPtr->SetCurSeq(sav);
	}
	else
		err = Error("Invalid IF condition", 0);

	return err;
}

// whilestmt = 'while' expr 'do' notelist 
int nlParser::WhileStmt()
{
	int err = 0;
	cvtPtr->DebugNotify(2, "Parse: WHILE");

	nlWhileNode *wn = new nlWhileNode;
	genPtr->AddNode(wn);

	nlSequence *wseq = new nlSequence;
	wn->SetSequence(wseq);

	theToken = lexPtr->Next();
	err = Expr();
	if (theToken == T_DO)
		theToken = lexPtr->Next();
	if (err == 0)
	{
		nlSequence *sav = genPtr->SetCurSeq(wseq);
		err = Notelist();
		genPtr->AddNode(T_ENDOF, 0L);
		genPtr->SetCurSeq(sav);
	}
	else
		err = Error("Invalid WHILE condition", 0);

	return err;
}


// valgroup = '{' exprs '}' | '[' exprs ']' | expr
// exprs    = exprs (',' expr)*
int nlParser::Valgroup()
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
				static int tolist[] = { 0, T_ENDSTMT, T_END, -1 };
				tolist[0] = nEndTok;
				SkipTo(tolist);
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

//  entry point for expression...
int nlParser::Expr()
{
	cvtPtr->DebugNotify(3, "Parse: <expr>");
	genPtr->AddNode(new nlExprNode);
	return Catenate();
}

// catenate = logical ('#' logical)*
int nlParser::Catenate()
{
	int nSavTok;
	if (Logical())
		return -1;
	while (theToken == T_CATOP)
	{
		cvtPtr->DebugNotify(3, "Parse: CATENATE");
		nSavTok = theToken;
		theToken = lexPtr->Next();
		if (Logical())
			return -1;
		genPtr->AddNode(nSavTok, 0L);
	}
	return 0;
}

// logical = relaton (logop relation)*
// logop = '&' | '|'
int nlParser::Logical()
{
	int nSavTok;
	if (Relation())
		return -1;
	while (theToken == T_AND
		|| theToken == T_OR)
	{
		cvtPtr->DebugNotify(3, "Parse: LOGICAL");
		nSavTok = theToken;
		theToken = lexPtr->Next();
		if (Relation())
			return -1;
		genPtr->AddNode(nSavTok, 0L);
	}
	return 0;
}

// relation = term (relop term)*
// relop = '<' | '<=' | '>' | '>=' | '<>' | '=' | '=='
int nlParser::Relation()
{
	int nSavTok;
	if (Term())
		return -1;
	while (theToken == T_LTOP
	    || theToken == T_LEOP
		|| theToken == T_GTOP
		|| theToken == T_GEOP
		|| theToken == T_NEOP
		|| theToken == T_EQ
		|| theToken == T_EQOP)
	{
		if (theToken == T_EQ)
			theToken = T_EQOP;
		cvtPtr->DebugNotify(3, "Parse: RELATION");
		nSavTok = theToken;
		theToken = lexPtr->Next();
		if (Term())
			return -1;
		genPtr->AddNode(nSavTok, 0L);
	}
	return 0;
}

// term = factor (addop factor)*
// addop = '+' | '-'
int nlParser::Term()
{
	int nSavTok;
	if (Factor())
		return -1;
	while (theToken == T_ADDOP 
		|| theToken == T_SUBOP)
	{
		cvtPtr->DebugNotify(3, "Parse: Term");
		nSavTok = theToken;
		theToken = lexPtr->Next();
		if (Factor())
			return -1;
		genPtr->AddNode(nSavTok, 0L);
	}
	return 0;
}

// factor = value (mulop value)*
// mulop = '*' | '/' | '^'
int nlParser::Factor()
{
	int nSavTok;
	if (Value())
		return -1;
	while (theToken == T_MULOP 
		|| theToken == T_DIVOP 
		|| theToken == T_EXPOP)
	{
		cvtPtr->DebugNotify(3, "Parse: FACTOR");
		nSavTok = theToken;
		theToken = lexPtr->Next();
		if (Value())
			return -1;
		genPtr->AddNode(nSavTok, 0L);
	}
	return 0;
}

// value = '(' expr ')' | num | strlit | pitch | dur | 'rand' | fngen | unop value
//       | 'count' | 'time' | 'pitch' | 'duration' | 'vol'
// fngen ::= fn '(' expr ',' expr ')'
// unop ::= '-' | 'eval' | '~' | 'not'
int nlParser::Value()
{
	nlDurNode *pdur;
	double d;
	nlSymbol *symb;
	int dot;
	int sav;

	switch (theToken)
	{
	case T_OPAREN:
		theToken = lexPtr->Next();
		if (Catenate()) // slight optmization, don't start a new Expr node
			return -1;
		if (theToken != T_CPAREN)
		{
			genPtr->AddNode(T_CPAREN, 0L);
			return Error("Missing close parenthesis", 0);
		}
		break;
	case T_SUBOP: // unary - -> negate
		theToken = lexPtr->Next();
		if (Value())
			return -1;
		genPtr->AddNode(T_NEG, 0L);
		return 0;
	case T_NOT:
		theToken = lexPtr->Next();
		if (Value())
			return -1;
		genPtr->AddNode(T_NOT, 0L);
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
	case T_CURPIT:
	case T_CURDUR:
	case T_CURVOL:
	case T_CURTIME:
		genPtr->AddNode(theToken, 0L);
		break;
	case T_FGEN:
	case T_RAND:
		sav = theToken;
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
		genPtr->AddNode(sav, 0L);
		break;
	case T_EVAL:
		theToken = lexPtr->Next();
		if (Value())
			return -1;
		genPtr->AddNode(T_EVAL, 0L);
		break;
	case T_VAR:
		symb = cvtPtr->Lookup(lexPtr->Tokbuf());
		if (symb != NULL)
		{
			nlVarNode *sp = new nlVarNode;
			sp->SetSymbol(symb);
			genPtr->AddNode(sp);
			break;
		}
		return -1;

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
		//if (Expr())
		if (Catenate())
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


long nlParser::PitVal(const char *pStr)
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

double nlParser::DurVal(const char *pStr, int& dot)
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
