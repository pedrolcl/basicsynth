include ../../BasicSynth.cfg

$(BSBIN)/example10: main.cpp
	$(CPP) -lxml2 -lm $(CPPFLAGS) -I/usr/include/libxml2 -o $@ main.cpp $(INSTLIB) $(CMNLIB)
	-cp test1.wav test2.wav data.xml $(BSBIN)

example10: $(BSBIN)/example10

clean:
	-rm main.o
	-rm $(BSBIN)/example10
	
main.cpp: \
$(BSINC)/SynthDefs.h \
$(BSINC)/WaveTable.h \
$(BSINC)/GenWave.h \
$(BSINC)/GenWaveWT.h \
$(BSINC)/GenNoise.h \
$(BSINC)/EnvGen.h \
$(BSINC)/EnvGenSeg.h \
$(BSINC)/BiQuad.h \
$(BSINC)/AllPass.h \
$(BSINC)/Filter.h \
$(BSINC)/DelayLine.h \
$(BSINC)/Flanger.h \
$(BSINC)/Reverb.h \
$(BSINC)/Mixer.h \
$(BSINC)/WaveFile.h \
$(BSINC)/Sequencer.h \
$(BSINC)/XmlWrap.h \
$(BSINSTR)/Tone.h \
$(BSINSTR)/LFO.h \
$(BSINSTR)/SubSynth.h \
$(BSINSTR)/AddSynth.h \
$(BSINSTR)/FMSynth.h \
$(BSINSTR)/MatrixSynth.h \
$(BSINSTR)/WFSynth.h
