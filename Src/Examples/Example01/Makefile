include ../../BasicSynth.cfg

$(BSBIN)/example01: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB) 

example01: $(BSBIN)/example01

clean:
	-rm main.o
	-rm $(BSBIN)/example01

main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h
