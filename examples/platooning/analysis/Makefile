REQUIRED_VERSION="6"
ifeq (, $(shell which opp_run))
$(error Cannot determine OMNeT++ version. Is opp_run in your PATH?)
endif
OPP_VERSION="$(shell opp_run -v | grep Version | sed -e "s/Version: //" -e "s/,.*//" | cut -d'.' -f 1)"
ifneq ("$(OPP_VERSION)","$(REQUIRED_VERSION)")
$(error This Makefile has been generated for OMNeT++ version $(REQUIRED_VERSION), but version $(OPP_VERSION) has been found. Please re-generate the Makefile using genmakefile.py)
endif

# tool for indexing vec files
SCAVETOOL = opp_scavetool
# results location
RESDIR = ../results
# script for merging
MERGESCRIPT = merge.R

# match all .vec files for the Sinusoidal config
SINUSOIDAL      = $(wildcard $(RESDIR)/Sinusoidal*.vec)
# change suffix from .vec to .Rdata and add the sin prefix
SINUSOIDAL_DATA = $(SINUSOIDAL:$(RESDIR)/%.vec=$(RESDIR)/sin.%.Rdata)
# match all .vec files for the Braking config
BRAKING         = $(wildcard $(RESDIR)/Braking*.vec)
# change suffix from .vec to .Rdata and add the bra prefix
BRAKING_DATA    = $(BRAKING:$(RESDIR)/%.vec=$(RESDIR)/bra.%.Rdata)

# vector index files and Rdata files
VCI = $(VECTOR:%.vec=%.vci)
RDATA = $(VECTOR:%.vec=%.Rdata)
CSV = $(VECTOR:%.vec=%.csv)

# all make targets
all: Sinusoidal.Rdata Braking.Rdata 

# to make Sinusoidal.Rdata we need to merge all files starting with sin.Sinusoidal
# before this, check that all SINUSOIDAL_DATA files have been processed
$(RESDIR)/Sinusoidal.Rdata: $(SINUSOIDAL_DATA)
	$(MERGESCRIPT) $(RESDIR)/ sin.Sinusoidal $(notdir $@) map-config default Rdata
Sinusoidal.Rdata: $(RESDIR)/Sinusoidal.Rdata

# to make Braking.Rdata we need to merge all files starting with bra.Braking
# before this, check that all BRAKING_DATA files have been processed
$(RESDIR)/Braking.Rdata: $(BRAKING_DATA)
	$(MERGESCRIPT) $(RESDIR)/ bra.Braking $(notdir $@) map-config default Rdata
Braking.Rdata: $(RESDIR)/Braking.Rdata

# to make all sin.*.Rdata files we need to run the generic parser
sin.%.csv: %.vec %.vci
	generic-parser.py $< map-config default sin csv
sin.%.Rdata: sin.%.csv
	csv-to-rdata.R $<
	rm $<

# to make all bra.*.Rdata files we need to run the generic parser
bra.%.csv: %.vec %.vci
	generic-parser.py $< map-config default bra csv
bra.%.Rdata: bra.%.csv
	csv-to-rdata.R $<
	rm $<

# if vec files are not indexed, index them
%.vci : %.vec
	$(SCAVETOOL) index $<

# helper to print variable values. e.g.: make print-DELAY_DATA
print-%:
	@echo '$*=$($*)'

# every intermediate file is kept instead of being automatically deleted.
# .vci files are detected as intermediate and thus cancelled when the make
# command terminates. however, such files can still be needed by other
# targets, so keep them instead of re-doing indexing
.SECONDARY :
