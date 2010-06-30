/// @file Docpages.h Documentation
// This file is used to create documentation pages with Doxygen 
// and does not need to be included in compiles.

/*! \mainpage BasicSynth Synthesizer Library

\section secIntro Introduction

The \e BasicSynth Common library classes provide the signal generation 
and processing foundation for the synthesizer. Much of the implementation 
is contained in the header files located in the \e Include directory. 
The file \e BasicSynth.h includes all of the header files, or individual files 
can be included as needed. The methods are typically short, and placing them
in the header allows the compiler to optimally expand the code as inline functions. 
Longer functions, and those that are not time critical, are located in the 
\e Src/Common directory and compiled into the Common library as object modules.

\section secDirs Directory Structure

\image html Structure.jpg

<table border="1" cellspacing="0">
<tr style="background-color:black;color:white;"><td>Directory</td><td>Content</td></tr>
<tr><td>Bin</td><td>Executable images</td></tr>
<tr><td>Lib</td><td> Object file libraries</td></tr>
<tr><td>Include</td><td> Source include files. Much of the BasicSynth sound generation code is contained in these files</td></tr>
<tr><td>Documents</td><td> Various documents.</td></tr>
<tr><td>Doxygen</td><td> Doxygen project files; used to generate library documentation.</td></tr>
 <tr> <td >OpenSource</td><td >Open source libraries and include files. These are only used for Windows builds of BSynthComposer.</td></tr>
<tr><td >OpenSource/scintilla</td><td >Header files and VS 2008 project files to build scintilla for BasicSynth. By default, 
the BSynthComposer looks here for the header files.
If scintilla is installed already, change the project file for BSynthComposer to reference the include directory.</td></tr>
<tr><td >OpenSource/WTL80</td><td >WTL files used by BasicSynth. By default, the BSynthComposer project looks here for the header files.</td></tr>
<tr><td >OpenSource/fltk-1.1.10</td><td>FLTK files used by BSynthComposer. By default, the FLTK project on Windows looks here for the header files  and libraries.  </td></tr>
<tr><td>Src</td><td>Source code</td></tr>
<tr><td>Src/Common</td><td>Common source, mostly implementation of sequencers and file I/O</td></tr>
<tr><td>Src/Instruments</td><td>Source to the BasicSynth instruments collection library.</td></tr>
<tr><td>Src/Notelist</td><td> Source to the notelist parser and interpreter library.</td></tr>
<tr><td>Src/Examples</td><td> Source to the example programs. Each sub-directory contains one example program. The theory and algorithm of each program is explained in the associated book chapter.</td></tr>
<tr><td>Src/Utilities</td><td> Source to the utility programs. Each sub-directory contains one utility program. These are Windows-only GUI programs that use the BasicSynth library and demonstrate various synthesis techniques.</td></tr>
<tr><td>BSynth</td><td> Command line version of the synthesizer.</td></tr>
<tr><td>Src/BSynthComposer</td><td>GUI version of the synthesizer. </td> </tr>
<tr><td>Src/BSynthComposer/Core</td><td>Platform independent parts.</td></tr>
<tr><td>Src/BSynthComposer/Forms</td><td>XML files defining editor forms.</td></tr>
<tr><td>Src/BSynthComposer/mswin</td><td>MS-Windows specific code.</td></tr>
<tr><td>Src/BSynthComposer/Help</td><td>Help files in HTML format and compiled help project for Windows.</td></tr>
<tr><td>Src/BSynthComposer/fltk</td><td>FLTK specific code.</td></tr>
</table>

To build your own synthesis program with \e BasicSynth you need at a minimum the Include files
and the Common library (\e Lib/Common.lib). The Include directory and the Common library provide
the sound generation, wave file and sequencer code. The header file \e BasicSynth.h includes all of the library.
If you want to use the \e BasicSynth instruments collection in your program, you must also include
the header file \e Src/Instruments/Instruments.h and link the Lib/Instruments.lib library.
To include the Notelist score processor, you must include the header file \e NLConvert.h 
from the \e Src/Notelist directory and link the \e Lib/Notelist.lib library.

\note Libraries on Linux have the \e a extension instead of \e lib. Debug libraries have a \e D
at the end of the name.

\section xmlwrap XML Wrapper classes

The library includes an XML wrapper class to hide the XML implementation. Four implementations are
available:
 - define USE_MSXML to use the MS Windows msxml dll
 - define USE_LIBXML to use the libxml2 library
 - define USE_TINYXML to use the tinyxml library
 - define none to create a dummy XML class

The default is to use the TINYXML code, included in the library source. To use libxml2, you must
have the library available on the linker library path.

\section secBuildW Building the Source on Windows with Visual Studio

There are several solution files that can be built depending on what you want to do. 
Default solution files are for Visual Studio 2008 (i.e. version 9). Solution files with 2005
in the name are for VisualStudio 2005 (version 8). Visual Studio 6 project files (.dsp) are
also included. However, the older project files are not always kept up to date and you may
need to update them. The VirtualKBD utility and BSynthComposer will not build with version 6 
as they need the gdi+ libraries. 
If you have a different version of the compiler, you will have to produce the project and/or solution files from one
of the supplied versions.

Each project provides four targets:

 - Release Win32 - 32-bit version without debug info. 
 - Debug Win32 - 32-bit version with debugging. 
 - Release x64 - 64-bit version without debug info. 
 - Debug x64 - 64-bit version with debug info.

Output of the 32-bit release version is in Bin and Lib. The 64-bit version produces output in Bin64 and Lib64.

To build everything, use the solution file \e Src/BasicSynth.sln. To build just the libraries,
use the solution files in \e Src/Common, \e Src/Instruments, and \e Src/Notelist directories. 
To build the Example programs, use the solution file in \e Src/Examples. 
This builds all the example programs and libraries. 
To build the utilities, use the solution file in \e Src/Utilities.
To build the stand-alone command line synthesizer, build the project in \e Src/BSynth.
To build the GUI synthesizer, build the project in \e Src/BSynthComposer. 

The utility programs and VC version of \e BSynthComposer use WTL 8.0, available on sourceforge.net.
The parts used by \e BasicSynth are located in the <em>OpenSource/WTL80</em> directory.
\e BSynthComposer requires the scintilla header files and library DLL. Scintilla is 
also available on sourceforge.net. The parts used by \e BasicSynth are located in the 
<em>OpenSource/scintilla</em> directory. Pre-built DLLs are installed in the \e Bin
and \e Binx64 directories if you download the source .zip file.

The FLTK version of \e BSynthComposer uses the FLTK header and library
files. The parts used by are located in the <em>OpenSource/fltk-1.1.10</em> directory.
Source code is available from the <a href="http://www.fltk.org">FLTK website (fltk.org).</a>

\section secBuildCB Building with Code::Blocks

There are two Code::Blocks workspace files in this release. In the Src directory,
the basicsynth.workspace file will build the libraries and example programs. 
In the \e BSynthComposer/fltk directory the fltk.workspace file will build the
synthesizer program using FLTK. </p>

<p>Most projects contain four targets:</p>
<ol>
<li>Release Win32 - 32-bit Windows version without debug info.</li>
<li>Debug Win32 - 32-bit Windows version with debugging.</li>
<li>Release UNIX - Unix/Linux version without debug info.</li>
<li>Debug UNIX - Unix/Linux with debug info.</p>
</ol>

There is a .cbp project file for each of the libraries, example programs, and FLTK version
of BasicSynth Composer. (The Utilities and MSWIN version of BasicSynth Composer use
ATL and must be compiled with VisualStudio, so no .cbp files are provided.) 

A global variable \e bshome controls the location of files for the build. The first time
you build the basicsynth workspace, Code::Blocks will prompt for the needed value.
You can also set this in advance from the <i>Settings:Global Variables</i> menu.

The \e bshome value is the top directory where BasicSynth is installed.
It's best to put the source in a path without spaces since the MINGW compiler and linker
will have problems otherwise. However, if the path has spaces in it, there are ways
around the problem. For example, on Windows you can create a network share name for the root folder and 
use the UNC path to access it. (e.g. \\\\MYCOMPUTER\\BasicSynth). 

The \e fltk project is split into separate projects, fltk.cbp and fltkwin.cbp. The
Windows version adds a global variable to set the location for the fltk include and library
files. The <i>$(BSHOME)/OpenSource/fltk-1.1.10</i> directory contains the files needed
to build on Windows and is normally the value used for the fltk variable. For Linux, download
and install the FLTK developement package (apt-get install fltk1.1-dev), or download the
source from <a href="http://www.fltk.org">fltk.org</a> and build it. Code::Blocks has
a default macro to include the fltk library.

Note: The pre-built libraries are only included in the .zip source archive, not the tar file.

\section secBuildL Building the Source with Makefiles

There is a master Makefile located in the Src directory that will build all of the system. 
There is also a Makefile in each directory that can be used if you only want to build part
of the system. The file \e BasicSynth.cfg in the Src directory contains the settings for the
compiler and linker. Edit this file to point to the appropriate places. Usually you will
only need to change the location of BSDIR in case you put the source in a directory other 
than your home directory. By default, BasicSynth is built with XML support using the tinyxml 
library. The libxml2 library is also supported. Install the libxml2-dev package and define USE_LIBXML2.

Output is to the directories \e Bin and \e Lib under the \e BasicSynth install directory. (Note the capitalization!)

\section libdoc Building the library documentation

There is a Doxygen project file in the Doxygen directory that can be used to generate
a new copy of this documentation if you modify the library. Check the paths to insure
they point to the correct locations. The instrument diagram files in the Documents 
directory must be manually copied to the output directory indicated in the doxygen project file.

\section license License
The source code to BasicSynth is &copy; 2008,2009, Daniel R. Mitchell and is licensed under the
Creative Commons/GNU-GPL (http://creativecommons.org/licenses/GPL/2.0/). Source code may be
used for non-commercial purposes without further restrictions.

Compositions in the ExampleProjects directory are &copy; Daniel R. Mitchell and are licensed
under the Creative Commons Attribution-Noncommercial-No Derivitave Works license. 
(http://creativecommons.org/licenses/by-nc-nd/3.0/us/</a>)

\page architecture BasicSynth Archtecture
 The BasicSynth library can be used in a variety of contexts with as little or as much
 of the library as needed. However, the library is specifically designed to support
 the architecture used by the BSynth program.

\section archover Overview
BasicSynth is a component-based system intended to be used as a compositional environment. Thus it centers around
the sequencing of a series of events defined by the score and allows a variable set of 
instrument definitions. The sequencer reads the score and invokes one or more synthesis 
instruments to generate samples. Samples are passed through a common mixer and then to the 
output, either a wave file or a sound output device (DAC). The project information and 
parameters component contains general synthesizer information and also allows the sequencer 
to discover the available instruments and scores that are to be used. 

\image html architecture2.jpg

Because it is a component architecture, it is possible to replace any of the components in the 
system with custom versions. So long as the interface to the component is maintained, the
remainder of the system will work without modification.

Since all configuration files are in XML format, a simple text editor may be used to configure the system. 
The \e BasicSynth Composer program implements GUI editors for each part of the system.

<table border="1" cellspacing="0">
<tr style="background-color:black;color:white;"><td>Module</td><td>Description</td></tr>
<tr><td>Sequence</td><td>Generates sample output based on a sequence of events. Events can be read from a file, added directly by a program, or added interactively through a user interface.</td></tr>
<tr><td>Seq. Edit</td><td>Editor (or generator) for sequencer files. Multiple formats can be defined by adding the appropriate file loading code to the sequencer.</td></tr>
<tr><td>Instr.</td><td>An Instrument, or sound generation and modification module. Instruments use generator, filter and effects objects to control sound generation.</td></tr>
<tr><td>Instr. Lib</td><td>File containing instrument configuration.</td></tr>
<tr><td>Instr. Edit</td><td>Editor for instrument definitions. This can be a simple text editor or a complex graphic interface.</td></tr>
<tr><td>Wave Buffer</td><td>Buffer for sample output.</td></tr>
<tr><td>Wave File</td><td>Output to a sound file, usually in WAV format, but possibly in other formats as well.</td></tr>
<tr><td>Play</td><td>Immediate output of samples to the computer's sound output device.</td></tr>
<tr><td>Mixer</td><td>Mixes sounds and applies effects.</td></tr>
<tr><td>Effects</td><td>Processing effects such as reverb, echo, flanger, etc.</td></tr>
<tr><td>Params</td><td>System wide parameters, such as sample rate, wave table length, input and output directories, etc., along with lists of instrument and sequencer files</td></tr>
<tr><td>Prjoject Edit</td><td>Editor for project parameters.</td></tr>
</table>

\section sequencer Sequencer
The sequencer is the synthesizer component that cycles through events, dispatching
each event in the proper order and at the proper time. Originally a sequencer was
a module on an analog synthesizer that could step through a series of control voltages
and produce a trigger at each step. When applied to an oscillator and envelope generator
in combination, the sequencer played a short tune. The addition of microprocessor control
allowed the sequencer to store any series of synthesizer control parameters and playback
the control signals automatically. In a software synthesis system, the sequencer is expanded
to provide the ability to automate timed control of any synthesizer function.

\image html sequencer.jpg

The Sequence File Loader implements functions to read a list of events and event parameters from a file. 
Each event (SeqEvent) includes a unique identifier that indicates which instrument will process the event. 
As each event is loaded, the loader calls the Instrument Manager to instantiate an Event object specific to the target instrument. 
The Instrument Manager uses the instrument identifier to locate information about the instrument in the Instrument Definition Table. 
The Instrument Manager then calls a method on the Instrument interface to instantiate the event object. 
Parameters from the file are added to the event object by calling the set parameter method on the event object. 
The Event object is then added to the Sequencer Loop track event list.

The Sequencer maintains a group of Track objects (SeqTrack). Each Track represents a time ordered list of sequencer events.
Track zero is the master track that is started when the Sequencer is started. Other tracks can be started
and stopped indpendently of the main track. The track is determined by a value in the event object.

Events can also be passed directly to the Sequencer for immediate execution. These events typically come
from a MIDI keyboard, but can be generated from any source that has access to the sequencer. Immediate
events do not have a start time, and are removed from the event list as soon as they are played.

The Instrument Definition Table contains entries to identify each instrument in the synthesizer. 
This list can be dynamically created when the synthesizer is loaded, or can be compiled into the program. 
Typically, the table is loaded from a file containing instrument types and default settings. 
The table contains the address of a factory for each instrument and event type.
The factory is the code that "manufactures" (i.e., instantiates) the instrument or event, and is defined as a part
of the instrument object implementation. By storing the information in a table created at runtime, the synthesizer
can have access to an unlimited number of instrument definitions.

Once the sequence is loaded, playback is started by calling a method on the Sequencer Loop. 
The Sequencer Loop calls the start method on the Instrument Manager. This allows the Instrument Manager to 
pre-allocate instruments if needed and also to initialize the Mixer and Wave File outputs. 
The Sequencer Loop scans the list of events associated with each Track until the start time of an event is reached. 
The Sequencer Loop then invokes the Instrument Manager to allocate an instance of the instrument identified by the
Event object and receives back a reference to the Instrument interface, which is stored in the Active Event list. 
The Event is then passed to the instrument using the start method. 

The instrument associated with the event is called for each sample time until the duration of the event is completed. 
The Sequencer Loop then calls the instrument stop method to signal the instrument to end. 
However, the instrument remains active, and the tick method continues to be called, so long as the \e isfinished method returns false. 
This allows the instrument to produce samples beyond the limit of its event duration if necessary. 
For example, the instrument can wait to begin the release portion of the envelope until a stop signal is received, 
and processors such as delay lines can empty the delay line before terminating. When the instrument indicates it has finished, 
the Sequencer Loop removes the instrument from the active list and notifies the Instrument Manager that the instrument is no 
longer in use by invoking the deallocate method. The Instrument Manager can then either destroy the instrument instance, or recycle it if possible.

When the tick method is called on an instrument, the instrument produces a sample and passes it to the Instrument Manager 
which then adds the sample to the appropriate Mixer input. However, an instrument does not need to produce samples. 
An instrument can also be defined to control other synthesizer functions, such as panning, mixer levels, etc. 
When all active instruments have been processed, the Sequencer Loop calls the tick method on the Instrument Manager. 
The output of the Mixer is then retrieved and passed to the Wave File output buffer.

Once all events have been completed, the stop method on the Instrument Manager is called and the Sequencer Loop stops.

This design allows for nearly unlimited flexibility in the sequencer. We only need a few event types and can sequence any number of instruments, 
processors and controllers without having to build the knowledge of the instruments into the sequencer. Any synthesizer function that can be 
performed with the stop, start, change, and tick signals can be sequenced. The key to this design lies in the flexibility of the event object, 
the dynamic allocation of instrument instances, and the abstract instrument interface.

\page pgbsynth BSynth synthesizer

\htmlinclude BSynth.html

\page instrDoc BasicSynth Instruments

\htmlinclude Instruments.html

\page notelist Notelist Score Script

\htmlinclude Notelist.html

\defgroup grpGeneral General Classes

\defgroup grpIO File I/O Classes

\defgroup grpOscil Oscillators

\defgroup grpNoise Noise Generators

\defgroup grpEnv Envelope Generators

\defgroup grpFilter Filters

\defgroup grpDelay Delay Lines

\defgroup grpMix Mixing and Effects

\defgroup grpSeq Sequencing

\defgroup grpMIDI MIDI specific classes

\defgroup grpInstrument Synthesis Instrument Classes

\defgroup grpSoundbank SoundFont(R) and DLS File Classes

*/

