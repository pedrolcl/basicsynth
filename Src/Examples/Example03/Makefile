include ../../BasicSynth.cfg

$(BSBIN)/example03: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example03: $(BSBIN)/example03

clean:
	-rm main.o
	-rm $(BSBIN)/example03

main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h $(BSINC)/EnvGen.h
