// Copyright 2008, Daniel R. Mitchell
#include <stdio.h>
#include "Converter.h"

// derived class for CSound score file (.SCO) output
class nlConverterSCO : public nlConverter
{
private:
	FILE *fpOutput;
	char *sconame;

	char *SetExtension(char *name, char *ext);

public:
	nlConverterSCO();
	virtual ~nlConverterSCO();

	virtual int  Convert(char *filename, nlLexIn *in);
	virtual void BeginNotelist();
	virtual void EndNotelist();
	virtual void BeginVoice(nlVoice *vp);
	virtual void EndVoice(nlVoice *vp);
	virtual void BeginNote(double start, double dur, double vol, double pit, int pcount, double *params);
	virtual void ContinueNote(double start, double vol, double pit, int pcount, double *params);
	virtual void Write(char *txt);
};

