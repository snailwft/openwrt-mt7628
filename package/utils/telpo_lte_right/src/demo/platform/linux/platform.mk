########################################################################
#
# Makefile for Linux specific code.
# Copyright 2014, Silicon Labs
#
# $Id: platform.mk 4672 2015-01-28 00:22:06Z nizajerk $
#
########################################################################
ifdef VMB1
$(error "VMB1 is not supported under Linux - sorry")
endif

ifdef VMB2
SPI_FLAGS = -DVMB2
SPI_IF_LIB = 
PROSLIC_PFORM += proslic_vmb2_posix.c proslic_vmb2.c
endif

PROSLIC_PFORM += proslic_timer.c

#
# Linux desktop VMB2 drivers use the POSIX compatible driver,
# assuming there's a device node...
#
PROSLIC_PFORM_DIRS += $(PROSLIC_PFORM_ROOT)/posix/timer
PROSLIC_PFORM_DIRS += $(PROSLIC_PFORM_ROOT)/posix/spi
PROSLIC_CFLAGS     += -I$(PROSLIC_PFORM_ROOT)/common/inc -I$(PROSLIC_PFORM_ROOT)/posix/spi
PROSLIC_CFLAGS     += -I$(PROSLIC_PFORM_ROOT)/posix/timer

PROSLIC_CFLAGS += -DPOSIX=1
