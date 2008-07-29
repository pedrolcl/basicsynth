//////////////////////////////////////////////////////////////////
// BasicSynth
//
// WaveFile output functions
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveFile.h>

void WaveFile::SetupWH()
{
	wh.riffId[0] = 'R';
	wh.riffId[1] = 'I';
	wh.riffId[2] = 'F';
	wh.riffId[3] = 'F';
	wh.riffSize = 0;
	wh.waveType[0] = 'W';
	wh.waveType[1] = 'A';
	wh.waveType[2] = 'V';
	wh.waveType[3] = 'E';
	wh.fmtId[0] = 'f';
	wh.fmtId[1] = 'm';
	wh.fmtId[2] = 't';
	wh.fmtId[3] = ' ';
	wh.fmtSize = 16; // TODO: allow other sample sizes, 8, 24, 32 ?
	wh.fmtCode = 1;    // 1 = PCM
	wh.channels = 1;    // 1 = mono, 2 = stereo
	wh.sampleRate = synthParams.isampleRate;
	wh.bits = sizeof(SampleValue) * 8;
	wh.align = (wh.channels * wh.bits) / 8;
	wh.avgbps = (wh.sampleRate * wh.align);
	wh.waveId[0] = 'd';
	wh.waveId[1] = 'a';
	wh.waveId[2] = 't';
	wh.waveId[3] = 'a';
	wh.waveSize = 0;
}


// Open wave output file, 
// fname is file name, channels number of outputs
int WaveFile::OpenWaveFile(char *fname, int chnls)
{
	wfp.FileClose();
	if (AllocBuf(synthParams.isampleRate * bufSecs * chnls, chnls))
		return -3;

	SetupWH();
	sampleNumber = 0;
	sampleTotal = 0;

	wh.channels = chnls;
	wh.align = (wh.channels * wh.bits) / 8;
	wh.avgbps = (wh.sampleRate * wh.align);

	if (wfp.FileOpen(fname))
		return -1;

	if (wfp.FileWrite(&wh, sizeof(wh)) != sizeof(wh))
	{
		wfp.FileClose();
		return -2;
	}

	sampleOOR = 0;
	return 0;
}

// Flush remaining output and close file
int WaveFile::CloseWaveFile()
{
	FlushOutput();

	size_t byteTotal = sampleTotal * sizeof(SampleValue);

	wh.riffSize = byteTotal + sizeof(wh) - 8; // filesize - RIFF chunk
	wh.waveSize = byteTotal;

	int err = 0;
	wfp.FileRewind();
	if (wfp.FileWrite(&wh, sizeof(wh)) != sizeof(wh))
		err = -1;
	wfp.FileClose();
	DeallocBuf();
	return err;
}

// write output to WAVE file
int WaveFile::FlushOutput()
{
	wfp.FileWrite(samples, sizeof(SampleValue)*sampleNumber);
	sampleNumber = 0;
	return 0;
}

// Load a wave file.
// returns: -1 -> file could not be opened
//          -2 -> wrong format
//          -3 -> no memory
//          -4 -> invalid argument
//           0 -> you have valid loaded wave file
int WaveFileIn::LoadWaveFile(const char *fname, bsInt16 id)
{
	if (*fname == 0)
		return -4;

	if (samples)
	{
		delete samples;
		samples = 0;
	}
	if (filename)
	{
		delete filename;
		filename = 0;
	}

	sampleTotal = 0;

	FileReadBuf wfp;
	if (wfp.FileOpen(fname) != 0)
		return -1;

	RiffChunk chunk;
	wfp.FileRead(&chunk, 8);
	if (memcmp(chunk.chunkId, "RIFF", 4) != 0)
	{
		wfp.FileClose();
		return -1;
	}
	long size = chunk.chunkSize;

	wfp.FileRead(&chunk, 4);
	if (memcmp(chunk.chunkId, "WAVE", 4) != 0)
	{
		wfp.FileClose();
		return -1; // "invalid file";
	}
	size -= 4;

	int foundFmt = 0;
	int foundWav = 0;
	
	char *data = NULL;
	int dataSize = 0;
	FmtData fmt;
	// Find the format and data chunks.
	// Note that this requires the format to come first!
	while (!(foundFmt && foundWav) && size > 0) 
	{
		if (wfp.FileRead(&chunk, 8) != 8)
			break;
		size -= 8;
		if (memcmp(chunk.chunkId, "fmt ", 4) == 0 && chunk.chunkSize == 16)
		{
			if (wfp.FileRead(&fmt, 16) != 16)
				break;
			size -= 16;
			if (fmt.fmtCode == 1 
			 && fmt.bits == (sizeof(SampleValue) * 8) 
			 && fmt.sampleRate == synthParams.sampleRate)
			{
				foundFmt = 1;
			}
		}
		else if (memcmp(chunk.chunkId, "data", 4) == 0)
		{
			foundWav = 1;
			dataSize = chunk.chunkSize;
			break;
		}
		else 
		{
			wfp.FileSkip(chunk.chunkSize);
			size -= chunk.chunkSize;
		}
	}

	if (!foundFmt || !foundWav)
	{
		wfp.FileClose();
		return -2;
	}

	filename = new char[strlen(fname)+1];
	if (filename)
		strcpy(filename, fname);
	sampleTotal = dataSize / fmt.align;
	samples = new AmpValue[sampleTotal];
	if (samples == NULL)
	{
		wfp.FileClose();
		sampleTotal = 0;
		return -3;
	}

	SampleValue *in;
	data = new char[fmt.align];
	AmpValue *sp = samples;
	AmpValue scale = (AmpValue) ((1L << (fmt.bits - 1)) - 1);
	AmpValue maxAmp = 1.0 / scale;
	for (size = sampleTotal; size > 0; size--)
	{
		wfp.FileRead(data, fmt.align);
		in = (SampleValue *) data;
		*sp = (AmpValue) in[0] / scale;
		for (bsInt16 n = 1; n < fmt.channels; n++)
			*sp += (AmpValue) in[n] / scale;
		if (*sp > maxAmp)
			maxAmp = *sp;
		sp++;
	}

	wfp.FileClose();
	delete data;

	size = sampleTotal;
	sp = samples;
	while (size > 0)
	{
		*sp++ /= maxAmp;
		size--;
	}

	fileID = id;

	return 0;
}
