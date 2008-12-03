///////////////////////////////////////////////////////////
// BasicSynth - send samples to the sound card using DirectSound
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////

#include <windows.h>
#include <dsound.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveFile.h>
#include <WaveOutDirect.h>

WaveOutDirect::WaveOutDirect()
{
	dirSndObj = 0;
	dirSndBuf = 0;

	nextWrite = 0;
	numBlk = 4;
	blkLen = 0;
	bufLen = 0;
	lastBlk = 0;
	startLock = 0;
	sizeLock = 0;
	pauseTime = 20;
	outState = 0;
}

WaveOutDirect::~WaveOutDirect()
{
	if (dirSndBuf)
		dirSndBuf->Release();
	if (dirSndObj)
		dirSndObj->Release();
}

void WaveOutDirect::Stop()
{
	if (dirSndBuf)
		dirSndBuf->Stop();
	outState = 0;
}

void WaveOutDirect::Restart()
{
	if (dirSndBuf)
	{
		// Lock the first block
		dirSndBuf->Lock(0, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		outState = 1;
	}
}

int WaveOutDirect::CreateSoundBuffer(HWND w, float leadtm)
{
	if (dirSndObj == NULL)
	{
		HRESULT hr;
		hr = DirectSoundCreate(NULL, &dirSndObj, NULL);
		if (hr == S_OK)
		{
			hr = dirSndObj->SetCooperativeLevel(w, DSSCL_NORMAL);
			if (hr != S_OK)
			{
				dirSndObj->Release();
				dirSndObj = NULL;
				return -1;
			}
		}
	}

	if (dirSndBuf)
	{
		dirSndBuf->Stop();
		dirSndBuf->Release();
		dirSndBuf = 0;
	}

	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 2;
    wf.nSamplesPerSec = synthParams.isampleRate;
	wf.nBlockAlign = wf.nChannels * 2;
    wf.wBitsPerSample = 16;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	wf.cbSize = 0;

	sampleMax = (bsInt32) ((synthParams.sampleRate * leadtm) * (FrqValue)wf.nChannels);
	if (sampleMax & 1)
		sampleMax++;
	blkLen = sampleMax * 2; // two bytes per sample
	bufLen = blkLen * numBlk;
	lastBlk = bufLen - blkLen;

	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2; 
	dsbd.dwBufferBytes = bufLen;
	dsbd.dwReserved = 0; 
	dsbd.lpwfxFormat = &wf;
	
	if (dirSndObj->CreateSoundBuffer(&dsbd, &dirSndBuf, NULL) != S_OK)
		return -1;

	outState = 1;
	nextWrite = 0;
	// when we must wait, we wait 1/4 of a block length
	pauseTime = (DWORD) (leadtm * 250);

	return 0;
}

int WaveOutDirect::Setup(HWND w, float leadtm, int nb)
{
	if (nb < 3)
		nb = 3;
	numBlk = nb;
	if (CreateSoundBuffer(w, leadtm))
		return -1;

	// Lock the first block
	if (dirSndBuf->Lock(0, blkLen, &startLock, &sizeLock, NULL, NULL, 0) != S_OK)
		return -1;
	samples = (SampleValue *) startLock;
	nxtSamp = samples;
	endSamp = nxtSamp + sampleMax;
	ownBuf = 0;

	return 0;
}

void WaveOutDirect::Shutdown()
{
	if (dirSndBuf)
	{
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		dirSndBuf->Stop();
		dirSndBuf->Release();
		dirSndBuf = 0;
	}
	if (dirSndObj)
	{
		dirSndObj->Release();
		dirSndObj = 0;
	}
}

// NB: we always leave this function with a portion of the
// buffer locked. Since samples are going directly to the buffer,
// it is imperative to do that.
int WaveOutDirect::FlushOutput()
{
	DWORD m, pl;
	switch (outState)
	{
	case 0:
		// suspended state...
		Sleep(pauseTime*4);
		break;
	case 1:
		// Initial write. Start playback
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		dirSndBuf->SetCurrentPosition(0);
		dirSndBuf->Play(0, 0, DSBPLAY_LOOPING);
		nextWrite = blkLen;
		dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		outState = 2;
		break;
	case 2:
		// filling buffer first time through
		nextWrite += blkLen;
		if (nextWrite < bufLen)
		{
			dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
			dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
			break;
		}
		nextWrite = 0;
		outState = 3;
		// FALLTHROUGH
	case 3:
		// Writing behind playback; wait for play position in next block
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		m = nextWrite + blkLen;
		dirSndBuf->GetCurrentPosition(&pl, NULL);
		while (pl < m)
		{
			Sleep(pauseTime);
			dirSndBuf->GetCurrentPosition(&pl, NULL);
		}
		dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		nextWrite = m;
		if (nextWrite >= lastBlk)
			outState = 4;
		break;
	case 4:
		// Writing to last block; wait for play position in first block
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		dirSndBuf->GetCurrentPosition(&pl, NULL);
		while (pl > blkLen)
		{
			Sleep(pauseTime);
			dirSndBuf->GetCurrentPosition(&pl, NULL);
		}
		dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		nextWrite = 0;
		outState = 3;
		break;
	}
	nxtSamp = (SampleValue*)startLock;
	endSamp = nxtSamp + sampleMax;
	return 0;
}

/////////////////////////////////////////////////////////

int WaveOutDirectI::Setup(HWND w, float leadtm, int nb)
{
	if (nb < 3)
		nb = 3;
	numBlk = nb;
	if (CreateSoundBuffer(w, leadtm))
		return -1;
	AllocBuf(sampleMax, 2);
	return 0;
}

void WaveOutDirectI::Restart()
{
	if (dirSndBuf)
		outState = 1;
}

int WaveOutDirectI::FlushOutput()
{
	DWORD m, pl;
	switch (outState)
	{
	case 0:
		// suspended state...
		Sleep(pauseTime*4);
		break;
	case 1:
		// Initial write. Start playback
		dirSndBuf->SetCurrentPosition(0);
		dirSndBuf->Lock(0, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		memcpy(startLock, samples, blkLen);
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		dirSndBuf->Play(0, 0, DSBPLAY_LOOPING);
		nextWrite = blkLen;
		outState = 2;
		break;
	case 2:
		// Filling to end of buffer
		dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		memcpy(startLock, samples, blkLen);
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		nextWrite += blkLen;
		if (nextWrite >= bufLen)
		{
			nextWrite = 0;
			outState = 3;
		}
		break;
	case 3:
		// wait for playback to move ahead of write position
		m = nextWrite + blkLen;
		dirSndBuf->GetCurrentPosition(&pl, NULL);
		if (pl >= nextWrite)
		{
			while (pl < m)
			{
				Sleep(pauseTime);
				dirSndBuf->GetCurrentPosition(&pl, NULL);
			}
		}
		dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		memcpy(startLock, samples, blkLen);
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		nextWrite = m;
		if (nextWrite >= lastBlk)
			outState = 4;
		break;
	case 4:
		// wait for position to wrap to first block
		dirSndBuf->GetCurrentPosition(&pl, NULL);
		while (pl > blkLen)
		{
			Sleep(pauseTime);
			dirSndBuf->GetCurrentPosition(&pl, NULL);
		}
		dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		memcpy(startLock, samples, blkLen);
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		nextWrite = 0;
		outState = 3;
		break;
	}

	nxtSamp = samples;
	return 0;
}
