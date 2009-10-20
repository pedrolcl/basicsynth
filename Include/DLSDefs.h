///////////////////////////////////////////////////////////
// BasicSynth - MIDI Downloadable Sounds (DLS) definitions
//
/// @file DLSDefs.h DLS file definitions.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSoundbank
//@{
#ifndef _DLSDEFS_H_
#define _DLSDEFS_H_

/////////////////// BEGIN RIFF STRUCTURES //////////////////////////

#ifndef FOURCC
#if BS_BIG_ENDIAN
#define FOURCC(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|d)
#else
#define FOURCC(a,b,c,d) (a|(b<<8)|(c<<16)|(d<<24))
#endif
#endif

// Chunk format values found in a DLS file
#define DLS_RIFF_CHUNK  FOURCC('R','I','F','F')
#define DLS_VERS_CHUNK  FOURCC('v','e','r','s')
#define DLS_DLID_CHUNK  FOURCC('d','l','i','d')
#define DLS_COLH_CHUNK  FOURCC('c','o','l','h')
#define DLS_LIST_CHUNK  FOURCC('L','I','S','T')
#define DLS_PTBL_CHUNK  FOURCC('p','t','b','l')
#define DLS_ART1_CHUNK  FOURCC('a','r','t','1')
#define DLS_ART2_CHUNK  FOURCC('a','r','t','2')
#define DLS_RGNH_CHUNK  FOURCC('r','g','n','h')
#define DLS_INSH_CHUNK  FOURCC('i','n','s','h')
#define DLS_WLNK_CHUNK  FOURCC('w','l','n','k')
#define DLS_WSMP_CHUNK  FOURCC('w','s','m','p')
#define DLS_FMT__CHUNK  FOURCC('f','m','t',' ')
#define DLS_DATA_CHUNK  FOURCC('d','a','t','a')

// INFO-LIST text chunks
#define DLS_IARL_CHUNK  FOURCC('I','A','R','L')
#define DLS_IART_CHUNK  FOURCC('I','A','R','T')
#define DLS_ICMS_CHUNK  FOURCC('I','C','M','S')
#define DLS_ICMT_CHUNK  FOURCC('I','C','M','T')
#define DLS_ICOP_CHUNK  FOURCC('I','C','O','P')
#define DLS_ICRD_CHUNK  FOURCC('I','C','R','D')
#define DLS_IENG_CHUNK  FOURCC('I','E','N','G')
#define DLS_IGNR_CHUNK  FOURCC('I','G','N','R')
#define DLS_IKEY_CHUNK  FOURCC('I','K','E','Y')
#define DLS_IMED_CHUNK  FOURCC('I','M','E','D')
#define DLS_INAM_CHUNK  FOURCC('I','N','A','M')
#define DLS_IPRD_CHUNK  FOURCC('I','P','R','D')
#define DLS_ISBJ_CHUNK  FOURCC('I','S','B','J')
#define DLS_ISFT_CHUNK  FOURCC('I','S','F','T')
#define DLS_ISRC_CHUNK  FOURCC('I','S','R','C')
#define DLS_ISRF_CHUNK  FOURCC('I','S','R','F')
#define DLS_ITCH_CHUNK  FOURCC('I','T','C','H')

#define DLS_FILE_FORMAT FOURCC('D','L','S',' ')
#define DLS_WVPL_FORMAT FOURCC('w','v','p','l')
#define DLS_PATH_FORMAT FOURCC('p','a','t','h')
#define DLS_WAVE_FORMAT FOURCC('w','a','v','e')
#define DLS_LINS_FORMAT FOURCC('l','i','n','s')
#define DLS_INS__FORMAT FOURCC('i','n','s',' ')
#define DLS_INSH_FORMAT FOURCC('i','n','s','h')
#define DLS_RGN__FORMAT FOURCC('r','g','n',' ')
#define DLS_RGN2_FORMAT FOURCC('r','g','n','2')
#define DLS_LRGN_FORMAT FOURCC('l','r','g','n')
#define DLS_LART_FORMAT FOURCC('l','a','r','t')
#define DLS_LAR2_FORMAT FOURCC('l','a','r','2')
#define DLS_INFO_FORMAT FOURCC('I','N','F','O')
#define DLS_CDL__FORMAT FOURCC('c','d','l',' ')

///////////////////////////////////////////////////////////////////////////
// Articulation connection graph definitions
///////////////////////////////////////////////////////////////////////////
// Generic Sources
#define CONN_SRC_NONE 0x0000
#define CONN_SRC_LFO 0x0001
#define CONN_SRC_KEYONVELOCITY 0x0002
#define CONN_SRC_KEYNUMBER 0x0003
#define CONN_SRC_EG1 0x0004
#define CONN_SRC_EG2 0x0005
#define CONN_SRC_PITCHWHEEL 0x0006
#define CONN_SRC_POLYPRESSURE 0x0007
#define CONN_SRC_CHANNELPRESSURE 0x0008
#define CONN_SRC_VIBRATO 0x0009
#define CONN_SRC_MONOPRESSURE 0x000a

// Midi Controllers 0-127
#define CONN_SRC_CC1 0x0081
#define CONN_SRC_CC2 0x0082
#define CONN_SRC_CC7 0x0087
#define CONN_SRC_CC10 0x008a
#define CONN_SRC_CC11 0x008b

// Generic Destinations
#define CONN_DST_NONE 0x0000
#define CONN_DST_ATTENUATION 0x0001
#define CONN_DST_RESERVED 0x0002
#define CONN_DST_PITCH 0x0003
#define CONN_DST_PAN 0x0004
#define CONN_DST_KEYNUMBER 0x0005

// LFO Destinations
#define CONN_DST_LFO_FREQUENCY 0x0104
#define CONN_DST_LFO_STARTDELAY 0x0105

#define CONN_DST_VIB_FREQUENCY 0x0114
#define CONN_DST_VIB_STARTDELAY 0x0115

// EG1 Destinations
#define CONN_DST_EG1_ATTACKTIME 0x0206
#define CONN_DST_EG1_DECAYTIME 0x0207
#define CONN_DST_EG1_RESERVED 0x0208
#define CONN_DST_EG1_RELEASETIME 0x0209
#define CONN_DST_EG1_SUSTAINLEVEL 0x020a
#define CONN_DST_EG1_DELAYTIME 0x020b
#define CONN_DST_EG1_HOLDTIME 0x20c
#define CONN_DST_EG1_SHUTDOWNTIME 0x020d

// EG2 Destinations
#define CONN_DST_EG2_ATTACKTIME 0x030a
#define CONN_DST_EG2_DECAYTIME 0x030b
#define CONN_DST_EG2_RESERVED 0x030c
#define CONN_DST_EG2_RELEASETIME 0x030d
#define CONN_DST_EG2_SUSTAINLEVEL 0x030e
#define CONN_DST_EG2_DELAYTIME 0x030f
#define CONN_DST_EG2_HOLDTIME 0x0310

#define CONN_DST_FLT_CUTOFF 0x0500
#define CONN_DST_FLT_Q 0x0501

#define CONN_TRN_NONE 0x0000
#define CONN_TRN_CONCAVE 0x0001

#pragma pack(push, 2)

/// DLS RIFF chunk header
struct dlsChunk
{
	bsUint32 ckid; //char ckid[4];
	bsUint32 cksz;
	// data[cksz]
};

// Note: this is identical to a 'GUID'
struct dlsID
{
	bsUint32 data1;
	bsUint16  data2;
	bsUint16 data3;
	bsUint8  data4[8];
};

/// DLS version record
struct dlsVersion
{
	bsUint32 dwVersionMS;
	bsUint32 dwVersionLS;
};

/// DLS connection record
struct dlsConnection
{
	bsUint16 source;
	bsUint16 control;
	bsUint16 destination;
	bsUint16 transform;
	bsInt32  scale;
};

/// DLS Level 1 Articulation Data
struct dlsConnectionList
{
	bsUint32 size; // size of the connection list structure
	bsUint32 connections; // count of connections in the list
};

///////////////////////////////////////////////////////////////////////////
// Generic type defines for regions and instruments
///////////////////////////////////////////////////////////////////////////

/// DLS Region key range.
struct dlsRgnRange
{
	bsUint16 low;
	bsUint16 high;
};

#define F_INSTRUMENT_DRUMS 0x80000000
struct dlsMidiLocale
{
	bsUint32 bank;
	bsUint32 instrument;
};

///////////////////////////////////////////////////////////////////////////
// Header structures found in an DLS file for collection, instruments, and
// regions.
///////////////////////////////////////////////////////////////////////////
#define F_RGN_OPTION_SELFNONEXCLUSIVE 0x0001
/// DLS Region header
struct dlsRgnHeader
{
	dlsRgnRange rangeKey; // Key range
	dlsRgnRange rangeVel; // Velocity Range
	bsUint16    options;  // Synthesis options for this range
	bsUint16    keyGroup; // Key grouping for non simultaneous play
	                      // 0 = no group, 1 up is group
	                      // for Level 1 only groups 1-15 are allowed
	bsUint16    exclusive; // Level 2 only
};

/// DLS Instrument header record.
struct dlsInstHeader
{
	bsUint32 regions; // Count of regions in this instrument
	dlsMidiLocale locale; // Intended MIDI locale of this instrument
};

/// DLS header record.
struct dlsHeader
{
	bsUint32 instruments; // Count of instruments in the collection
};

//////////////////////////////////////////////////////////////////////////////
// definitions for the Wave link structure
//////////////////////////////////////////////////////////////////////////////
// **** For level 1 only WAVELINK_CHANNEL_MONO is valid ****
// ulChannel allows for up to 32 channels of audio with each bit position
// specifiying a channel of playback
#define WAVELINK_CHANNEL_LEFT 0x0001l
#define WAVELINK_CHANNEL_RIGHT 0x0002l
#define F_WAVELINK_PHASE_MASTER 0x0001
struct dlsWaveLink
{
	// any paths or links are stored right after struct
	bsUint16 options;     // options flags for this wave
	bsUint16 phaseGroup;  // Phase grouping for locking channels
	bsUint32 channel;     // channel placement
	bsUint32 tableIndex;  // index into the wave pool table, 0 based
};

#define POOL_CUE_NULL 0xffffffffl
struct dlsPoolCue
{
	// ULONG ulEntryIndex; // Index entry in the list
	bsUint32 offset; // Offset to the entry in the list
};

struct dlsPoolTable
{
	bsUint32 size; // size of the pool table structure
	bsUint32 cues; // count of cues in the list
};

//////////////////////////////////////////////////////////////////////////////
// Structures for the "wsmp" chunk
//////////////////////////////////////////////////////////////////////////////
#define F_WSMP_NO_TRUNCATION 0x0001l
#define F_WSMP_NO_COMPRESSION 0x0002l
/// DLS sample data
struct dlsSample
{
	bsUint32 size;
	bsUint16 unityNote;   // MIDI Unity Playback Note
	bsInt16  fineTune;    // Fine Tune in log tuning
	bsInt32  attenuation; // Overall Attenuation to be applied to data
	bsUint32 options;     // Flag options
	bsUint32 sampleLoops; // Count of Sample loops, 0 loops is one shot
};

// This loop type is a normal forward playing loop which is continually
// played until the envelope reaches an off threshold in the release
// portion of the volume envelope
#define WLOOP_TYPE_FORWARD 0
/// DLS loop points record.
struct dlsLoop
{
	bsUint32 size;
	bsUint32 type; // Loop Type
	bsUint32 start; // Start of loop in samples
	bsUint32 length; // Length of loop in samples
};

/// DLS wavetable data format; same as FORMATETC.
struct dlsWaveFmt
{
	bsUint16 fmtTag; 
	bsUint16 channels;
	bsUint32 sampleRate;
	bsUint32 avgBytesPerSec;
	bsUint16 blockAlign;
	bsUint16 bitsPerSamp;
};

#pragma pack(pop)
//@}
#endif
