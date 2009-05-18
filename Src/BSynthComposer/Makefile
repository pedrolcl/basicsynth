###########################################################################
# Master makefile for BasicSynth libraries and examples
#
# "make all" makes all modules
# "make [module]" to make one specific example
# "make clean" removes libraries and executable images 
#
# Note: There seems to be a bug in GNU make when descending
# into sub-makes that causes a 'w' flag to be appended to
# $MAKEFLAGS. Use 'make --no-print-directory' if that happens.
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
include ../BasicSynth.cfg

BSMODULES= \
core \
fltkx

all: chkdirs $(BSMODULES)
	@echo BSynthComposer done

chkdirs:
	test -d Lib || mkdir Lib

clean:
	cd Core; make clean
	cd fltk; make clean
	
core:
	cd Core; make $(MAKEFLAGS)

fltkx:
	cd fltk; make $(MAKEFLAGS)
