///////////////////////////////////////////////////////////////
//
// BasicSynth - Wave file output
//
// This is a simple wave file output class.
//
// N.B. This code does not do canonicalization for big-endian processor.
//
// Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _WAVEFILE_H_
#define _WAVEFILE_H_

#include <SynthFile.h>

#if BIG_ENDIAN
extern SampleValue SwapSample(SampleValue x);
#else
#define SwapSample(x) x
#endif

#define CHUNK_SIZE 8
#define CHUNK_ID   4

struct RiffChunk
{
	bsInt8  chunkId[CHUNK_ID];
	bsInt32 chunkSize;
};

struct FmtData
{
	bsInt16 fmtCode;     // 1 = PCM
	bsInt16 channels;    // 1 = mono, 2 = stereo
	bsInt32 sampleRate;  // 44100
	bsInt32 avgbps;      // samplerate * align
	bsInt16 align;       // (channels*bits)/8;
	bsInt16 bits;        // bits per sample (16)
};

struct WavHDR
{
	bsInt8  riffId[4];   // 'RIFF' chunk
	bsInt32 riffSize;    // filesize - 8
	bsInt8  waveType[4]; // 'WAVE' type of file
	bsInt8  fmtId[4];    // 'fmt ' format chunk
	bsInt32 fmtSize;     // size of format chunk (16)
	bsInt16 fmtCode;     // 1 = PCM
	bsInt16 channels;    // 1 = mono, 2 = stereo
	bsInt32 sampleRate;  // 44100
	bsInt32 avgbps;      // samplerate * align
	bsInt16 align;       // (channels*bits)/8;
	bsInt16 bits;        // bits per sample (16 or 24)
	bsInt8  waveId[4];   // 'data' chunk
	bsInt32 waveSize;    // size of chunk in bytes
};

// output sample buffer
class WaveOutBuf
{
protected:
	bsInt32 sampleTotal;
	bsInt32 sampleNumber;
	bsInt32 sampleMax;
	bsInt32 sampleOOR;
	bsInt16 channels;
	SampleValue *samples;
	SampleValue *nxtSamp;
	SampleValue *endSamp;
	int   ownBuf;

public:
	WaveOutBuf()
	{
		channels = 0;
		sampleTotal = 0;
		sampleNumber = 0;
		sampleMax = 0;
		sampleOOR = 0;
		samples = 0;
		nxtSamp = 0;
		endSamp = 0;
		ownBuf = 0;
	}

	~WaveOutBuf()
	{
		if (ownBuf)
			DeallocBuf();
	}

	SampleValue *GetBuf()
	{
		return samples;
	}

	virtual int AllocBuf(long length, bsInt16 ch)
	{
		DeallocBuf();
		samples = new SampleValue[length];
		if (samples == NULL)
		{
			sampleMax = 0;
			return -1;
		}
		channels = ch;
		sampleMax = length;
		memset(samples, 0, length * sizeof(SampleValue));
		sampleNumber = 0;
		sampleTotal = 0;
		nxtSamp = samples;
		endSamp = samples + length;
		ownBuf = 1;
		return 0;
	}

	virtual void DeallocBuf()
	{
		if (ownBuf && samples)
			delete samples;
		samples = NULL;
		ownBuf = 0;
		sampleMax = 0;
	}

	virtual void SetBuf(long length, bsInt16 ch, SampleValue *bp)
	{
		DeallocBuf();
		sampleMax = length;
		channels = ch;
		samples = bp;
		nxtSamp = samples;
		endSamp = samples + length;
		//ownBuf = 0;
	}

	// Put value directly into buffer
	// without range checking or scaling.
	void OutS(SampleValue value)
	{
	//	if (sampleNumber >= sampleMax)
	//		FlushOutput();
	//	samples[sampleNumber++] = SwapSample(value);
		if (nxtSamp >= endSamp)
			FlushOutput();
		*nxtSamp++ = SwapSample(value);
		sampleTotal++;
	}

	// Write one value only
	virtual void Output(AmpValue value)
	{
		// the out-of-range test can be removed to gain a
		// slight performance increase if you are sure
		// the values cannot go overrange, or just don't care...
		if (value > 1.0)
		{
			sampleOOR++;
			value = 1.0;
		}
		else if (value < -1.0)
		{
			value = -1.0;
			sampleOOR++;
		}
		//if (sampleNumber >= sampleMax)
		//	FlushOutput();
		//samples[sampleNumber++] = SwapSample((SampleValue) (value * synthParams.sampleScale));
		if (nxtSamp >= endSamp)
			FlushOutput();
		*nxtSamp++ = SwapSample((SampleValue) (value * synthParams.sampleScale));
		sampleTotal++;
	}

	// Write one value to all channels
	virtual void Output1(AmpValue value)
	{
		Output(value);
		if (channels == 2)
			Output(value);
	}

	// Write two values, combining if needed
	virtual void Output2(AmpValue vleft, AmpValue vright)
	{
		if (channels == 1)
		{
			Output((vleft + vright) / 2);
		}
		else
		{
			Output(vleft);
			Output(vright);
		}
	}

	// Dummy base class method.
	// derived classes must implement this
	virtual int FlushOutput()
	{
		nxtSamp = samples;
		//sampleNumber = 0;
		return 0;
	}

	long GetOOR() { return sampleOOR; }
};

// wave file writer 
class WaveFile : public WaveOutBuf
{
private:
	FileWriteUnBuf wfp;
	WavHDR wh;
	int   bufSecs;

	void SetupWH();

public:
	WaveFile()
	{
		bufSecs = 5;
	}

	~WaveFile()
	{
		wfp.FileClose();
	}

	// size of buffer in seconds
	void SetBufSize(int secs)
	{
		bufSecs = secs;
	}

	// Open wave output file, 
	// fname is file name, channels number of outputs
	int OpenWaveFile(char *fname, int chnls = 1);

	// Flush remaining output and close file
	int CloseWaveFile();

	// write output to WAVE file
	int FlushOutput();
};

#define SMPL_BUFSIZE 8192

class WaveFileIn
{
private:
	char *filename;
	bsInt16 fileID;
	AmpValue *samples;
	bsInt32 sampleTotal;

public:
	WaveFileIn()
	{
		filename = NULL;
		samples = NULL;
		sampleTotal = 0;
		fileID = -1;
	}

	~WaveFileIn()
	{
		if (samples)
			delete samples;
		if (filename)
			delete filename;
	}

	const char *GetFilename()
	{
		return filename;
	}

	bsInt16 GetFileID()
	{
		return fileID;
	}

	AmpValue *GetSampleBuffer()
	{
		return samples;
	}

	long GetInputLength()
	{
		return (long)sampleTotal;
	}

	void Clear()
	{
		if (filename)
		{
			delete filename;
			filename = 0;
		}
		if (samples)
		{
			delete samples;
			samples = 0;
		}
		fileID = -1;
		sampleTotal = 0;
	}

	int LoadWaveFile(const char *fname, bsInt16 id);
};
#endif
