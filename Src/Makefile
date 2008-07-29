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
include BasicSynth.cfg

BSMODULES= \
common \
instruments \
notelist \
examples \
bsynth

all: chkdirs $(BSMODULES)
	@echo All done

chkdirs:
	test -d $(BSBIN) || mkdir $(BSBIN)
	test -d $(BSLIB) || mkdir $(BSLIB)
	
clean:
	cd Common; make clean
	cd Instruments; make clean
	cd Notelist; make clean
	cd Examples; make clean
	cd BSynth; make clean
	
common:
	cd Common; make $(MAKEFLAGS)

instruments:
	cd Instruments; make $(MAKEFLAGS)

notelist:
	cd Notelist; make $(MAKEFLAGS)

examples:
	cd Examples; make $(MAKEFLAGS)

bsynth:
	cd BSynth; make $(MAKEFLAGS)

