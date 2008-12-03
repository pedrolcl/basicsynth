###########################################################
# Makefile for Common library
###########################################################
include ../BasicSynth.cfg
TINYXML=tinyxml

OBJS=\
	Global.o \
	MIDISequencer.o \
	Sequencer.o \
	SequenceFile.o \
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

MIDISequencer.o: \
	MIDISequencer.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/MIDISequencer.h

Sequencer.o: \
	Sequencer.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/Sequencer.h

SequenceFile.o:\
	SequenceFile.cpp \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/SequenceFile.h

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
