This directory contains .h files and pre-built libraries from open source code
libraries that are needed to build the Utilities and BSynthComposer on Windows. 

The scintilla directory contains .h files needed to build BSynthComposer\mswin.
The pre-built SciLex.dll files are in the basicsynth Bin and Binx64 directories. 
These were built from version 2.12 with VS 2008 using the project files located
in the scintilla directory. Source code and up-to-date info is available at the
scintilla website (http://www.scintilla.org).

The WTL80 directory contains the .h files used to build BSynthCompser\mswin and
the projects in the Utilities source directory. Source code is available on
sourceforge under the WTL project (http://sourceforge.net/projects/wtl/).

The fltk-1.1.10\FL directory contains .h files used to build BSynthComposer\fltk.
The fltk-1.1.10\lib subdirectory contains pre-built library files for both VC and MINGW. 
Source code and up-to-date info is available on the fltk website (http://www.fltk.org/).

If you need to rebuild the libraries, download the source and extract it here.
If you already have these on the computer, you can edit the appropriate 
basicsynth project files and point to the location of the .h files and libraries.

On Linux, you can get the fltk pieces with "apt-get install libfltk1.1-dev" or the
equivalent for your installation. Note that this may not be the latest version.
BasicSynth should build with either 1.1.9 or 1.1.10 without problems. For newer
versions - you are on your own. Check http://www.fltk.org/. WTL and Scintilla
are not used for the Linux version of BasicSynth.
