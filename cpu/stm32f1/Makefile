# define the module that is build
MODULE =cpu

# add a list of subdirectories, that should also be build
DIRS += periph

.PHONY: all $(CORTEX-M3_COMMON)


all: $(CORTEX-M3_COMMON) $(BINDIR)$(MODULE).a
	$(AD)for i in $(DIRS) ; do "$(MAKE)" -C $$i ; done ;

$(CORTEX-M3_COMMON):
	$(AD)$(MAKE) -C $(CORTEX-M3_COMMON)

include $(RIOTBASE)/Makefile.base

clean::
	$(AD)for i in $(DIRS) ; do "$(MAKE)" -C $$i clean ; done ;
	$(AD)$(MAKE) -C $(CORTEX-M3_COMMON) clean
