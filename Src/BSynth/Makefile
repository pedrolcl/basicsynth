include ../BasicSynth.cfg

$(BSBIN)/BSynth: main.cpp $(CMNLIB) $(NLLIB) $(INSTLIB)
	$(CPP) $(XMLLIB) -lm -I../Notelist $(CPPFLAGS) -o $@ main.cpp \
		$(NLLIB) $(INSTLIB) $(CMNLIB)
	-cp testinst.xml testprj.xml test.nl test2.nl $(BSBIN)
	-cp tst*synth.xml $(BSBIN)
	-cp tst*synth.nl $(BSBIN)
	-cp testnl.xml testnl.nl $(BSBIN)

BSynth: $(BSBIN)/BSynth

clean:
	-rm $(BSBIN)/BSynth
	-rm $(BSBIN)/testinst.xml $(BSBIN)/testprj.xml
	-rm $(BSBIN)/test.nl $(BSBIN)/test2.nl
	-rm $(BSBIN)/testnl.xml $(BSBIN)/testnl.nl
	-rm $(BSBIN)/tst*synth.xml
	-rm $(BSBIN)/tst*synth.nl
	-rm $(BSBIN)/tst*synth.wav
