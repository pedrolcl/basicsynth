//////////////////////////////////////////////////////////////////////
// Required include files for instruments
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#if !defined(_INCLUDES_H_)
#define _INCLUDES_H_

#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <BasicSynth.h>

struct InstrParamMap
{
	char *name;
	bsInt16 id;
};

extern bsInt16 SearchParamID(const char *name, InstrParamMap *map, int n);
extern const char *SearchParamName(bsInt16 id, InstrParamMap *map, int count);

#endif
