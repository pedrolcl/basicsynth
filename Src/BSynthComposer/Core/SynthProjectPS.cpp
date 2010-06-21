//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

/////////////////////////////////////////////////////////////////////////////
// Platform-specific project functions
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <WaveOutDirect.h>
#endif
#ifdef UNIX
#include <WaveOutALSA.h>
#endif

int SynthProject::SetupSoundDevice(float latency)
{
	if (wop == NULL)
	{
#ifdef _WIN32
//		WaveOutDirect *wvd = new WaveOutDirect;
		WaveOutDirectI *wvd = new WaveOutDirectI;
		if (wvd->Setup(prjOptions.dsoundHWND, latency, 4, prjOptions.waveID))
		{
			delete wvd;
			return -1;
		}
#endif
#ifdef UNIX
		WaveOutALSA *wvd = new WaveOutALSA;
		if (wvd->Setup(prjOptions.waveDevice, latency, 1))
		{
			delete wvd;
			return -1;
		}
#endif
		wop = wvd;
	}
	return 0;
}
