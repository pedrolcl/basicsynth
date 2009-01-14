//////////////////////////////////////////////////////////////////
// BasicSynth
//
// WaveFile output functions
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveFile.h>

void WaveFile::SetupWH(int ch)
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
	wh.channels = ch;    // 1 = mono, 2 = stereo
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
// This is only designed for chnls = 1 or 2
int WaveFile::OpenWaveFile(char *fname, int chnls)
{
	wfp.FileClose();
	if (AllocBuf(synthParams.isampleRate * bufSecs * chnls, chnls))
		return -3;

	SetupWH(chnls);
	sampleTotal = 0;

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

// Flush remaining output, update header, and close file
int WaveFile::CloseWaveFile()
{
	FlushOutput();

	bsUint32 byteTotal = sampleTotal * sizeof(SampleValue);

	wh.riffSize = byteTotal + sizeof(wh) - 8; // filesize - RIFF chunk
	wh.waveSize = byteTotal;

	int err = 0;
	wfp.FileRewind();
	// TODO: swap bytes in the header
	if (wfp.FileWrite(&wh, sizeof(wh)) != sizeof(wh))
		err = -1;
	wfp.FileClose();
	DeallocBuf();
	return err;
}

// write output to WAVE file
int WaveFile::FlushOutput()
{
	if (nxtSamp > samples)
	{
		wfp.FileWrite(samples, sizeof(SampleValue)*(nxtSamp - samples));
		nxtSamp = samples;
	}
	return 0;
}

// Load a wave file.
// returns: -1 -> file could not be opened
//          -2 -> wrong format
//          -3 -> no memory
//          -4 -> invalid argument
//           0 -> you have valid loaded wave file
// This code only accepts files that have a frame size of 16 bits.
// Although possible to allow 32 bit files, this will not always work
// since there is variation in how high-resolution formats are encoded. 
// see: http://www.microsoft.com/whdc/device/audio/multichaud.mspx
// Also, this code only sums the first two channels found in the file.
// For some multi-channel formats, it would be possible to merge
// all channels, but for most, it makes no sense.
#ifdef BS_BIG_ENDIAN
SampleValue SwapSample(SampleValue x)
{
	return ((x>>8)&0xFF)|((x&0xFF)<< 8); 
}
static int ReadChunk(FileReadBuf& wfp, RiffChunk& chunk)
{
	if (wfp.FileRead(chunk.chunkId, 4) != 4)
		return 0;
	chunk.chunkSize = wfp.ReadCh() 
	               | (wfp.ReadCh() << 8)
                   | (wfp.ReadCh() << 16)
				   | (wfp.ReadCh() << 24);
	return 1;
}
static int ReadFmt(FileReadBuf& wfp, FmtData& fmt)
{
	fmt.fmtCode = wfp.ReadCh() | (wfp.ReadCh() << 8);
	fmt.channels = wfp.ReadCh() | (wfp.ReadCh() << 8);
	fmt.sampleRate = wfp.ReadCh() | (wfp.ReadCh() << 8) | (wfp.ReadCh() << 16) | (wfp.ReadCh() << 24);
	fmt.avgbps = wfp.ReadCh() | (wfp.ReadCh() << 8) | (wfp.ReadCh() << 16) | (wfp.ReadCh() << 24);
	fmt.align = wfp.ReadCh() | (wfp.ReadCh() << 8);
	fmt.bits = wfp.ReadCh() | (wfp.ReadCh() << 8);
	return fmt.bits >= 0;
}
#else
#define ReadChunk(wfp,chunk) (wfp.FileRead(&chunk, 8) == 8)
#define ReadFmt(wfp,fmt) (wfp.FileRead(&fmt, 16) == 16)
#endif


int WaveFileIn::LoadWaveFile(const char *fname, bsInt16 id)
{
	if (fname == 0 || *fname == 0)
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

	bsString path;
	if (!synthParams.FindOnPath(path, fname))
		return -1;

	FileReadBuf wfp;
	if (wfp.FileOpen(path) != 0)
		return -1;

	RiffChunk chunk;
	if (!ReadChunk(wfp, chunk)
	 || memcmp(chunk.chunkId, "RIFF", 4) != 0)
	{
		wfp.FileClose();
		return -2;
	}
	long fileSize = chunk.chunkSize;
	long wavePos = 0;

	wfp.FileRead(chunk.chunkId, 4);
	if (memcmp(chunk.chunkId, "WAVE", 4) != 0)
	{
		wfp.FileClose();
		return -2;
	}
	fileSize -= 4;
	wavePos += 4;

	int foundFmt = 0;
	int foundWav = 0;
	
	int dataSize = 0;
	// Find the format and data chunks.
	while (fileSize > 0)
	{
		if (!ReadChunk(wfp, chunk))
			break;
		fileSize -= 8;
		if (!foundWav)
			wavePos += 8;
		if (memcmp(chunk.chunkId, "fmt ", 4) == 0 && chunk.chunkSize >= 16)
		{
			if (!ReadFmt(wfp,fmt))
				break;
			if (chunk.chunkSize > 16)
				wfp.FileSkip(chunk.chunkSize-16);
			fileSize -= chunk.chunkSize;
			if (!foundWav)
				wavePos += chunk.chunkSize;
			if ((fmt.fmtCode == 1 && fmt.bits == 16) 
			 || (fmt.fmtCode == 3 && fmt.bits == 32))
			{
				foundFmt = 1;
			}
		}
		else if (memcmp(chunk.chunkId, "data", 4) == 0)
		{
			foundWav = 1;
			dataSize = chunk.chunkSize;
			wfp.FileSkip(chunk.chunkSize);
			fileSize -= chunk.chunkSize;
			// we must break here since the code below
			// assumes the file is positioned at the
			// beginning of the sample data block. 
			//break;
		}
		else if (memcmp(chunk.chunkId, "fact", 4) == 0)
		{
			wfp.FileRead(&sampleTotal, 4);
			if (!foundWav)
				wavePos += 4;
		}
		else 
		{
			wfp.FileSkip(chunk.chunkSize);
			fileSize -= chunk.chunkSize;
			if (!foundWav)
				wavePos += chunk.chunkSize;
		}
	}

	if (!foundFmt || !foundWav)
	{
		wfp.FileClose();
		return -2;
	}

	wfp.FileRewind(wavePos);

	filename = new char[strlen(fname)+1];
	if (filename)
		strcpy(filename, fname);
	if (sampleTotal == 0)
		sampleTotal = dataSize / fmt.align;
	else
		sampleTotal *= fmt.channels;
	samples = new AmpValue[sampleTotal];
	if (samples == NULL)
	{
		wfp.FileClose();
		sampleTotal = 0;
		return -3;
	}

	AmpValue *sp = samples;
	AmpValue peak = 0;
	AmpValue val = 0;
	long count;
	if (fmt.fmtCode == 1)
	{
		bsInt16 *in16 = new bsInt16[fmt.align/2];
		for (count = sampleTotal; count > 0; count--)
		{
			if (wfp.FileRead(in16, fmt.align) != fmt.align)
			{
				while (count-- > 0)
					*sp++ = 0;
				break;
			}
			val = (AmpValue) SwapSample(in16[0]);
			if (fmt.channels > 1)
				val += (AmpValue) SwapSample(in16[1]);
			*sp++ = val;
			val = fabs(val);
			if (val > peak)
				peak = val;
		}
		delete in16;
	}
	else if (fmt.fmtCode == 3)
	{
		float *inflp = new float[fmt.align/4];
		for (count = sampleTotal; count > 0; count--)
		{
			if (wfp.FileRead(inflp, fmt.align) != fmt.align)
			{
				while (count-- > 0)
					*sp++ = 0;
				break;
			}
			val = (AmpValue) inflp[0];
			if (fmt.channels > 1)
				val += (AmpValue) inflp[1];
			*sp++ = val;
			val = fabs(val);
			if (val > peak)
				peak = val;
		}
		delete inflp;
	}

	wfp.FileClose();

	if (peak != 0)
	{
		sp = samples;
		for (count = sampleTotal; count > 0; count--)
			*sp++ /= peak;
	}

	fileID = id;

	return 0;
}
