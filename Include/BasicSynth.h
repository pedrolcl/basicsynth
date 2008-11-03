/// @file BasicSynth.h Master include file for BasicSynth common library.
#include <SynthDefs.h>
#include <SynthString.h>
#include <SynthList.h>
#include <SynthFile.h>
#include <WaveTable.h>
#include <WaveFile.h>

#include <GenWave.h>
#include <GenWaveWT.h>
#include <GenNoise.h>
#include <GenWaveX.h>

#include <EnvGen.h>
#include <EnvGenSeg.h>

#include <BiQuad.h>
#include <AllPass.h>
#include <Filter.h>
#include <DynFilter.h>

#include <DelayLine.h>
#include <Flanger.h>
#include <Reverb.h>

#include <Mixer.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <Instrument.h>
#include <Sequencer.h>
#include <SequenceFile.h>

/*! \mainpage BasicSynth Sound Generation Library

\section secIntro Introduction

The \e BasicSynth Common library classes provide the signal generation 
and processing foundation for the synthesizer. Much of the implementation 
is contained in the header files located in the Include directory. 
The file BasicSynth.h includes all of the header files, or individual files 
can be included as needed. The methods are typically short, and placing them
in the header allows the compiler to optimally expand the code as inline functions. 
Longer functions, and those that are not time critical, are located in the 
Src/Common directory and compiled into the Common library as object modules.

\section secDirs Directory Structure
<table>
<tr><td>Directory</td><td> Usage</td></tr>
<tr><td>Bin</td><td>Executable images</td></tr>
<tr><td>Lib</td><td> Object file libraries</td></tr>
<tr><td>Include</td><td> Source include files. Much of the BasicSynth sound generation code is contained in these files</td></tr>
<tr><td>Documents</td><td> Various documents.</td></tr>
<tr><td>Src</td><td>Source code</td></tr>
<tr><td>Src/Common</td><td>Common source, mostly implementation of sequencers and file I/O</td></tr>
<tr><td>Src/Instruments</td><td>Source to the BasicSynth instruments collection library.</td></tr>
<tr><td>Src/Notelist</td><td> Source to the notelist parser and interpreter library.</td></tr>
<tr><td>Src/Examples</td><td> Source to the example programs. Each sub-directory contains one example program. The theory and algorithm of each program is explained in the associated book chapter.</td></tr>
<tr><td>Src/Utilities</td><td> Source to the utility programs. Each sub-directory contains one utility program. These are Windows-only GUI programs that use the BasicSynth library and demonstrate various synthesis techniques.</td></tr>
<tr><td>BSynth</td><td> Command line version of the synthesizer.</td></tr>
</table>

To build your own synthesis program with \e BasicSynth you need at a minimum the Include files
and the Common library (Lib/Common.lib). The Include directory and the Common library provide
the sound generation, wave file and sequencer code. The header file BasicSynth.h includes all of the library.
If you want to use the \e BasicSynth instruments collection in your program, you must also include
the header file Src/Instruments/Instruments.h and link the Lib/Instruments.lib library.
To include the \b Notelist score processor, you must include the header file NLConvert.h 
from the Src/Notelist directory and link the Lib/Notelist.lib library.

\note Libraries on Linux have the \e a extension instead of \e lib. Debug libraries have a \e D
at the end of the name.

\section secBuildW Building the Source on Windows

There are several solution files that can be built depending on what you want to do. 
All solution files are for Visual Studio 2008 (i.e. version 9). If you have a different 
version of the compiler, you will have to reproduce the project and/or solution files.
To build everything, use the solution file Src/BasicSynth.sln. To build just the libraries,
use the solution files in Src/Common, Src/Instruments, and Src/Notelist directories. 
To build the Example programs, use the solution file in Src/Examples. 
This builds all the example programs and libraries. 
To build the utilities, use the solution file in Src/Utilities.
To build the stand-alone command line synthesizer, build the project in Src/BSynth.

Each project provides four targets:

 - Release Win32 - 32-bit version without debug info. 
 - Debug Win32 - 32-bit version with debugging. 
 - Release x64 - 64-bit version without debug info. 
 - Debug x64 - 64-bit version with debug info.

Output of the 32-bit release version is in Bin and Lib. The 64-bit version produces output in Bin64 and Lib64.

\section secBuildL Building the Source on Linux

There is a master Makefile located in the Src directory that will build all of the system. 
There is also a Makefile in each directory that can be used if you only want to build part
of the system. The file BasicSynth.cfg in the Src directory contains the settings for the
compiler and linker. Edit this file to point to the appropriate places. Usually you will
only need to change the location of BSDIR in case you put the source in a directory other 
than your home directory. By default, BasicSynth is built with XML support using the libxml2 
library. You should install the libxml2-dev package first before building BasicSynth. 
If you do not need to use the project/instrument file loading code, there is a null 
implementation of the XmlWrapper class that can be used instead. The file is Src/Common/XmlWrapN.cpp 
and should be compiled and added to the Common library in place of XmlWrapU.cpp

Output is to the directories Bin and Lib under the BasicSynth install directory. (Note the capitalization!)

\defgroup grpGeneral General Classes

\defgroup grpIO File I/O Classes

\defgroup grpOscil Oscillators

\defgroup grpNoise Noise Generators

\defgroup grpEnv Envelope Generators

\defgroup grpFilter Filters

\defgroup grpDelay Delay Lines

\defgroup grpMix Mixing and Effects

\defgroup grpSeq Sequencing
*/
