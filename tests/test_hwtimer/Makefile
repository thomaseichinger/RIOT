export APPLICATION = test_hwtimer
include ../Makefile.tests_common

DISABLE_MODULE += auto_init

include $(RIOTBASE)/Makefile.include

reset:
	$(RIOTCPU)/$(CPU)/tools/reset.sh
