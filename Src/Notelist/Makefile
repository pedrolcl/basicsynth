include ../BasicSynth.cfg

OBJS=\
	Converter.o \
	Parser.o \
	Lex.o \
	Generate.o

$(NLLIB): $(OBJS)
	-@rm $@
	$(AR) r $@ $(OBJS)

all: $(NLLIB)

clean:
	-@rm $(OBJS)
	-@rm $(NLLIB)

Converter.cpp: Converter.h

Parser.cpp: Parser.h

Lex.cpp: Lex.h

Generate.cpp: Generate.h
