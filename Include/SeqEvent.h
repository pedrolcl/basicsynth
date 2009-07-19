///////////////////////////////////////////////////////////
// BasicSynth Sequencer events
//
// SeqEvent - sequencer event base class.
// NoteEvent - instrument event base class
// VarParamEvent - variable parameters class
//
/// @file SeqEvent.h Sequencer events
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
////////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{
#ifndef _SEQEVENT_H
#define _SEQEVENT_H

/// Start a sound
#define SEQEVT_START 0
/// Stop a sound
#define SEQEVT_STOP  1
/// Change instrument parameters without restart
#define SEQEVT_PARAM 2
/// Restart the sound with new parameters
#define SEQEVT_RESTART 3
/// Start a sequencer track
#define SEQEVT_STARTTRACK 4
/// Stop a sequencer track
#define SEQEVT_STOPTRACK 5
/// MIDI (or other) Control change
#define SEQEVT_CONTROL 6

// Paramater index
/// Instrument number
#define P_INUM  0
/// Mixer channel number
#define P_CHNL  1
/// Start time in samples
#define P_START 2
/// Duration in samples
#define P_DUR   3
/// First id for user params
#define P_XTRA 4

class InstrConfig;

///////////////////////////////////////////////////////////
/// A sequencer event.
/// SeqEvent defines the minimum information needed by the
/// sequencer class. This can be used alone in cases where
/// a control function is needed. In that case, the
/// "instrument" only needs to know to start/stop operation
/// and does not need pitch, volume, duration, etc.
///
/// Events provide the coupling between the sequencer, instrument
/// manager and active instruments. The base class defines the
/// information needed by the sequencer. Each instrument derives
/// an event structure from SeqEvent, NoteEvent or VarParamEvent
/// and can add whatever additional parameters or methods 
/// the instrument needs. The event is passed to the instrument
/// for the START or PARAM signals.
///
/// A parameter ID value is the value used by the instrument
/// to identify the parameter. The first sixteen parameter IDs
/// are reserved for the sequencer.
///////////////////////////////////////////////////////////
class SeqEvent : public SynthList<SeqEvent>
{
public:
	bsInt16 type;     // event type, 0 -> start output, 1 -> alter parameters
	bsInt16 inum;     // instrument number
	bsInt16 chnl;     // channel number (usually mixer input)
	bsInt16 track;    // track number (formerly known as extra)
	bsInt32 evid;     // event ID or reference to earlier event
	bsInt32 start;    // start time in samples
	bsInt32 duration; // duration in samples
	InstrConfig *im;  //

	SeqEvent()
	{
		type = -1;
		inum = -1;
		evid = -1;
		track = 0;
		chnl = 0;
		start = 0;
		duration = 0;
		im = 0;
	}

	virtual ~SeqEvent() { }

	/// Destroy the event.
	/// The instrument manager and sequencer will call Destroy
	/// rather than the desctructor. Derived classes can use
	/// this feature to recycle events if desired.
	virtual void Destroy() { delete this; }

	/// Allocate space for variable parameters.
	/// @param n the number of parameters needed.
	virtual int AllocParam(bsInt16 n) { return 0; }

	/// Get the maximum parameter ID.
	virtual bsInt16 MaxParam() { return P_DUR; }

	virtual void SetInum(bsInt16 i) { inum = i; }
	virtual void SetType(bsInt16 t) { type = t; }
	virtual void SetID(bsInt16 id) { evid = id; }
	virtual void SetChannel(bsInt16 c) { chnl = c; }
	virtual void SetTrack(bsInt16 t) { track = t; }
	virtual void SetStart(bsInt32 s) { start = s; }
	virtual void SetDuration(bsInt32 d) { duration = d; }

	/// Set a parameter
	/// @param id unique id number for this value
	/// @param v the parameter value
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_INUM:
			SetInum((bsInt16) v);
			break;
		case P_CHNL:
			SetChannel((bsInt16) v);
			break;
		case P_START: // in seconds
			SetStart((bsInt32) (synthParams.sampleRate * v));
			break;
		case P_DUR: // in seconds
			SetDuration((bsInt32) (synthParams.sampleRate * v));
			break;
		}
	}

	/// Set parameter from a string. This is a convenience
	/// for sequencer file readers.
	/// @param id unique id number for this value
	/// @param s the parameter value
	virtual void SetParam(bsInt16 id, char *s)
	{
		SetParam(id, (float) atof(s));
	}

	virtual float GetParam(bsInt16 id)
	{
		switch (id)
		{
		case P_INUM:
			return (float) inum;
		case P_CHNL:
			return (float) chnl;
		case P_START:
			return (float) start / synthParams.sampleRate;
		case P_DUR:
			return (float) duration / synthParams.sampleRate;
		}
		return 0;
	}

	virtual void Reset()
	{
		inum = 0;
		chnl = 0;
		start = 0;
		duration = 0;
		track = 0;
	}
};

/// The pitch value (0-127)
#define P_PITCH  4
/// The frequency value (0-SR/2)
#define P_FREQ   5
/// The volume (0-1)
#define P_VOLUME 6
/// Track number
#define P_TRACK  7
/// Note-on velocity (MIDI)
#define P_NOTEONVEL 8
/// First instrument specific parameter is P_USER = 16
#define P_USER 16

///////////////////////////////////////////////////////////
/// A note event.
/// The NoteEvent structure adds pitch, frequency and volume
/// to the pre-defined event parameters. Instruments that 
/// are to be used with Notelist should derive their events
/// from the NoteEvent class instead of SeqEvent.
/// ID numbers up to P_USER are reserved for future use.
/// @sa SeqEvent
///////////////////////////////////////////////////////////
class NoteEvent : public SeqEvent
{
public:
	bsInt16 pitch;
	bsInt16 noteonvel;
	FrqValue frq;
	AmpValue vol;

	NoteEvent()
	{
		pitch = 0;
		noteonvel = 0;
		frq = 0.0;
		vol = 1.0;
	}

	/// @copydoc SeqEvent::MaxParam
	virtual bsInt16 MaxParam() { return P_NOTEONVEL; }
	virtual void SetFrequency(FrqValue f) { frq = f; }
	virtual void SetPitch(bsInt16 p) 
	{
		pitch = p;
		SetFrequency(synthParams.GetFrequency(pitch));
	}
	virtual void SetVolume(AmpValue v) { vol = v; }
	virtual void SetVelocity(bsInt16 v) { noteonvel = v; }

	/// @copydoc SeqEvent::SetParam
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_PITCH:
			SetPitch((bsInt16)v);
			break;
		case P_FREQ:
			SetFrequency(FrqValue(v));
			break;
		case P_VOLUME:
			SetVolume(AmpValue(v));
			break;
		case P_TRACK:
			SetTrack((bsInt16) v);
			break;
		case P_NOTEONVEL:
			SetVelocity((bsInt16)v);
			break;
		default:
			SeqEvent::SetParam(id, v);
			break;
		}
	}

	virtual float GetParam(bsInt16 id)
	{
		switch (id)
		{
		case P_PITCH:
			return (float) pitch;
		case P_FREQ:
			return (float) frq;
		case P_VOLUME:
			return (float) vol;
		case P_TRACK:
			return (float) track;
		case P_NOTEONVEL:
			return (float) noteonvel;
		}
		return SeqEvent::GetParam(id);
	}
};


///////////////////////////////////////////////////////////
/// Variable parameter event.
/// Instruments that have variable number of parameters,
/// or have a very large number of parameters, can use
/// VarParamEvent as a class for events. The event
/// factory should set the maxParam member to the largest
/// possible ID value. 
/// @sa SeqEvent
///////////////////////////////////////////////////////////
class VarParamEvent : public NoteEvent
{
public:
	bsInt16 maxParam;
	bsInt16 allParam;
	bsInt16 numParam;
	bsInt16 *idParam;
	float *valParam;

	VarParamEvent()
	{
		maxParam = 0;
		allParam = 0;
		numParam = 0;
		idParam  = 0;
		valParam = 0;
	}

	~VarParamEvent()
	{
		if (allParam > 0)
		{
			delete idParam;
			delete valParam;
		}
	}

	/// @copydoc SeqEvent::AllocParam
	int AllocParam(bsInt16 n)
	{
		bsInt16 *ndx = new bsInt16[n];
		if (ndx == 0)
			return 0;
		float *val = new float[n];
		if (val == 0)
		{
			delete ndx;
			return 0;
		}
		if (idParam != NULL)
		{
			memcpy(ndx, idParam, allParam*sizeof(bsInt16));
			delete idParam;
		}
		if (valParam != NULL)
		{
			memcpy(val, valParam, allParam*sizeof(float));
			delete valParam;
		}
		idParam = ndx;
		valParam = val;
		allParam = n;
		return n;
	}

	/// @copydoc SeqEvent::MaxParam
	virtual bsInt16 MaxParam() 
	{ 
		return maxParam;
	}

	/// @copydoc SeqEvent::SetParam
	void SetParam(bsInt16 id, float v)
	{
		if (id <= NoteEvent::MaxParam())
			NoteEvent::SetParam(id, v);
		else if (id <= maxParam)
		{
			if (numParam >= allParam)
			{
				if (AllocParam(allParam+5) == 0)
					return;
			}
			idParam[numParam] = id;
			valParam[numParam] = v;
			numParam++;
		}
	}

	void Reset()
	{
		numParam = 0;
	}

	float GetParam(bsInt16 id)
	{
		bsInt16 *pp = idParam;
		for (int n = 0; n < numParam; n++, pp++)
		{
			if (*pp == id)
				return valParam[n];
		}
		return NoteEvent::GetParam(id);
	}

	/// Replace or Set the value of a parameter
	void UpdateParam(bsInt16 id, float val)
	{
		if (id < P_USER)
		{
			NoteEvent::SetParam(id, val);
			return;
		}

		bsInt16 *pp = idParam;
		for (int n = 0; n < numParam; n++, pp++)
		{
			if (*pp == id)
			{
				valParam[n] = val;
				return;
			}
		}
		SetParam(id, val);
	}
};

#define P_MMSG  4
#define P_CTRL  5
#define P_CVAL  6

/// A ControlEvent is used for global synthsizer control.
/// For the most part, this is used to implement MIDI
/// channel voice messages. It could be used for other
/// things as well. These events are handled by the
/// channel manager object of the Sequencer.
class ControlEvent : public SeqEvent
{
public:
	bsInt16 mmsg;
	bsInt16 ctrl;
	bsInt16 cval;

	ControlEvent()
	{
		mmsg = 0;
		ctrl = 0;
		cval = 0;
	}

	/// Get the maximum parameter ID.
	virtual bsInt16 MaxParam() { return P_CVAL; }
	virtual void SetMessage(bsInt16 m) { mmsg = m; }
	virtual void SetControl(bsInt16 c) { ctrl = c; }
	virtual void SetValue(bsInt16 v)   { cval = v; }

	/// Set a parameter
	/// @param id unique id number for this value
	/// @param v the parameter value
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_MMSG:
			SetMessage((bsInt16) v);
			break;
		case P_CTRL:
			SetControl((bsInt16) v);
			break;
		case P_CVAL:
			SetValue((bsInt16) v);
			break;
		case P_TRACK:
			SetTrack((bsInt16) v);
			break;
		default:
			SeqEvent::SetParam(id, v);
			break;
		}
	}
	virtual float GetParam(bsInt16 id)
	{
		switch (id)
		{
		case P_MMSG:
			return (float) mmsg;
		case P_CTRL:
			return (float) ctrl;
		case P_CVAL:
			return (float) cval;
		case P_TRACK:
			return (float) track;
		}
		return SeqEvent::GetParam(id);
	}
};

#define P_TRKNO 5
#define P_LOOP  6

class TrackEvent : public SeqEvent
{
public:
	bsInt16 trkNo;
	bsInt16 loopCount;

	TrackEvent()
	{
		trkNo = -1;
		loopCount = 0;
	}

	virtual void SetTrkNo(bsInt16 tk) { trkNo = tk; }
	virtual void SetLoop(bsInt16 lc) { loopCount = lc; }

	/// Get the maximum number of parameters.
	virtual bsInt16 MaxParam() { return P_LOOP; }

	/// Set a parameter
	/// @param id unique id number for this value
	/// @param v the parameter value
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_TRKNO:
			SetTrkNo((bsInt16) v);
			break;
		case P_LOOP:
			SetLoop((bsInt16) v);
			break;
		case P_TRACK:
			SetTrack((bsInt16) v);
			break;
		default:
			SeqEvent::SetParam(id, v);
			break;
		}
	}

	virtual float GetParam(bsInt16 id)
	{
		switch (id)
		{
		case P_TRKNO:
			return (float) trkNo;
		case P_LOOP:
			return (float) loopCount;
		case P_TRACK:
			return (float) track;
		}
		return SeqEvent::GetParam(id);
	}
};

//@}
#endif
