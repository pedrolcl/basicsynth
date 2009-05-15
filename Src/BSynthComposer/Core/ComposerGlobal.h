
#include <stdlib.h>
#include <math.h>

#if _WIN32
#define snprintf _snprintf
#endif

#include <BasicSynth.h>
#include <SynthString.h>

#ifdef WIN32
#include <dsound.h>
#include <WaveOutDirect.h>
#include <Player.h>
#endif
#include <Instruments.h>
#include <NLConvert.h>
