//////////////////////////////////////////////////////////////////////
/// @file GMSynthDLL.h General MIDI Synthesizer
/// This is a prototype and may change significantly in future versions.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpMIDI
///@{
#ifndef GMSYNTHDLL_H
#define GMSYNTHDLL_H

#ifdef _WIN32
# ifndef STATIC_GMSYNTH
#  ifdef _USRDLL
#   define EXPORT __declspec(dllexport)
#  else
#   define EXPORT __declspec(dllimport)
#  endif
# else
#  define EXPORT
# endif
#endif
#ifdef UNIX
# define HANDLE long
# define EXPORT
#endif

// ID values for MetaText
#define GMSYNTH_META_SEQTEXT  0  ///< MIDI file text
#define GMSYNTH_META_SEQCPYR  1  ///< MIDI file copyright
#define GMSYNTH_META_SEQNAME  2  ///< MIDI file sequence name
#define GMSYNTH_META_TIMESIG  3  ///< MIDI file time signature
#define GMSYNTH_META_KEYSIG   4  ///< MIDI file key signature
#define GMSYNTH_META_SBKNAME  6  ///< SoundBank name (INFO text from file)
#define GMSYNTH_META_SBKCPYR  7  ///< SoundBank copyright
#define GMSYNTH_META_SBKCMNT  8  ///< SoundBank comment
#define GMSYNTH_META_SBKVER   9  ///< SoundBank version

#define GMSYNTH_EVENT_TICK    1
#define GMSYNTH_EVENT_START   2  ///< Sequencer started
#define GMSYNTH_EVENT_STOP    3  ///< Sequencer stopped
#define GMSYNTH_EVENT_PAUSE   4  ///< Sequencer paused
#define GMSYNTH_EVENT_RESUME  5  ///< Sequencer resumed
#define GMSYNTH_EVENT_NOTEON  6  ///< Note start
#define GMSYNTH_EVENT_NOTEOFF 7  ///< Note end
#define GMSYNTH_EVENT_CTLCHG  8  ///< MIDI Control change
#define GMSYNTH_EVENT_TRKON   9  ///< Start a sequencer track
#define GMSYNTH_EVENT_TRKOFF  10  ///< Stop a sequencer track

#define GMSYNTH_MODE_PLAY     0 ///< Play immediate events
#define GMSYNTH_MODE_SEQUENCE 1 ///< Play the sequence once
#define GMSYNTH_MODE_SEQPLAY  2 ///< Play both

#define GMSYNTH_NOERROR       0
#define GMSYNTH_ERR_BADHANDLE 1
#define GMSYNTH_ERR_FILETYPE  2
#define GMSYNTH_ERR_FILEOPEN  3
#define GMSYNTH_ERR_BADID     4
#define GMSYNTH_ERR_GENERATE  5

/// Function to receive events
typedef void (*GMSYNTHCB)(bsInt32 evtid, bsInt32 count, Opaque arg);

#ifdef __cplusplus

/// Wrapper class for GMSynthDlL.
class EXPORT GMSynth
{
private:
	void *synth;

public:
	GMSynth(HANDLE w);
	virtual ~GMSynth();

	virtual void SetCallback(GMSYNTHCB cb, bsInt32 cbRate, Opaque arg);
	virtual int LoadSoundBank(const char *alias, const char *fileName, int preload);
	virtual int GetMetaText(int id, char *txt, size_t len);
	virtual int GetMetaData(int id, long *vals);
	virtual int LoadSequence(const char *fileName, const char *sbnkName);
	virtual int Start(int mode);
	virtual int Stop();
	virtual int Pause();
	virtual int Resume();
	virtual int Generate(const char *fileName);
	virtual int MidiIn(int onoff, int device);
	virtual void ImmediateEvent(short mmsg, short val1, short val2);
};

extern "C" {
#endif

HANDLE EXPORT GMSynthOpen(HANDLE w, bsInt32 sr);
int EXPORT GMSynthClose(HANDLE gm);
int EXPORT GMSynthLoadSoundBank(HANDLE gm, const char *fileName, int preload, const char *alias);
int EXPORT GMSynthLoadSequence(HANDLE gm, const char *fileName, const char *sbnkName);
int EXPORT GMSynthStart(HANDLE gm, int mode);
int EXPORT GMSynthStop(HANDLE gm);
int EXPORT GMSynthPause(HANDLE gm);
int EXPORT GMSynthResume(HANDLE gm);
int EXPORT GMSynthGenerate(HANDLE gm, const char *fileName);
int EXPORT GMSynthMetaText(HANDLE gm, int id, char *txt, size_t len);
int EXPORT GMSynthMetaData(HANDLE gm, int id, long *vals);
int EXPORT GMSynthSetCallback(HANDLE gm, GMSYNTHCB cb, bsUint32 cbRate, Opaque arg);
int EXPORT GMSynthMIDIKbdIn(HANDLE gm, int onoff, int device);
int EXPORT GMSynthMIDIEvent(HANDLE gm, short mmsg, short val1, short val2);

#ifdef __cplusplus
// end extern "C"
}
#endif

#endif
//@}