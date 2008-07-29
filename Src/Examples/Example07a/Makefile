include ../../BasicSynth.cfg

$(BSBIN)/example07a: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example07a: $(BSBIN)/example07a

clean:
	-rm main.o
	-rm $(BSBIN)/example07a
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/Mixer.h $(BSINC)/DelayLine.h \
	$(BSINC)/Reverb.h
