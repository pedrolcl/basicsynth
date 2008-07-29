###########################################################
# Makefile for Common library
###########################################################
include ../BasicSynth.cfg

OBJS=\
	MIDISequencer.o \
	Sequencer.o \
	SequenceFile.o \
	WaveFile.o \
	SynthFileU.o \
	XmlWrapU.o \
	SynthString.o \
	Global.o

$(CMNLIB): $(OBJS)
	-@rm $@
	$(AR) r $@ $(OBJS)

all: $(CMNLIB)

clean:
	-@rm $(OBJS)
	-@rm $(CMNLIB)

MIDISequencer.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/MIDISequencer.h

Sequencer.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/Sequencer.h

SequenceFile.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/SequenceFile.h

WaveFile.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/WaveFile.h

SynthFileU.cpp: $(BSINC)/SynthFile.h

XmlWrapU.cpp: $(BSINC)/XmlWrap.h

SynthString.cpp: $(BSINC)/SynthString.h

Global.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveTable.h
