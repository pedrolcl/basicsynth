/////////////////////////////////////////////////////
// BasicSynth File classes.
//
// Two simple file classes, one for unbuffered output
// and one for buffered input. These are optimized for
// the needs of the synthesizer and make direct OS API
// calls to eliminate stdio library overhead.
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////

#ifndef _SYNTHFILE_H_
#define _SYNTHFILE_H_

#if defined(WIN32)
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

class FileWriteUnBuf
{
private:
#if defined(WIN32)
	HANDLE fh;
#endif
#if defined(UNIX)
	int fd;
#endif

public:
	FileWriteUnBuf();
	~FileWriteUnBuf();
	int FileOpen(const char *fname);
	int FileClose();
	int FileWrite(void *buf, size_t siz);
	int FileRewind(int pos = 0);
};

class FileReadBuf
{
private:
	int doneAll;
	bsUint8 *inbuf;
#if defined(WIN32)
	HANDLE fh;
	DWORD insize;
	DWORD inread;
	DWORD inpos;
#endif
#if defined(UNIX)
	int fd;
	off_t insize;
	off_t inread;
	off_t inpos;
#endif

public:
	FileReadBuf();
	~FileReadBuf();
	// Note: SetBufSize has no effect unless called before FileOpen
	void SetBufSize(size_t sz);
	int FileOpen(const char *fname);
	int FileRead(void *rdbuf, int rdsiz);
	int ReadCh();
	int FileSkip(int n);
	int FileRewind(int pos = 0);
	int FileClose();
};

int SynthFileExists(const char *fname);

#endif
