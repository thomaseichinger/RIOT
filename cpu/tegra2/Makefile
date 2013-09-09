MODULE =cpu

INCLUDES = -Iinclude -Idrivers/include -I../armv7_common/include -I../tegra_common/include
DIRS =
ifneq (,$(findstring module,$(USEMODULE)))
	DIRS += module
endif

all: $(BINDIR)$(MODULE).a
	@for i in $(DIRS) ; do $(MAKE) -C $$i ; done ;

include $(RIOTBASE)/Makefile.base

clean::
	@for i in $(DIRS) ; do $(MAKE) -C $$i clean ; done ;

