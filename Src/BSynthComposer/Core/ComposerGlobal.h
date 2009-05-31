//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
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
