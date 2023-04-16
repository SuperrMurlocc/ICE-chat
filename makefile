.PHONY: all clean ice

ICEDIR=./ice
ICEOUT=./ice/out
ICEFLG=--impl-c++11 --output-dir $(ICEOUT) --depend-file $(ICEOUT)/depend
LICE=-lIce++11

OUTDIR=./out

USERDIR=./user
USEROUT=$(OUTDIR)

LOBBYDIR=./lobby
LOBBYOUT=$(OUTDIR)

FCTRDIR=./factory
FCTROUT=$(OUTDIR)

UTILSDIR=./utils
UTILSOUT=$(OUTDIR)

ODIR=./ofiles
OFLG=-std=c++17 -I. -DICE_CPP11_MAPPING -c
OMAKE=c++ $(OFLG) $< -o $@

default: all

all: $(OUTDIR) $(ODIR) user lobby factory

$(OUTDIR):
	mkdir $(OUTDIR)

$(ODIR):
	mkdir $(ODIR)

user: $(ODIR)/utils.o $(ODIR)/user.o $(ODIR)/chat.o $(ODIR)/chatI.o
	c++ -Wall -Wextra $^ -o $(USEROUT)/$@ $(LICE)

lobby: $(ODIR)/lobby.o $(ODIR)/utils.o $(ODIR)/chat.o $(ODIR)/chatI.o
	c++ -Wall -Wextra $^ -o $(LOBBYOUT)/$@ $(LICE)

factory: $(ODIR)/factory.o $(ODIR)/utils.o $(ODIR)/chat.o $(ODIR)/chatI.o
	c++ -Wall -Wextra $^ -o $(FCTROUT)/$@ $(LICE)

$(ODIR)/user.o: $(USERDIR)/user.cpp $(UTILSDIR)/utils.h
	$(OMAKE)

$(ODIR)/lobby.o: $(LOBBYDIR)/lobby.cpp
	$(OMAKE)

$(ODIR)/factory.o: $(FCTRDIR)/factory.cpp
	$(OMAKE)

$(ODIR)/utils.o: $(UTILSDIR)/utils.cpp $(UTILSDIR)/utils.h
	$(OMAKE)

$(ODIR)/chat.o: $(ICEOUT)/chat.cpp $(ICEOUT)/chat.h
	$(OMAKE)

$(ODIR)/chatI.o: $(ICEOUT)/chatI.cpp $(ICEOUT)/chatI.h
	$(OMAKE)

clean:
	rm -rf $(ODIR)
	rm -rf $(OUTDIR)

ice:
	mkdir -p $(ICEOUT)
	slice2cpp $(ICEFLG) $(ICEDIR)/chat.ice
	sed -i '' 's/<chat.h>/"chat.h"/g' $(ICEOUT)/chatI.h
	sed -i '' 's/<chat.h>/"chat.h"/g' $(ICEOUT)/chat.cpp
	sed -i '' 's/<chatI.h>/"chatI.h"/g' $(ICEOUT)/chatI.cpp
