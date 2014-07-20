####
#### Sample Makefile for building apps with the RIOT OS
####
#### The Sample Filesystem Layout is:
#### /this makefile
#### ../../RIOT 
#### ../../boards   for board definitions (if you have one or more)
#### 

# name of your application
APPLICATION = openwsn

# for easy switching of boards
BOARD ?= iot-lab_M3

# this has to be the absolute path of the RIOT-base dir
RIOTBASE ?= $(CURDIR)/../..

# uncomment this to enable scheduler statistics for ps
#CFLAGS += -DSCHEDSTATISTICS

QUIET ?= 1

## Modules to include. 

USEMODULE += ps
USEMODULE += vtimer
USEMODULE += openwsn_stack

INCLUDES += -I$(RIOTBASE)/sys/net/openwsn_stack/ -I$(RIOTBASE)/sys/net/openwsn_stack/kernel -I$(RIOTBASE)/sys/net/openwsn_stack/bsp -I$(RIOTBASE)/sys/net/openwsn_stack/bsp/riot

include $(RIOTBASE)/Makefile.include
