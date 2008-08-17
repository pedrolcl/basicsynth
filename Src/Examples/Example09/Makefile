include ../../BasicSynth.cfg

$(BSBIN)/example09: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example09: $(BSBIN)/example09

clean:
	-rm main.o
	-rm $(BSBIN)/example09
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/BiQuad.h \
	$(BSINC)/GenNoise.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/DelayLine.h $(BSINC)/Mixer.h \
	$(BSINC)/Sequencer.h $(BSINC)/MIDISequencer.h
