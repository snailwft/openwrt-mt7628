########################################################################
#
# Makefile for Cygwin specific code.
# Copyright 2014-2015, Silicon Labs
#
# $Id: platform.mk 4681 2015-01-29 19:48:37Z nizajerk $
#
########################################################################

ifdef VMB1
SPI_FLAGS = -DVMB1
SPI_IF_LIB = -L$(PROSLIC_PFORM_ROOT)/$(PROSLIC_OS)/bin -lquad_io
PROSLIC_PFORM += proslic_spiGci_usb.c
endif

ifdef VMB2
SPI_FLAGS = -DVMB2
SPI_IF_LIB = -L$(PROSLIC_PFORM_ROOT)/$(PROSLIC_OS)/bin -lvmb2_dll
PROSLIC_PFORM += spi_pcm_vcp.c
endif

PROSLIC_PFORM += proslic_timer.c

PROSLIC_PFORM_DIRS += $(PROSLIC_PFORM_ROOT)/$(PROSLIC_OS)/spi 
PROSLIC_PFORM_DIRS += $(PROSLIC_PFORM_ROOT)/posix/timer
PROSLIC_CFLAGS     += -I$(PROSLIC_PFORM_ROOT)/posix/timer -I$(PROSLIC_PFORM_ROOT)/$(PROSLIC_OS)/spi
PROSLIC_CFLAGS     += -I$(PROSLIC_PFORM_ROOT)/common/inc

OS_EXT=.exe
PROSLIC_CFLAGS += -DPOSIX=1
