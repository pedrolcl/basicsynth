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

// Paramater index
/// Instrument number
#define P_INUM  0
/// Mixer channel number
#define P_CHNL  1
/// Start time in samples
#define P_START 2
/// Duration in samples
#define P_DUR   3
/// Index of first instrument-specific ID
#define P_XTRA  4

class InstrMapEntry;

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
/// A parameter number is a monotonically increasing value that
/// is an index into the array of parameters. A parameter ID value
/// is the actual value used by the instrument to identify the 
/// parameter. For a fixed number of parameters, the two values
/// are the same. For variable number of parameters, the index and ID
/// will usually be different.
///////////////////////////////////////////////////////////
class SeqEvent : public SynthList<SeqEvent>
{
public:
	bsInt16 type;     // event type, 0 -> start output, 1 -> alter parameters
	bsInt16 inum;     // instrument number
	bsInt16 chnl;     // channel number (usually mixer input)
	bsInt16 xtra;     // padding - reserved for future use
	bsInt32 evid;     // event ID or reference to earlier event
	bsInt32 start;    // start time in samples
	bsInt32 duration; // duration in samples
	InstrMapEntry *im; //

	SeqEvent()
	{
		type = -1;
		inum = -1;
		evid = -1;
		xtra = 0;
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

	/// Get the maximum number of parameters.
	virtual bsInt16 MaxParam() { return P_XTRA; }

	/// Set a parameter
	/// @param id unique id number for this value
	/// @param v the parameter value
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_INUM:
			inum = (bsInt16) v;
			break;
		case P_CHNL:
			chnl = (bsInt16) v;
			break;
		case P_START:
			start = (bsInt32) (synthParams.sampleRate * v);
			break;
		case P_DUR:
			duration = (bsInt32) (synthParams.sampleRate * v);
			break;
		case P_XTRA:
			xtra = (bsInt16) v;
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
};

/// The pitch value (0-127)
#define P_PITCH  4
/// The frequency value (0-SR/2)
#define P_FREQ   5
/// The volume (0-1)
#define P_VOLUME 6
/// First instrument specific parameter is P_USER = 16
#define P_USER 16

///////////////////////////////////////////////////////////
/// A note event.
/// The NoteEvent structure adds pitch, frequency and volume
/// to the pre-defined event parameters. Instruments that 
/// are to be used with Notelist should derive their events
/// from the NoteEvent class instead of SeqEvent.
/// ID numbers from P_XTRA up to P_USER are reserved for Notelist.
/// @sa SeqEvent
///////////////////////////////////////////////////////////
class NoteEvent : public SeqEvent
{
public:
	int pitch;
	FrqValue frq;
	AmpValue vol;

	NoteEvent()
	{
		pitch = 0;
		frq = 0.0;
		vol = 1.0;
	}

	/// @copydoc SeqEvent::MaxParam
	virtual bsInt16 MaxParam() { return P_VOLUME; }

	/// @copydoc SeqEvent::SetParam
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_PITCH:
			pitch = (int) v;
			frq = synthParams.GetFrequency(pitch);
			break;
		case P_FREQ:
			frq = FrqValue(v);
			break;
		case P_VOLUME:
			vol = AmpValue(v);
			break;
		default:
			SeqEvent::SetParam(id, v);
			break;
		}
	}
};


///////////////////////////////////////////////////////////
/// Variable parameter event.
/// Instruments that have variable number of parameters,
/// or have a very large number of parameters, can use
/// VarParamEvent as a class for events. The event
/// factory should set the maxParam member to the number
/// of unique parameters, not the maximum instrument parameter
/// ID value. 
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
		else if (id < maxParam)
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
};
//@}
#endif
