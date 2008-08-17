#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>
#include "NLConvert.h"
#include "ConverterSCO.h"

nlConverterSCO::nlConverterSCO()
{
	fpOutput = stdout;
	sconame = NULL;
}

nlConverterSCO::~nlConverterSCO()
{
	delete sconame;
}

int nlConverterSCO::Convert(char *filename, nlLexIn *in)
{
	sconame = SetExtension(filename, ".sco");
	return nlConverter::Convert(filename, in);
}

void nlConverterSCO::BeginNotelist()
{
	if (sconame != NULL)
	{
		fpOutput = fopen(sconame, "wa");

		if (fpOutput == NULL)
		{
			fpOutput = stdout;
			fprintf(stderr, "Cannot open file: %s\n", sconame);
		}
	}
}

void nlConverterSCO::EndNotelist()
{
	fputs("\ne\n", fpOutput);
	if (fpOutput != stdout)
		fclose(fpOutput);
	fpOutput = NULL;
}

void nlConverterSCO::BeginVoice(nlVoice *vp)
{
	fprintf(fpOutput, "; BeginVoice %d\n", vp->voiceNum);
	curVoice = vp;
}

void nlConverterSCO::EndVoice(nlVoice *vp)
{
	curVoice = NULL;
}

void nlConverterSCO::BeginNote(double start, double dur, double vol, double pit, int pcount, double *params)
{
	if (curVoice == NULL)
		return;

	// Notelist has middle C at octave 4, CSound is octave 8
	// i inum time dur volume pitch p6 ... pn
	long ipit = (long) pit;
	fprintf(fpOutput, "i%ld %g %g %g %ld%c%02ld",
		curVoice->instr, start, dur, vol, 4 + (ipit / 12), '.', (ipit % 12));
	int pn;
	double val;
	for (pn = 0; pn < pcount; pn++)
	{
		val = params[pn];
		if (curMap)
			val = curMap->ScaleValue(pn, val);
		fprintf(fpOutput, " %g", val);
	}

	fputc('\n', fpOutput);
}

void nlConverterSCO::ContinueNote(double start, double vol, double pit, int pcount, double *params)
{
	// how do we do this in CSound?
}

void nlConverterSCO::Write(char *txt)
{
	fputs(txt, fpOutput);
	fputc('\n', fpOutput);
}

char *nlConverterSCO::SetExtension(char *name, char *ext)
{
	size_t len = strlen(name) + strlen(ext) + 1;
	char *newname = new char[len];
	if (newname == NULL)
		return NULL;

	strcpy(newname, name);

	char *lastp = strrchr(newname, '\\');
	if (lastp == NULL)
		lastp = strrchr(newname, '/');
	if (lastp == NULL)
		lastp = newname;

	char *dotp = strrchr(lastp, '.');
	if (dotp != NULL)
		strcpy(dotp, ext);
	else
		strcat(newname, ext);
	return newname;
}
