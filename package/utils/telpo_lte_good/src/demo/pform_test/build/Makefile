########################################################################
#
# Generic Makefile for ProSLIC_API Platform tester
# Copyright 2014-2015, Silicon Labs
#
# $Id: Makefile 4609 2014-12-08 17:04:16Z nizajerk $
#
# Usage:  make VMB<1|2>=1  [DAA=1]
#
# DAA=1 - SPI access tests should assume channel is for Si3050 vs.
#         ProSLIC.
#
# Example make VMB2=1 
#
########################################################################
# -------------------
# Variables and Paths
# -------------------
.PHONY: all chk_params

# These could alternately be set as env variables
HOME_DIR = .
PROSLIC_API_DIR ?= $(HOME_DIR)/../../..

DEMO_SRC_DIR ?= $(HOME_DIR)/../src
DEMO_INC_DIR ?= $(HOME_DIR)/../inc
DEMO_CUSTOM_DIR ?= $(HOME_DIR)/../custom

OBJ_DIR = obj
OBJ = o

# --------------
# Compiler Flags
# --------------
CFLAGS_COMMON = -D_CONSOLE -DUSB -w 
CFLAGS_NODBG = -O3
CFLAGS_DBG = -ggdb

PROSLIC_CFLAGS += -I$(DEMO_INC_DIR) -I$(DEMO_CUSTOM_DIR) -I$(PROSLIC_API_DIR)/inc

# -------------
# Demo options
# -------------

# -----------
# Demo Source
# -----------
DEMO_CODE = pform_test.c si_voice.c spi_tests_common.c

ifdef DAA
DEMO_CODE += spi_tests_daa.c
OUT_FILE_PREFIX =pform_tester_daa
TEST_TYPE = -DDAA_TESTS=1
else
OUT_FILE_PREFIX=pform_tester
DEMO_CODE += spi_tests.c timer_tests.c
TEST_TYPE = -DPROSLIC_TESTS=1
endif

# --------------
# Platform specific code
# --------------
include ../../platform/platform_common.mk

# -------------------------
# Pull it all together
# -------------------------

SRCS := $(PROSLIC_SRC_CORE) $(PROSLIC_PFORM) $(DEMO_CODE) 

o_files_tmp := $(patsubst %.c,%.$(OBJ),$(notdir $(SRCS)))
OBJS := $(addprefix $(OBJ_DIR)/,$(o_files_tmp))
d_files := $(patsubst %.$(OBJ),%.d, $(OBJS))

CFLAGS = $(PROSLIC_CFLAGS) $(CFLAGS_COMMON) $(SPI_FLAGS) $(TEST_TYPE)
LFLAGS = -lc $(SPI_IF_LIB)

ifdef DEBUG
CFLAGS += $(CFLAGS_DBG)
DBG_EXT = _DBG
endif

ifdef STRICT
CFLAGS += -Werror -Wall
endif


# --------------------
#  
# ---------------
vpath %.d $(OBJ_DIR)
vpath %.$(OBJ) $(OBJ_DIR)
vpath %.c $(DEMO_SRC_DIR) $(PROSLIC_PFORM_DIRS) $(PROSLIC_API_DIR)/src

all: chk_params $(OUT_FILE_PREFIX)$(DBG_EXT)$(OS_EXT)

chk_params:
ifndef SPI_FLAGS
	$(error Need to either set VMB1=1 or VBM2=1)
endif


-include $(d_files)


$(OBJ_DIR)/%.$(OBJ) : %.c
	$(CC) -c $(CFLAGS) -o $(OBJ_DIR)/$(notdir $@) -MD $<


$(OUT_FILE_PREFIX)$(DBG_EXT)$(OS_EXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

clean:
	-$(RM) $(OBJ_DIR)/*.$(OBJ) $(OUT_FILE_PREFIX)$(OS_EXT) $(OBJ_DIR)/*.d
