///////////////////////////////////////////////////////////
// BasicSynth - send samples to the sound card using DirectSound
//
// We have two options:
// 1. Indirect - samples are put into a local buffer and then
//             copied to the direct sound buffer when the local
//             buffer is filled.
// 2. Direct - samples are written into the direct sound buffer.
//
// The direct method is slightly faster, but the indirect
// method is safer since it always has a valid buffer.
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////

#ifndef _WAVEOUTDIRECT_H
#define _WAVEOUTDIRECT_H 1

class WaveOutDirect : public WaveOutBuf
{
protected:
	IDirectSound *dirSndObj;
	IDirectSoundBuffer *dirSndBuf;

	DWORD numBlk;
	DWORD nextWrite;
	DWORD blkLen;
	DWORD bufLen;
	DWORD lastBlk;
	void *startLock;
	DWORD sizeLock;
	DWORD pauseTime;
	int outState;

	int CreateSoundBuffer(HWND w, float leadtm);

public:
	WaveOutDirect();
	~WaveOutDirect();
	int Setup(HWND wnd, float leadtm, int nb = 4);
	void Stop();
	virtual int FlushOutput();
};

/// Ths class uses an indirect buffer write.
class WaveOutDirectI : public WaveOutDirect
{
public:
	WaveOutDirectI();
	int Setup(HWND wnd, float leadtm, int nb = 4);
	virtual int FlushOutput();
};

#endif
