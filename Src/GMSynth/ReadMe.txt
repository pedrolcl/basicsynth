========================================================================
    GMSynthDLL - plug-in synthesis modulue
========================================================================

NOTE: THIS IS UNTESTED CODE, FOR DEMONSTRATION AND EXPERIMENTATION

GMSynth is an prototype for a DLL containing a complete MIDI-based synth module.
The DLL has a C++ wrapper, but can also be invoked through a function call 
interface from various programming languages, such as C, VB and C#.

See the API in GMSynthDLL.h for the function call list and C++ wrapper class.

Typical use to play a MIDI SMF file would be something like...

	HANDLE synth = GMSynthInit(mainWindow, 44100);
	GMSynthLoadSoundBank(synth, "Sounds.sf2", 1, "Sounds");
	GMSynthLoadSequence(synth, "Song.mid", "Sounds");
	GMSynthGenerate(synth, "Song.wav");
	<< Wait for file complete - see below >>
	GMSynthClose(synth);

Typical use for live playing from a keyboard:

	HANDLE synth = = 0;

	StartSynth() {
		synth = GMSynthInit(mainWindow, 44100);
		GMSynthLoadSoundBank(synth, "Sounds.sf2", 1, "Sounds");
		GMSynthMIDIKbdIn(synth, TRUE, device);
		GMSynthStart(synth);
	}

	StopSynth() {
		GMSynthStop(synth);
		GMSynthClose(synth);
	}

The first argument to GMSynthInit is used to set up the sound
output device. On Windows, this is a window handle to pass to
the DirectSound functions and is usually the main application window.
On Linux, this is the ALSA device name (e.g., "hw:0").

Sound generation is performed on a background thread.
A callback function is used to receive various events. 

	void SynthEvent(bsInt32 evtid, bsInt32 count, Opaque arg) {
		<< send message to main thread >>
	}

	GMSynthSetCallback(synth, SynthEvent, samples, arg);

If 'samples' is non-zero, the callback is invoked every 'samples'
of output. This can be used to display the current time or to
synchronize screen updates with the sound playback.

The callback will be invoked from a background thread and must not
make calls to GMSynth functions. The callback function will
need to signal the main thread and return. On windows, the easiest
implementation is to post a message to the main thread's message queue.
Alternatively, and on UNIX, set an event, clear a semaphore, write to a pipe, etc.