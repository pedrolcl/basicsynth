
///////////////////////////////////////////////////////////
// Sequence file parser 
//
// Each line in the file has the form:
//   [+|-|&]inum start duration [parameters]
//
// The inum field is used to locate the event factory for the
// instrument. Parameters are then passed to the event for
// storage. The start and duration fields are defined by
// the base class and must be present. All other parameters
// are optional and defined by the instrument.
//
// The events do not need to be sorted by start time if
// the caller invokes Sequencer::Sort() after the file
// is loaded. If the caller knows the file is sorted, the
// call to Sort can be skipped.
//
// Events can be read from a file, but can also be passed
// directly to the ParseMem function as a character string.
// This allows on-the-fly generation of a sequence for 
// algorithmic composition, interactive music programs,
// parsing directly from an editor buffer, etc.
//
// TODO: query the instrument for the number of parameters
// so that we can report errors of too-few and too-many values.
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
