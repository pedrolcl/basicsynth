//////////////////////////////////////////////////////////////////////
// This is the lexical scanner (lexer) for Notelist
// The nlLex class relies on a contained class derived from nlLexIn
// to manage character-by-character input.
//
// N.B. - this is not yet UTF-8 aware. It might work for extended chars
// in quoted strings, but probably not for much of anything else.
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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define TOKBUFSIZ 2048
nlLex::nlLex()
{
	cTokbuf = new char[TOKBUFSIZ];
	theToken = -1;
	nLineno = 0;
}

nlLex::~nlLex()
{
	delete cTokbuf;
}

int nlLex::Open(nlLexIn *p)
{
	in = p;
	nLineno = 1;
	return p->Open();
}

void nlLex::Close()
{
	if (in)
		in->Close();
}

int nlLex::Next()
{
	int	inch;
	int insv;
	char *bp = cTokbuf;
	char *be = &cTokbuf[TOKBUFSIZ-1];

	// skip comments and white space
	while (1)
	{
		inch = in->Getc();
		if (inch == EOF)
			return T_ENDOF;

		if (inch == '!' || inch == '\'')
		{
			// comment to EOL
			while ((inch = in->Getc()) != EOF)
			{
				if (inch == '\n')
				{
					nLineno++;
					break;
				}
			}
		}
		else if (strchr(" \t\r\n", inch) != NULL)
		{
			if (inch == '\n')
				nLineno++;
		}
		else if (inch != 0)
			break;
	}

	insv = inch;
	*bp++ = inch;
	*bp = '\0';

	// look for terminal
	switch (inch)
	{
	case '"':
		bp = cTokbuf;
		while ((inch = in->Getc()) != '"' )
		{
			if (inch == EOF)
				return T_ENDOF;
			if ( inch == '\\' )	// quoted
			{
				inch = in->Getc();
				// slash at EOL means keep going...
				if (inch == '\r')
				{
					int nl = in->Getc();
					if (nl != '\n')
						in->Ungetc(nl);
					inch = nl;
				}
				if (inch == '\n')
					continue;
				if (inch == 'n')
					inch = '\n';
				else if (inch == 'r')
					inch = '\r';
				else if (inch == 't')
					inch = '\t';
				// othwerise, use as is
			}
			if (inch && bp < be)
				*bp++ = inch;
		}
		*bp = '\0';
		return T_STRLIT;
	case ';':
	case ',':
	case '(':
	case ')':
	case '{':
	case '}':
	case '[':
	case ']':
	case '-':
	case '+':
	case '*':
	case '/':
	case '^':
	case '&':
	case '|':
	case '~':
		return theToken = inch;
	case ':':
		if ((inch = in->Getc()) == ':')
			return T_CATOP;
		in->Ungetc(inch);
		return T_COL;
	case '<':
		if ((inch = in->Getc()) == '>')
		{
			*bp++ = inch;
			theToken = T_NEOP;
		}
		else if (inch == '=')
		{
			*bp++ = inch;
			theToken = T_LEOP;
		}
		else
		{
			in->Ungetc(inch);
			theToken = T_LTOP;
		}
		*bp = '\0';
		return theToken;
	case '>':
		if ((inch = in->Getc()) == '=')
		{
			*bp++ = inch;
			theToken = T_GEOP;
		}
		else
		{
			in->Ungetc(inch);
			theToken = T_GTOP;
		}
		*bp = '\0';
		return theToken;
	case '=':
		if ((inch = in->Getc()) == '=')
		{
			*bp++ = inch;
			theToken = T_EQOP;
		}
		else
		{
			in->Ungetc(inch);
			theToken = T_EQ;
		}
		*bp = '\0';
		return theToken;
	}

	inch = in->Getc();
	// read the rest of the token. Note: characters in the following string
	// are ordered from most common to least common (more or less)
	while (inch != EOF && strchr(" \t\n\r;,(){}[]+-*/^&|=<>\"'!:", inch) == NULL)
	{
		if (bp < be && inch)
			*bp++ = inch;
		inch = in->Getc();
	}
	*bp = '\0';

	// save the terminal character for the next call to lex.
	if (inch != EOF)
		in->Ungetc(inch);

	bp = cTokbuf;
	inch = insv;

	// Now, determine what it is we read. Check for a pitch..
	if ( (inch >= 'A' && inch <= 'G') || (inch >= 'a' && inch <= 'g') )
	{
		inch = ((int)*++bp) & 0xFF;
		if (inch == '\0')
			return T_PIT;
		if (strchr("s#xbdn", inch) != NULL)
			inch = ((int)*++bp) & 0xFF;
		while (isdigit(inch))
			inch = ((int)*++bp) & 0xFF;
		if (inch == '\0')
			return T_PIT;
		// not a pitch, backup to start of buffer
		bp = cTokbuf;
		inch = insv;
	}

	// Or a rhythm...
	if (inch == '%')
	{
		while ((inch = ((int)*++bp)&0xFF) != '\0')
		{
			if (!isdigit(inch) && inch != '.')
				return -1;
		}
		return T_DUR;
	}
	// or a rhythm letter...
	if (strchr("whqestWHQEST", inch) != NULL)
	{
		if (*++bp == '\0')
			return T_DUR;
		if ( (inch == 'e' || inch == 'E') 
		  && (*bp == 'i' || *bp == 'I') )	/* eighth */
			bp++;
		if (*bp == '.')
			bp++;
		if (*bp == '\0')
			return T_DUR;
		bp = cTokbuf;
		inch = insv;
	}
	// or a number.
	else if (isdigit(inch) || inch == '.')
	{
		while ((inch = ((int)*++bp)&0xFF) != '\0')
		{
			if (!isdigit(inch) && inch != '.')
				return -1;
		}
		return T_NUM;
	}

	// check keywords...
	switch (toupper(inch))
	{
	case 'A':
		if (CompareToken(cTokbuf, "ARTIC") == 0)
			return T_ART;
		if (CompareToken(cTokbuf, "AND") == 0)
			return T_AND;
		if (CompareToken(cTokbuf, "ADD") == 0)
			return T_ADD;
		break;
	case 'B':
		if (CompareToken(cTokbuf, "BEGIN") == 0)
			return T_BEGIN;
		break;
	case 'C':
		if (CompareToken(cTokbuf, "CHANNEL") == 0)
			return T_CHNL;
		if (CompareToken(cTokbuf, "CHNL") == 0)
			return T_CHNL;
		if (CompareToken(cTokbuf, "COUNT") == 0)
			return T_COUNT;
		if (CompareToken(cTokbuf, "CALL") == 0)
			return T_CALL;
		if (CompareToken(cTokbuf, "CURPIT") == 0)
			return T_CURPIT;
		if (CompareToken(cTokbuf, "CURDUR") == 0)
			return T_CURDUR;
		if (CompareToken(cTokbuf, "CURVOL") == 0)
			return T_CURVOL;
		if (CompareToken(cTokbuf, "CURTIME") == 0)
			return T_CURTIME;
		break;
	case 'D':
		if (CompareToken(cTokbuf, "DOUBLE") == 0)
			return T_DOUBLE;
		if (CompareToken(cTokbuf, "DO") == 0)
			return T_DO;
		if (CompareToken(cTokbuf, "VAR") == 0)
			return T_DECLARE;
		break;
	case 'E':
		if (CompareToken(cTokbuf, "END") == 0)
			return T_END;
		if (CompareToken(cTokbuf, "EXP") == 0)
			return T_EXP;
		if (CompareToken(cTokbuf, "EVAL") == 0)
			return T_EVAL;
		if (CompareToken(cTokbuf, "ELSE") == 0)
			return T_ELSE;
		break;
	case 'F':
		if (CompareToken(cTokbuf, "FIXED") == 0)
			return T_FIXED;
		if (CompareToken(cTokbuf, "FGEN") == 0)
			return T_FGEN;
		if (CompareToken(cTokbuf, "FREQUENCY") == 0)
			return T_FREQ;
		break;
	case 'I':
		if (CompareToken(cTokbuf, "INSTR") == 0)
			return T_INSTNUM;
		if (CompareToken(cTokbuf, "INSTRUMENT") == 0)
			return T_INSTNUM;
		if (CompareToken(cTokbuf, "INIT") == 0)
			return T_INIT;
		if (CompareToken(cTokbuf, "INCLUDE") == 0)
			return T_INC;
		if (CompareToken(cTokbuf, "IF") == 0)
			return T_IF;
		break;
	case 'L':
		if (CompareToken(cTokbuf, "LOOP") == 0)
			return T_LOOP;
		if (CompareToken(cTokbuf, "LINE") == 0)
			return T_LINE;
		if (CompareToken(cTokbuf, "LOG") == 0)
			return T_LOG;
		break;
	case 'M':
		if (CompareToken(cTokbuf, "MARK") == 0)
			return T_MARK;
		if (CompareToken(cTokbuf, "MAP") == 0)
			return T_MAP;
		if (CompareToken(cTokbuf, "MAXPARAM") == 0)
			return T_MAXPARAM;
		if (CompareToken(cTokbuf, "MIXER") == 0)
			return T_MIX;
		if (CompareToken(cTokbuf, "MIDDLEC") == 0)
			return T_MIDC;
		break;
	case 'N':
		if (CompareToken(cTokbuf, "NOT") == 0)
			return T_NOT;
		if (CompareToken(cTokbuf, "NOTE") == 0)
			return T_NOTE;
		break;
	case 'O':
		if (CompareToken(cTokbuf, "ON") == 0)
			return T_ON;
		if (CompareToken(cTokbuf, "OFF") == 0)
			return T_OFF;
		if (CompareToken(cTokbuf, "OR") == 0)
			return T_OR;
		if (CompareToken(cTokbuf, "OPTION") == 0)
			return T_OPTION;
		break;
	case 'P':
		if (CompareToken(cTokbuf, "PARAM") == 0)
			return T_PARAM;
		if (CompareToken(cTokbuf, "PLAY") == 0)
			return T_PLAY;
		if (CompareToken(cTokbuf, "PERCENT") == 0)
			return T_PCNT;
		break;
	case 'R':
		if (!cTokbuf[1] ) /* rest */
			return T_PIT;
		if (CompareToken(cTokbuf, "RAND") == 0)
			return T_RAND;
		if (CompareToken(cTokbuf, "REPEAT") == 0)
			return T_LOOP;
		break;
	case 'S':
		if (CompareToken(cTokbuf, "SUS") == 0)
			return T_SUS;
		if (CompareToken(cTokbuf, "SUSTAIN") == 0)
			return T_SUS;
		if (CompareToken(cTokbuf, "SEQ") == 0)
			return T_SEQ;
		if (CompareToken(cTokbuf, "SEQUENCE") == 0)
			return T_SEQ;
		if (CompareToken(cTokbuf, "SYNC") == 0)
			return T_SYNC;
		if (CompareToken(cTokbuf, "SET") == 0)
			return T_SET;
		if (CompareToken(cTokbuf, "SCRIPT") == 0)
			return T_SCRIPT;
		if (CompareToken(cTokbuf, "SYS") == 0)
			return T_SYSTEM;
		if (CompareToken(cTokbuf, "SYSTEM") == 0)
			return T_SYSTEM;
		break;
	case 'T':
		if (CompareToken(cTokbuf, "TIE") == 0)
			return T_TIE;
		if (CompareToken(cTokbuf, "TRANSPOSE") == 0)
			return T_XPOSE;
		if (CompareToken(cTokbuf, "TIME") == 0)
			return T_TIME;
		if (CompareToken(cTokbuf, "TEMPO") == 0)
			return T_TEMPO;
		if (CompareToken(cTokbuf, "THEN") == 0)
			return T_THEN;
		break;
	case 'V':
		if (CompareToken(cTokbuf, "VOICE") == 0)
			return T_VOICE;
		if (CompareToken(cTokbuf, "VOL") == 0)
			return T_VOL;
		if (CompareToken(cTokbuf, "VOLUME") == 0)
			return T_VOL;
		if (CompareToken(cTokbuf, "VAR") == 0)
			return T_DECLARE;
		if (CompareToken(cTokbuf, "VARIABLE") == 0)
			return T_DECLARE;
		if (CompareToken(cTokbuf, "VERSION") == 0)
			return T_VER;
		if (CompareToken(cTokbuf, "VER") == 0)
			return T_VER;
		break;
	case 'W':
		if (CompareToken(cTokbuf, "WHILE") == 0)
			return T_WHILE;
		if (CompareToken(cTokbuf, "WRITE") == 0)
			return T_WRITE;
		break;
	}

	// symbol?
	inch = insv;
	if (!isalpha(inch))
		return -1;
	bp = &cTokbuf[1];
	while ((inch = ((int)*bp++)&0xFF) != 0)
	{
		if (!isalpha(inch) && !isdigit(inch))
			return -1;
	}
	return T_VAR;
}

