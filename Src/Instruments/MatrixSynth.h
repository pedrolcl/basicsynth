// MatrixSynth.h: interface for the MatrixSynth class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MATRIXSYNTH_H_)
#define _MATRIXSYNTH_H_

#include "LFO.h"

// MATGEN sets the size of the matrix. We can go up to 16 oscillators because 
// we have 16 available bits in the toneFlags member. Eight is usually enough.
#define MATGEN     8
#define TONE_OUT_BITS 0x0000FFFF
#define TONE_ON      0x00000001
#define TONE_OUT     0x00000002
#define TONE_LFOIN   0x00000004
#define TONE_PBIN    0x00000008
#define TONE_FX1OUT  0x00000010
#define TONE_FX2OUT  0x00000020
#define TONE_FX3OUT  0x00000040
#define TONE_FX4OUT  0x00000080
//#define TONE_XXX   0x00007F00 <= available
#define TONE_PAN     0x00008000
#define TONE_OUTANY  0x000000F2

#define TONE_MOD_BITS 0xFFFF0000
#define TONE_MOD1IN  0x00010000
#define TONE_MOD2IN  0x00020000
#define TONE_MOD3IN  0x00040000
#define TONE_MOD4IN  0x00080000
#define TONE_MOD5IN  0x00100000
#define TONE_MOD6IN  0x00200000
#define TONE_MOD7IN  0x00400000
#define TONE_MOD8IN  0x00800000
//#define TONE_MODXX   0xFF000000 <= available
#define TONE_MODANY  0x00FF0004

class MatrixTone
{
private:
	GenWaveWT  osc;    // signal oscillator
	FrqValue frqMult;  // Frequency multiplier
	AmpValue volLvl;   // Signal out level
	AmpValue modLvl;   // Modulation in level
	PhsAccum modRad;   // in radians
	AmpValue lfoLvl;   // LFO input level
	AmpValue pbLvl;    // Pitch Bend level
	AmpValue panSet;   // Pan setting -1,+1
	AmpValue panLft;   // Pan left 0-1
	AmpValue panRgt;   // Pan right 0-1
	AmpValue fx1Lvl;   // Fx1 send level
	AmpValue fx2Lvl;   // Fx2 send level
	AmpValue fx3Lvl;   // Fx3 send level
	AmpValue fx4Lvl;   // fx4 send level
	bsUint32 toneFlags;  // TONE_* bits
	bsUint16 envIndex; // envelope index 0-7

public:
	MatrixTone();
	~MatrixTone();

	void Copy(MatrixTone *tp);
	void Start(FrqValue frqBase);
	void AlterFreq(FrqValue frqBase);
	void PhaseModWT(PhsAccum phs);
	AmpValue Gen();

	int Load(XmlSynthElem *elem);
	int Save(XmlSynthElem *elem);

	friend class MatrixSynth;
};

class MatrixSynth : public Instrument
{
private:
	int chnl;
	FrqValue frq;
	AmpValue vol;
	MatrixTone   gens[MATGEN];
	EnvGenSegSus envs[MATGEN];    // envelope A,D1,D2,S,R
	bsUint16 envUsed;

	LFO lfoGen;

	int lfoOn;
	int panOn;
	int fx1On;
	int fx2On;
	int fx3On;
	int fx4On;
	bsUint32 allFlags;

	InstrManager *im;

	int LoadEnv(XmlSynthElem *elem);
	int SaveEnv(XmlSynthElem *elem, int en);
	void SetParams(VarParamEvent *evt);

public:
	MatrixSynth();
	virtual ~MatrixSynth();
	static Instrument *MatrixSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *MatrixSynthEventFactory(Opaque tmplt);
	void Copy(MatrixSynth *tp);
	void Start(SeqEvent *evt);
	void Param(SeqEvent *evt);
	void Stop();
	void Tick();
	int  IsFinished();
	void Destroy();

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
};

#endif
