///////////////////////////////////////////////////////////////
//
// BasicSynth - Wave Table set
//
// Class to hold wave tables;
//
// A single global instance of this class must be defined (see below)
// if you intend to use the GenWaveWT class and its derivations.
// 
// N.B.: The Init() method must be called
//       before using the wave table generators. The generators
//       do not check for null tables in order to optimize
//       performance. If you fail to call Init(), things will
//       crash very quickly...
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _WAVETABLE_H_
#define _WAVETABLE_H_

#define WT_SIN 0
#define WT_SAW 1
#define WT_SQR 2
#define WT_TRI 3
#define WT_PLS 4
#define WT_SAWL 5
#define WT_SQRL 6
#define WT_TRIL 7
#define WT_SAWP 8
#define WT_TRIP 9
#define WT_USR(n) ((n)+10)

class WaveTableSet
{
public:
	AmpValue *wavSin;
	AmpValue *wavSqr;
	AmpValue *wavSaw;
	AmpValue *wavTri;
	AmpValue *wavPls;
	AmpValue *lfoSaw;
	AmpValue *lfoTri;
	AmpValue *lfoSqr;
	AmpValue *posSaw;
	AmpValue *posTri;
	AmpValue **wavSet;
	bsInt32 wavTblMax;

	WaveTableSet()
	{
		wavSin = NULL;
		wavSqr = NULL;
		wavSaw = NULL;
		wavTri = NULL;
		wavPls = NULL;
		lfoSqr = NULL;
		lfoSaw = NULL;
		lfoTri = NULL;
		posSaw = NULL;
		posTri = NULL;
		wavSet = NULL;
		wavTblMax = 0;
	}

	~WaveTableSet()
	{
		DestroyTables();
	}

	void DestroyTables()
	{
		if (wavSet)
		{
			for (bsInt32 i = 0; i < wavTblMax; i++)
			{
				if (wavSet[i] != NULL)
					delete wavSet[i];
			}
			delete wavSet;
		}
		wavSet = NULL;
	}

	// Initialize the default wavetables. 
	// This method initializes the default wave tables.
	// The NUM_PARTS constant is set to allow oscillator frequencies 
	// up to 2 octaves above middle C.
	// Higher pitches will produce alias frequencies.
	// For higher partials, or higher pitches, use GenWaveSum
	void Init(bsInt32 wtUsr = 0)
	{
		DestroyTables();

		wavTblMax = WT_USR(wtUsr);
		wavSet = new AmpValue*[wavTblMax];
		bsInt32 index;
		for (index = 0; index < wavTblMax; index++)
			wavSet[index] = NULL;

		size_t allocSize = synthParams.itableLength+1;
		wavSin = new AmpValue[allocSize];
		wavSaw = new AmpValue[allocSize];
		wavSqr = new AmpValue[allocSize];
		wavTri = new AmpValue[allocSize];
		wavPls = new AmpValue[allocSize];
		lfoSaw = new AmpValue[allocSize];
		lfoSqr = new AmpValue[allocSize];
		lfoTri = new AmpValue[allocSize];
		posSaw = new AmpValue[allocSize];
		posTri = new AmpValue[allocSize];

#define NUM_PARTS 16
		double phsInc[NUM_PARTS];
		double phsVal[NUM_PARTS];
		int partNum;
		int partMax = 1;

		phsInc[0] = twoPI / (double) synthParams.ftableLength;
		phsVal[0] = 0.0f;
		for (partNum = 1; partNum < NUM_PARTS; partNum++)
		{
			phsInc[partNum] = phsInc[0] * (partNum+1);
			phsVal[partNum] = 0.0;
			if (phsInc[partNum] < PI)
				partMax++;
		}
		
		double sawValue = 0.0;
		double sawPeak = 0.0;

		double sqrValue;
		double sqrPeak = 0.0;

		double triValue;
		double triPeak = 0.0;

		double plsValue;
		double plsPeak = 0.0;

		double sigK = PI / partMax;
		double sigN;
		double sigma;
		double amp;

		double value;
		double partP1;

		for (index = 0; index < synthParams.itableLength; index++)
		{
			value = sin(phsVal[0]);
			wavSin[index] = (AmpValue) value;

			// direct calculation for LFO, not BW limited..
			if (phsVal[0] > PI)
				lfoSqr[index] = -1.0;
			else
				lfoSqr[index] = 1.0;

			lfoSaw[index] = (phsVal[0] / PI) - 1;
			posSaw[index] = (phsVal[0] / twoPI);
			lfoTri[index] = 1 - (2/PI * fabs(phsVal[0] - PI));
			posTri[index] = 1 - (fabs(phsVal[0] - PI) / PI);

			phsVal[0] += phsInc[0];

			// Calculate these by inverse Fourier transform
			sawValue = value;
			sqrValue = value;
			triValue = value;
			plsValue = value;

			sigN = sigK;
			partP1 = 2;
			for (partNum = 1; partNum < partMax; partNum++)
			{
				// no adjustment:
				//amp = 1.0 / partP1;
				// Adjustment with Lanczos sigma to minimize Gibbs phenomenon
				sigma = sin(sigN) / sigN; 
				plsValue += sin(phsVal[partNum]) * sigma;
				amp = sigma / partP1;
				sigN += sigK;
				value = sin(phsVal[partNum]) * amp;
				if (!(partNum & 1))
				{
					sawValue += value;
					sqrValue += value;
					triValue += ((cos(phsVal[partNum]) * sigma) / (partP1 * partP1));
				}
				else
					sawValue -= value;
				phsVal[partNum] += phsInc[partNum];
				partP1 += 1.0;
			}
			if (fabs(sawValue) > sawPeak)
				sawPeak = fabs(sawValue);
			if (fabs(sqrValue) > sqrPeak)
				sqrPeak = fabs(sqrValue);
			if (fabs(triValue) > triPeak)
				triPeak = fabs(triValue);
			if (fabs(plsValue) > plsPeak)
				plsPeak = fabs(plsValue);
			wavSaw[index] = (AmpValue) sawValue;
			wavSqr[index] = (AmpValue) sqrValue;
			wavTri[index] = (AmpValue) triValue;
			wavPls[index] = (AmpValue) plsValue;
		}

		// Normalize summed values
		for (index = 0; index < synthParams.itableLength; index++)
		{
			wavSaw[index] = wavSaw[index] / (AmpValue) sawPeak;
			wavSqr[index] = wavSqr[index] / (AmpValue) sqrPeak;
			wavTri[index] = wavTri[index] / (AmpValue) triPeak;
			wavPls[index] = wavPls[index] / (AmpValue) plsPeak;
		}

		// Set gaurd point for interpolation/round-up
		wavSin[synthParams.itableLength] = wavSin[0];
		wavSaw[synthParams.itableLength] = wavSaw[0];
		wavSqr[synthParams.itableLength] = wavSqr[0];
		wavTri[synthParams.itableLength] = wavTri[0];
		wavPls[synthParams.itableLength] = wavPls[0];
		lfoSaw[synthParams.itableLength] = lfoSaw[0];
		lfoSqr[synthParams.itableLength] = lfoSqr[0];
		lfoTri[synthParams.itableLength] = lfoTri[0];
		posSaw[synthParams.itableLength] = posSaw[0];
		posTri[synthParams.itableLength] = posTri[0];

		wavSet[WT_SIN] = wavSin;
		wavSet[WT_SAW] = wavSaw;
		wavSet[WT_SQR] = wavSqr;
		wavSet[WT_TRI] = wavTri;
		wavSet[WT_PLS] = wavPls;
		wavSet[WT_SAWL] = lfoSaw;
		wavSet[WT_SQRL] = lfoSqr;
		wavSet[WT_TRIL] = lfoTri;
		wavSet[WT_SAWP] = posSaw;
		wavSet[WT_TRIP] = posTri;
	}

	// Set an indexed wave table.
	// n -> table index
	// nparts -> number of partials
	// mul[] -> array of partial numbers, NULL if all partials through n included
	// amp[] -> array of partial amplitudes
	// phs[] -> array of phase offsets, NULL for all 0 phase
	// gibbs -> turn gibbs correction on/off
	int SetWaveTable(bsInt32 n, bsInt32 nparts, bsInt32 *mul, double *amp, double *phs, int gibbs)
	{
		if (n < 0 || n >= wavTblMax || amp == NULL)
			return -1;

		AmpValue *wavTable = new AmpValue[synthParams.itableLength+1];
		if (wavTable == NULL)
			return -1;
		if (wavSet[n])
			delete wavSet[n];
		wavSet[n] = wavTable;

		double *phsVal = new double[nparts];
		double *phsInc = new double[nparts];

		double incr = twoPI / (double) synthParams.ftableLength;
		int partNum;
		int partMax = 0;
		for (partNum = 0; partNum < nparts; partNum++)
		{
			if (mul != NULL)
				phsInc[partNum] = incr * mul[partNum];
			else
				phsInc[partNum] = incr * (partNum + 1);
			if (phsInc[partNum] < PI)
				partMax++;
			if (phs)
				phsVal[partNum] = phs[partNum];
			else
				phsVal[partNum] = 0.0;
		}

		double value;
		double maxvalue = 0.00001;
		double sigK = PI / partMax;
		double sigN;

		int index = 0;
		for (index = 0; index < synthParams.itableLength; index++)
		{
			value = 0;
			sigN = sigK;
			for (partNum = 0; partNum < partMax; partNum++)
			{
				if (amp[partNum] != 0)
				{
					// no adjustment:
					if (!gibbs || partNum == 0)
						value += sin(phsVal[partNum]) * amp[partNum];
					else
					{
						// Adjustment with Lanczos sigma to minimize Gibbs phenomenon
						value += sin(phsVal[partNum]) * (sin(sigN) / sigN) * amp[partNum];
						sigN += sigK;
					}
					phsVal[partNum] += phsInc[partNum];
				}
			}
			wavTable[index] = (AmpValue) value;
			if (fabs(value) > maxvalue)
				maxvalue = fabs(value);
		}

		// Normalize summed values
		for (index = 0; index < synthParams.itableLength; index++)
			wavTable[index] = wavTable[index] / (AmpValue) maxvalue;

		wavTable[synthParams.itableLength] = wavTable[0];
		delete phsVal;
		delete phsInc;

		return 0;
	}
};

// this global is defined and initialized by the InitSynthesizer method.
// it is shared by all oscialltors
extern WaveTableSet wtSet;

#endif
