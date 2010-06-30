###########################################################
# Makefile for GMSynth library
# TODO: This should build a shared library by linking with Common and Instruments
###########################################################
include ../BasicSynth.cfg

OBJS=\
	GMSynth.o

GMSYNTHLIB=$(BSHOME)/Lib/GMSynth.a

$(GMSYNTHLIB): $(OBJS)
	-@rm $@
	$(AR) r $@ $(OBJS)

all: $(GMSYNTHLIB)

clean:
	-@rm $(OBJS)
	-@rm $(GMSYNTHLIB)

GMSynth.o: \
	GMSynth.cpp \
	GMSynthDLL.h \
	GMInstrManager.h \
	stdafx.h
