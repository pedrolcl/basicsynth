
///////////////////////////////////////////////////////////
// BasicSynth - Sequence file parser 
//
// See SequenceFile.cpp for explanation
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////

#ifndef _SEQUENCEFILE_H_
#define _SEQUENCEFILE_H_

#define SEQ_MAX_LINE 2048
#define SEQ_MAX_ARG  256
#define SEQ_PARAM_GROW 10

class SeqFileMap : public SynthList<SeqFileMap>
{
public:
	int inum;
	int maxent;
	int entries;
	int *map;

	SeqFileMap()
	{
		inum = -1;
		maxent = 0;
		entries = -1;
		map = 0;
	}

	~SeqFileMap()
	{
		delete map;
	}

	void AddEntry(int argn, int mapn)
	{
		if (argn >= maxent)
		{
			int *newmap = new int[maxent+SEQ_PARAM_GROW];
			if (newmap == NULL)
				return;
			if (maxent)
			{
				memcpy(newmap, map, maxent*sizeof(int));
				delete map;
			}
			memset(&newmap[maxent], 0, SEQ_PARAM_GROW*sizeof(int));
			map = newmap;
			maxent += 10;
		}
		map[argn] = mapn;
		if (argn > entries)
			entries = argn;
	}

	int MapParam(int n)
	{
		if (n <= entries)
			return map[n];
		return n;
	}
};

class SequenceFile
{
private:
	InstrManager *inMgr;
	Sequencer *seq;
	SeqFileMap *map;
	SeqFileMap *curmap;
	int error;
	int lineno;
	bsString errlin;
	bsInt32 lastID;
	FileReadBuf fp;

	char *NextParam(char *pin, char *pout);
	int Comment(char *line);
	int ReadLine(char *buf);


public:
	SequenceFile()
	{
		inMgr = NULL;
		seq = NULL;
		map = NULL;
		error = 0;
		lineno = 0;
		lastID = 0;
		map = NULL;
		curmap = NULL;
	}

	~SequenceFile()
	{
		SeqFileMap *p;
		while ((p = map) != NULL)
		{
			map = p->next;
			delete p;
		}
	}

	// Initialize - the InstrManager instance is used to locate the
	// event factory by instrument id. The sequencer is called to
	// store each event as it is parsed.
	void Init(InstrManager *im, Sequencer *s)
	{
		inMgr = im;
		seq = s;
	}

	int ParseMem(char *linbuf);
	int LoadFile(const char *fileName);

	int GetError(bsString& buf)
	{
		if (!error)
			return 0;
		buf = errlin;
		return lineno;
	}
};

#endif
