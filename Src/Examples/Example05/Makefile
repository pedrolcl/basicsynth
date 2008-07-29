include ../../BasicSynth.cfg

$(BSBIN)/example05: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example05: $(BSBIN)/example05

clean:
	-rm main.o
	-rm $(BSBIN)/example05
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/GenWave.h \
	$(BSINC)/GenWaveWT.h $(BSINC)/WaveTable.h \
	$(BSINC)/GenNoise.h $(BSINC)/BiQuad.h \
	$(BSINC)/AllPass.h $(BSINC)/Filter.h
