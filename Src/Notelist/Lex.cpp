// Lex.cpp: implementation of the nlLex class.
//
//////////////////////////////////////////////////////////////////////

#include "NLConvert.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

nlLex::nlLex()
{
	cTokbuf = new char[1024];
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
	char *bp = cTokbuf;
	char *bpend = cTokbuf + 1023;

	while (1)
	{
		inch = in->Getc();
		if (inch == EOF)
			return T_ENDOF;

		if (inch == '!')
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
		else if (strchr(" \t\r\n", inch) != NULL) // soak up white space
		{
			if (inch == '\n')
				nLineno++;
		}
		else
			break;
	}

	// look for terminal
	if (strchr(";(){}[]=*/-+^&\",<>", inch) != NULL)
	{
		theToken = inch;		/* return it */
		if (theToken == '"')	/* quoted string */
		{
			while ((inch = in->Getc()) != '"' )
			{
				if (inch == EOF)
					return T_ENDOF;

				if ( inch == '\\' )	/* quoted */
				{
					inch = in->Getc();
					if ( inch == '\n' )
						continue;
				}
				if (bp < bpend)
					*bp++ = inch;
			}
			*bp = 0;
		}
		else
		{
			*bp++ = inch;
			if (theToken == '<')
			{
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
			}
			else if (theToken == '>')
			{
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
			}
			else if (theToken == '=')
			{
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
			}
			*bp = '\0';
		}
		return theToken;
	}

	/*
	 * read the rest of the token.
	 * If we hit a terminal character,
	 * it will be saved for the next call to lex.
	 */
	while (inch != EOF && strchr(" \t\n\r;(){}[]=*/-+^&\",!<>", inch) == NULL)
	{
		if (bp < bpend && inch)
			*bp++ = inch;
		inch = in->Getc();
	}
	*bp = 0;
	if (inch != EOF)
		in->Ungetc(inch);

	bp = cTokbuf;
	inch = *bp;

	/*
	 * determine what it is we read
	 * check for a pitch first
	 */
	if ( (inch >= 'A' && inch <= 'G') || (inch >= 'a' && inch <= 'g') )
	{
		inch = *++bp;
		if (inch == '\0' || isdigit(inch))
			return T_PIT;
		if (strchr("#xbdn", inch) != NULL)
			inch = *++bp;
		while (isdigit(inch))
			inch = *++bp;
		if (inch == 0)
			return T_PIT;
		// not a pitch, backup to start of buffer
		bp = cTokbuf;
		inch = *bp;
	}

	if (inch == '%')	/* rhythm value */
		return T_DUR;

	if (strchr("whqestWHQEST", inch) != NULL) // rhythm letter
	{
		bp++;
		if (*bp == '\0')
			return T_DUR;
		if ( (inch == 'e' || inch == 'E') 
		  && (*bp == 'i' || *bp == 'I') )	/* eighth */
			bp++;
		if (*bp == '.')
			bp++;
		if (*bp == '\0')
			return T_DUR;
	}
	if ( isdigit(inch) || inch == '.')
	{
		/* numeric */
		while ((inch = *bp++) != '\0')
		{
			if (!isdigit(inch) && inch != '.')
				return -1;
		}
		return T_NUM;
	}

//	if ( volume(Tokbuf) )			/* dynamic */
//		return (DYNAMIC);

	switch (toupper(inch))
	{
	case 'A':
		if (CompareToken(cTokbuf, "ACCEL") == 0)
			return T_ACCEL;
		if (CompareToken(cTokbuf, "ARTIC") == 0)
			return T_ART;
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
		if (CompareToken(cTokbuf, "CRESC") == 0)
			return T_CRESC;
		if (CompareToken(cTokbuf, "COUNT") == 0)
			return T_COUNT;
		break;
	case 'D':
		if (CompareToken(cTokbuf, "DIM") == 0)
			return T_DIM;
		break;
	case 'E':
		if (CompareToken(cTokbuf, "END") == 0)
			return T_END;
		if (CompareToken(cTokbuf, "EXP") == 0)
			return T_EXP;
		break;
	case 'F':
		if (CompareToken(cTokbuf, "FIXED") == 0)
			return T_FIXED;
		if (CompareToken(cTokbuf, "FROM") == 0)
			return T_FROM;
		if (CompareToken(cTokbuf, "FGEN") == 0)
			return T_FGEN;
		break;
	case 'I':
		if (CompareToken(cTokbuf, "INSTR") == 0)
			return T_INSTNUM;
		if (CompareToken(cTokbuf, "IN") == 0)
			return T_IN;
		if (CompareToken(cTokbuf, "INIT") == 0)
			return T_INIT;
		if (CompareToken(cTokbuf, "INCLUDE") == 0)
			return T_INC;
		break;
	case 'L':
		if (CompareToken(cTokbuf, "LOOP") == 0)
			return T_LOOP;
		if (CompareToken(cTokbuf, "LINE") == 0)
			return T_LINE;
		if (CompareToken(cTokbuf, "LOG") == 0)
			return T_LOG;
		if (CompareToken(cTokbuf, "LOOKUP") == 0)
			return T_LOOKUP;
		break;
	case 'M':
		if (CompareToken(cTokbuf, "MAP") == 0)
			return T_MAP;
		if (CompareToken(cTokbuf, "MARK") == 0)
			return T_MARK;
		if (CompareToken(cTokbuf, "MAXPARAM") == 0)
			return T_MAXPARAM;
		if (CompareToken(cTokbuf, "MIXER") == 0)
			return T_MIX;
		if (CompareToken(cTokbuf, "MIDDLEC") == 0)
			return T_MIDC;
		break;
	case 'O':
		if (CompareToken(cTokbuf, "ON") == 0)
			return T_ON;
		if (CompareToken(cTokbuf, "OFF") == 0)
			return T_OFF;
		break;
	case 'P':
		if (CompareToken(cTokbuf, "PLAY") == 0)
			return T_PLAY;
		if (CompareToken(cTokbuf, "PERCENT") == 0)
			return T_PCNT;
		if (CompareToken(cTokbuf, "PATCH") == 0)
			return T_PATCH;
		if (CompareToken(cTokbuf, "PARAM") == 0)
			return T_PARAM;
		break;
	case 'R':
		if (!cTokbuf[1] ) /* rest */
			return T_PIT;
		if (CompareToken(cTokbuf, "REPEAT") == 0)
			return T_REP;
		if (CompareToken(cTokbuf, "RIT") == 0)
			return T_RIT;
		if (CompareToken(cTokbuf, "RAND") == 0)
			return T_RAND;
		break;
	case 'S':
		if (CompareToken(cTokbuf, "SUS") == 0)
			return T_SUS;
		if (CompareToken(cTokbuf, "SEQ") == 0)
			return T_SEQ;
		if (CompareToken(cTokbuf, "SYNC") == 0)
			return T_SYNC;
		break;
	case 'T':
		if (CompareToken(cTokbuf, "TIME") == 0)
			return T_TIME;
		if (CompareToken(cTokbuf, "TEMPO") == 0)
			return T_TEMPO;
		if (CompareToken(cTokbuf, "TIE") == 0)
			return T_TIE;
		if (CompareToken(cTokbuf, "TO") == 0)
			return T_TO;
		if (CompareToken(cTokbuf, "TRANSPOSE") == 0)
			return T_XPOSE;
		if (CompareToken(cTokbuf, "TABLE") == 0)
			return T_TABLE;
		break;
	case 'V':
		if (CompareToken(cTokbuf, "VOICE") == 0)
			return T_VOICE;
		if (CompareToken(cTokbuf, "VOL") == 0)
			return T_VOL;
		if (CompareToken(cTokbuf, "VER") == 0)
			return T_VER;
		break;
	case 'W':
		if (CompareToken(cTokbuf, "WRITE") == 0)
			return T_WRITE;
		if (CompareToken(cTokbuf, "WAVETABLE") == 0)
			return T_WVTBL;
		break;
	}

	return -1;
}

