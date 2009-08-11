###########################################################
# Makefile for Common library
###########################################################
include ../BasicSynth.cfg
TINYXML=tinyxml

OBJS=\
	DLSFile.o \
	Global.o \
	MIDIControl.o \
	MIDISequencer.o \
	Player.o \
	Sequencer.o \
	SequenceFile.o \
	SFFile.o \
	SMFFile.o \
	SoundBank.o \
	WaveFile.o \
	SynthFileU.o \
	SynthString.o \
	XmlWrapU.o \
	XmlWrapN.o \
	XmlWrapT.o \
	tinyxml.o \
	tinyxmlerror.o \
	tinyxmlparser.o \
	tinystr.o

$(CMNLIB): $(OBJS)
	-@rm $@
	$(AR) r $@ $(OBJS)

all: $(CMNLIB)

clean:
	-@rm $(OBJS)
	-@rm $(CMNLIB)

DLSFile.o: \
	DLSFile.cpp \
	DLSDefs.h \
	DLSFile.h \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/SoundBank.h
	
Player.o: \
	Player.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Player.h

MIDISequencer.o: \
	MIDISequencer.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/MIDISequencer.h

MIDIControl.o: \
	MIDIControl.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/XmlWrap.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/GenWaveWT.h \
	$(BSINC)/MIDIDefs.h \
	$(BSINC)/MIDIControl.h

Sequencer.o: \
	Sequencer.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveTable.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Sequencer.h

SequenceFile.o: \
	SequenceFile.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/SequenceFile.h

SFFile.o: \
	SFFile.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/SFDefs.h \
	$(BSINC)/SoundBank.h \
	$(BSINC)/SFFile.h

SoundBank.o: \
	SoundBank.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/SoundBank.h
	
SMFFile.o: \
	SMFFile.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/MIDIDefs.h \
	$(BSINC)/SMFFile.h

WaveFile.o:\
	WaveFile.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/WaveFile.h

SynthFileU.o: SynthFileU.cpp $(BSINC)/SynthFile.h

XmlWrapU.o: XmlWrapU.cpp $(BSINC)/XmlWrap.h

XmlWrapN.o: XmlWrapN.cpp $(BSINC)/XmlWrap.h

SynthString.o: SynthString.cpp $(BSINC)/SynthString.h

Global.o: Global.cpp $(BSINC)/SynthDefs.h $(BSINC)/WaveTable.h

XmlWrapT.o: XmlWrapT.cpp $(BSINC)/XmlWrap.h

tinyxml.o: $(TINYXML)/tinyxml.cpp
	$(CPP) -c $(CPPFLAGS) $< 
 
tinyxmlerror.o: $(TINYXML)/tinyxmlerror.cpp
	$(CPP) -c $(CPPFLAGS) $< 

tinyxmlparser.o: $(TINYXML)/tinyxmlparser.cpp
	$(CPP) -c $(CPPFLAGS) $< 

tinystr.o: $(TINYXML)/tinystr.cpp
	$(CPP) -c $(CPPFLAGS) $< 
