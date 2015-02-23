# name of your application
APPLICATION = sensor_node

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Uncomment this to enable scheduler statistics for ps:
#CFLAGS += -DSCHEDSTATISTICS

# Uncomment this to enable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
export QUIET ?= 1

# Modules to include:
USEMODULE += defaulttransceiver
USEMODULE += vtimer

export SENSOR_NODE_SRC_ID ?= 0x14
export SENSOR_NODE_PAN_ID ?= 12
export CENTRAL_NODE_ADDR ?= 256

include $(RIOTBASE)/Makefile.include

ifneq (,$(SENSOR_NODE_PAN_ID))
export CFLAGS += -DSENSOR_NODE_PAN_ID=$(SENSOR_NODE_PAN_ID)
endif
ifneq (,$(SENSOR_NODE_SRC_ID))
export CFLAGS += -DSENSOR_NODE_SRC_ID=$(SENSOR_NODE_SRC_ID)
endif
ifneq (,$(CENTRAL_NODE_ADDR))
export CFLAGS += -DCENTRAL_NODE_ADDR=$(CENTRAL_NODE_ADDR)
endif
