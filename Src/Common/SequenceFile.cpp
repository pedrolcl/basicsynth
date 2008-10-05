///////////////////////////////////////////////////////////
// Sequencer file loading routines.
//
// A sequencer file is a line of timed events. Each event line
// in the file must start with the values required by the 
// sequencer event base class. The format of one event line
// in the sequence file is as follows:
//    [+|-|&]inum chnl time duration { param }
// A line beginning with a plus (+) will generate a PARAM event.
// Lines beginning with a minus (-) will generate a STOP event. 
// A line beginning with ampersand will generate a RESTART event.
// All other lines (except as noted below) will produce a START event. 
//
// Fields are separated by one or more spaces and may be either
// a number or a character string. Numbers (other than the inum field)
// may be entered as a real or integer value and may also include
// a leading negative sign. Character strings are enclosed in either
// single or double quote marks. The maximum field length is 256 characters.
// The first four fields are required. The inum field is an integer 
// value that matches an entry in the instrument definition table. 
// The chnl field indicates the output channel, usually a mixer input. 
// The time and duration fields are numbers that specify the event 
// time and duration in seconds. 
//
// The params are optional and specific to the instrument inum. 
// Any number of param fields may be included, up to a maximum line 
// length of 2048 characters. 
//
// A line beginning with equal (=) defines a parameter map. Since
// the number of parameters is potentially large, the parameter map
// allows selecting only those parameters that are essential. The map
// statement has the form:
//   =inum p1 p2 p3...
// Each pn value represents the actual parameter ID value to use for
// parameter number n.
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthString.h>
#include <SynthList.h>
#include <SynthFile.h>
#include <Mixer.h>
#include <WaveFile.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <Instrument.h>
#include <Sequencer.h>
#include <SequenceFile.h>

// scan for the next parameter
// we allow numbers and quoted character strings
// with each parameter separated by white space.
char *SequenceFile::NextParam(char *pin, char *pout)
{
	while (isspace(*pin))
		pin++;

	if (*pin == '\0')
		return NULL;

	error = 0;
	if (*pin == '"' || *pin == '\'') // parse "[^"]*"|'[^']*'
	{
		int quote = *pin++;
		while (*pin)
		{
			if (*pin == quote)
			{
				pin++;
				break;
			}
			*pout++ = *pin++;
		}
	}
	else
	{  // parse [-]?[0-9]+(\.[0-9]*)?
		if (*pin == '-')
			*pout++ = *pin++;
		if (isdigit(*pin))
		{
			do
				*pout++ = *pin++;
			while (isdigit(*pin));
			if (*pin == '.')
			{
				*pout++ = *pin++;
				while (isdigit(*pin))
					*pout++ = *pin++;
			}
		}
		else
		{
			error = *pin;
			return NULL;
		}
	}

	*pout = '\0';
	return pin;
}

// Parse a line from a memory buffer.
int SequenceFile::ParseMem(char *linbuf)
{
	char *parg = linbuf;
	bsInt16 argn = 0;
	bsInt16 argmax;
	int evtype;
	int inum;
	SeqEvent *evt = NULL;
	char argbuf[SEQ_MAX_ARG];

	while (isspace(*parg))
		parg++;
	if (*parg == 0)
		return 0;

	if (*parg == '=')
	{
		// parameter map
		if ((parg = NextParam(++parg, argbuf)) == NULL)
			return -1;
		inum = atoi(argbuf);
		curmap = new SeqFileMap;
		curmap->inum = inum;
		if (map)
			map->Insert(curmap);
		else
			map = curmap;
		curmap->AddEntry(P_INUM, P_INUM);
		curmap->AddEntry(P_CHNL, P_CHNL);
		curmap->AddEntry(P_START, P_START);
		curmap->AddEntry(P_DUR, P_DUR);
		argn = P_XTRA;
		while ((parg = NextParam(parg, argbuf)) != NULL)
		{
			curmap->AddEntry(argn++, atoi(argbuf));
			if (Comment(parg))
				break;
		}
		return 0;
	}
	if (*parg == '+')
	{
		parg++;
		evtype = SEQEVT_PARAM;
	}
	else if (*parg == '-')
	{
		parg++;
		evtype = SEQEVT_STOP;
	}
	else if (*parg == '&')
	{
		parg++;
		evtype = SEQEVT_RESTART;
	}
	else
	{
		lastID++;
		evtype = SEQEVT_START;
	}

	while ((parg = NextParam(parg, argbuf)) != NULL)
	{
		if (argn == 0)
		{
			inum = atoi(argbuf);
			evt = inMgr->ManufEvent(inum);
			if (evt == NULL)
				return -1;
			evt->type = evtype;
			evt->evid = lastID;
			evt->inum = inum;
			argn = P_INUM + 1;
			argmax = evt->MaxParam();
			if (!curmap || curmap->inum != inum)
			{
				curmap = map;
				while (curmap)
				{
					if (curmap->inum == inum)
						break;
					curmap = curmap->next;
				}
			}
		}
		else
		{
			int mn;
			int pn = argn++;
			if (curmap)
				mn = curmap->MapParam(pn);
			else
				mn = pn;
			if (pn <= argmax)
				evt->SetParam(mn, argbuf);
		}
		if (Comment(parg))
			break;
	}
	if (error)
		delete evt;
	else if (evt)
		seq->AddEvent(evt);
	return error;
}

// Lines beginning with '//' or ; are comments.
// Blank lines are also treated as comments.
int SequenceFile::Comment(char *line)
{
	while (isspace(*line))
		line++;
	char ch = *line++;
	if (ch == 0 || ch == ';' || (ch == '/' && *line == '/'))
		return 1;
	return 0;
}

int SequenceFile::ReadLine(char *buf)
{
	int cnt = 0;
	int inch;

	while ((inch = fp.ReadCh()) != -1)
	{
		if (++cnt < SEQ_MAX_LINE)
			*buf++ = (char) inch;
		if (inch == '\n')
			break;
	}
	*buf = 0;
	return cnt;
}

// LoadFile
// open a file and read it one line at a time into memory.
// Each line is passed to the parser in turn.
int SequenceFile::LoadFile(const char *fileName)
{
	if (fp.FileOpen((char*)fileName))
	{
		errlin = "Cannot open file ";
		errlin += fileName;
		return error = -1;
	}

	lastID = 0;
	lineno = 1;
	error = 0;
	char linbuf[SEQ_MAX_LINE];

	while (ReadLine(linbuf) != 0)
	{
		if (!Comment(linbuf))
		{
			if (ParseMem(linbuf))
			{
				errlin = "Error at: ";
				errlin += linbuf;
				break;
			}
		}
		lineno++;
	}

	fp.FileClose();
	return error;
}
