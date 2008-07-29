include ../../BasicSynth.cfg

$(BSBIN)/example07b: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example07b: $(BSBIN)/example07b

clean:
	-rm main.o
	-rm $(BSBIN)/example07b
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/Mixer.h $(BSINC)/DelayLine.h \
	$(BSINC)/AllPass.h $(BSINC)/Flanger.h
