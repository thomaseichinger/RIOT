# define the module that is build
MODULE =cpu

# add a list of subdirectories, that should also be build
DIRS += periph

.PHONY: all $(CORTEX_COMMON)


all: $(CORTEX_COMMON) $(BINDIR)$(MODULE).a
	$(AD)for i in $(DIRS) ; do "$(MAKE)" -C $$i ; done ;

$(CORTEX_COMMON):
	$(AD)$(MAKE) -C $(CORTEX_COMMON)

include $(RIOTBASE)/Makefile.base

clean:
	$(AD)for i in $(DIRS) ; do "$(MAKE)" -C $$i clean ; done ;
	$(AD)$(MAKE) -C $(CORTEX_COMMON) clean
