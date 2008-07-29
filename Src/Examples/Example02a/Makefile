include ../../BasicSynth.cfg

$(BSBIN)/example02a: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example02a: $(BSBIN)/example02a

clean:
	-rm main.o
	-rm $(BSBIN)/example02a
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h $(BSINC)/EnvGen.h $(BSINC)/EnvGenSeg.h
