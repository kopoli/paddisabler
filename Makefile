#T‰m‰ makefile tukee useamman ohjelman generoimista.
#Vaatii GNU Make:n

# K‰ytt‰minen: 
# -Eri ohjelmat laitetaan PROGS muuttujaan
# -vastaavien ohjelmien l‰hdekoodit laitetaan muuttujiin tyyliin:
#  ohjelma_SRC  -l‰hdekoodi
#  ohjelma_HDRS -otsikkotiedostot
# -l‰hdekoodissa voi olla C ja C++ koodia sekaisin
# -projektin nimi on PROJECT muuttujassa (tulee varmuuskopiopaketin nimeen)
# -versio on VERSION

#projektin nimi
PROJECT=paddisabler

#ohjelmien nimet
PROGS=paddisabler
# palvelin test nettest
VERSION=1.0.0

#l‰hdekoodin tiedot
paddisabler_SRC  = paddisabler.c
paddisabler_HDRS = 

EXTRA_DIST=

#asetukset

#mink‰tyyppiset tiedostot kuuluvat l‰hdekoodiin
suffixes=c cc

#asennuskansio
PREFIX?=./

DOXFILE=hajap.dox
DOXDIR=docs

SRCDIR=src
OUTDIR=build

#k‰ytetyt ohjelmat
RM=rm -f
RMR=$(RM) -r
CP=cp -r
MKDIR=mkdir -p
DOX=doxygen

#kirjastot
NETTLE?=/usr/local/lang/nettle-1.15

CC=gcc
CFLAGS=-Wall -std=c99 -g -O2 -DVERSION=\"$(VERSION)\" -DPREFIX=\"$(PREFIX)\" -I$(NETTLE)/include

CXX=g++
CXXFLAGS=$(CFLAGS)

LDFLAGS=-lm -lX11 -lXi

ifeq ($(shell uname -s),SunOS) 
  LDFLAGS+=-lsocket -lnsl
else
endif

DIST=zip -9 -r -m -q
DTYPE=zip


#generointi
map=$(foreach v,$2,$(call $1,$v))

#tiedostonimien generointia
x_gen=$(sort $(foreach p,$1,$($(p)_$2)))
src_gen=$(call x_gen,$1,SRC)
obj_gen=$(filter-out $(src_gen),$(foreach pr,$(suffixes),\
	$(patsubst %.$(pr),$(OUTDIR)/%.o,$(src_gen))))

distdir=$(PROJECT)-`date +%y%m%d`
distfile=$(distdir).$(DTYPE)
headers=$(call x_gen,$(PROGS),HDRS)
dirs=$(OUTDIR) $(DOXDIR)
generated=$(PROGS) $(sort $(call map,obj_gen,$(PROGS))) $(dirs)


#hiljaisuus
ifeq ($(VERBOSE),)
  Q=@
endif

.PHONY: help
.SUFFIXES: $(foreach s,$(suffixes),.$(s))

#t‰m‰ on sit‰ varten, ett‰ $@ muuttujaa voi k‰ytt‰‰ prereq-listassa
.SECONDEXPANSION:

all: $(PROGS)

$(PROGS): $(OUTDIR) $$(call obj_gen,$$@)
	$(Q)echo "  LINK  $@"
	$(Q)$(CXX) $(CXXFLAGS) -o $@ $(call obj_gen,$@) $(LDFLAGS)

$(OUTDIR)/%.o: %.c
	$(Q)echo "  CC    $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

$(OUTDIR)/%.o: %.cc
	$(Q)echo "  CXX   $@"
	$(Q)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(dirs):
	$(Q)echo "  MKDIR $@"
	$(Q)$(MKDIR) $@

dox: $(DOXDIR) $(DOXFILE)
	$(Q)echo "  DOX   $(DOXFILE) -> $(DOXDIR)"
	$(Q)$(DOX) $(DOXFILE)

clean:
	$(Q)echo "  CLEAN $(generated)"
	$(Q)$(RMR) $(generated)

help: 
	$(Q)echo "Projekti: $(PROJECT) v.$(VERSION)"
	$(Q)echo "Makefile targets:"
	$(Q)echo "  make all      K‰‰nn‰ ohjelma"
	$(Q)echo "  make help     T‰m‰ apuviesti"
	$(Q)echo "  make dox      Generoi dokumentaatio"
	$(Q)echo "  make dist     Generoi varmuuskopio"
	$(Q)echo "  make clean    Poista generoidut"
	$(Q)echo "  make install  Asenna ohjelma PREFIX-kansioon"

dist: clean
	$(Q)echo "  DIST  $(distfile)"
	$(Q)$(MKDIR) $(distdir)
	$(Q)$(CP) Makefile $(sort $(call map,src_gen,$(PROGS))) $(EXTRA_DIST) \
	  $(headers) $(distdir)
	$(Q)$(RM) $(distfile)
	$(Q)$(DIST) $(distfile) $(distdir)
	$(Q)$(RMR) $(distdir)


#ohjelman asennus
install: check-prefix all
	$(Q)echo "  INST  $(PREFIX)"
	$(Q)if test ! -e $(PREFIX); then \
	  mkdir -p $(PREFIX); \
	elif [ ! -d $(PREFIX) ]; then \
	  echo "Virhe: $(PREFIX) ei ole kansio!"; \
	  exit 1; \
	fi;
	$(Q)$(CP) $(PROGS) $(PREFIX)/

check-prefix:
ifdef PREFIX
	@ # ei tee mit‰‰n
else
	$(error muuttuja PREFIX on asetettava [gmake PREFIX=/polku])
endif
