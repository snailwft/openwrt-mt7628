########################################################################
#
# Generic Makefile for ProSLIC_API Demo
# Copyright 2014-2015, Silicon Labs
#
# $Id: Makefile 5000 2015-05-08 21:04:07Z elgeorge $
#
# Usage:  make VMB<1|2>=1 <CHIPSET_TYPE> [PROSLIC_CFG_SRC=<fn>] [DEBUG=1] [MLT=1]
#
# Example make VMB2=1 SI3217X_B_FB=1 
#
########################################################################
# -------------------
# Variables and Paths
# -------------------
.PHONY: all chk_params

# These could alternately be set as env variables
HOME_DIR = ./demo/api_demo/build
PROSLIC_API_DIR ?= $(HOME_DIR)/../../..
MLT_API_DIR ?= $(HOME_DIR)/../../../../proslic_mlt_api
DEMO_SRC_DIR ?= $(HOME_DIR)/../src
DEMO_INC_DIR ?= $(HOME_DIR)/../inc
DEMO_CUSTOM_DIR ?= $(HOME_DIR)/../custom

GSM_SRC_DIR ?= $(HOME_DIR)/../gsrc
GSM_INC_DIR ?= $(HOME_DIR)/../ginc
SOUND_DIR ?= $(HOME_DIR)/../sound

OBJ_DIR = obj
OBJ = o

# --------------
# Compiler Flags
# --------------
CFLAGS_COMMON = -D_CONSOLE -DUSB -w 
CFLAGS_NODBG = -O3
CFLAGS_DBG = -ggdb

# --------------------------------
# Device/Converter Specific Source
# --------------------------------
# -----------------------
# Si3217x Chipset Options
# -----------------------

# Si3217x RevB Flyback
ifdef SI3217X_B_FB
PROSLIC_CHIPSET=3217x
PROSLIC_CONVERTER=FB_GD
OUT_FILE_PREFIX=si3217x_b_fb
endif

# Si3217x RevB BJT Buck-Boost
ifdef SI3217X_B_BB
PROSLIC_CHIPSET=3217x
PROSLIC_CONVERTER=BBB
OUT_FILE_PREFIX=si3217x_b_bb
endif

# Si3217x RevC Flyback
ifdef SI3217X_C_FB
PROSLIC_CHIPSET=3217x
PROSLIC_CONVERTER=FB_NOGD
OUT_FILE_PREFIX=si3217x_c_fb
endif

# Si3217x RevC BJT Buck-Boost
ifdef SI3217X_C_BB
PROSLIC_CHIPSET=3217x
PROSLIC_CONVERTER=BBC
OUT_FILE_PREFIX=si3217x_c_bb
endif

# Si3217x RevC LCQC 3W (was LCQCUK)
ifdef SI3217X_C_LCQC3W
PROSLIC_CHIPSET=3217x
PROSLIC_CONVERTER=LCQC3W
#OUT_FILE_PREFIX=si3217x_c_lcqc3w
OUT_FILE_PREFIX=telpo_lte_right

endif

# Si3217x RevC LCQC 6W
ifdef SI3217X_C_LCQC6W
PROSLIC_CHIPSET=3217x
PROSLIC_CONVERTER=LCQC6W
OUT_FILE_PREFIX=si3217x_c_lcqc6w
endif

# Si3217x Rev B PMOS Buck-Boost
ifdef SI3217X_B_PBB
PROSLIC_CHIPSET=3217x
PROSLIC_CONVERTER=PBB_GD
OUT_FILE_PREFIX=si3217x_b_pbb
endif

# Si3217x Rev C PMOS Buck-Boost
ifdef SI3217X_C_PBB
PROSLIC_CHIPSET=3217x
PROSLIC_CONVERTER=PBB_NOGD
OUT_FILE_PREFIX=si3217x_c_pbb
endif

# -----------------------
# Si3218x Chipset Options
# -----------------------

# Si3218x Rev A LCQC 5W
ifdef SI3218X_A_LCQC5W
PROSLIC_CHIPSET=3218x
PROSLIC_CONVERTER=LCQC5W
OUT_FILE_PREFIX=si3218x_a_lcqc5w
endif

# Si3218x Rev A LCCB
ifdef SI3218X_A_LCCB
PROSLIC_CHIPSET=3218x
PROSLIC_CONVERTER=LCCB
OUT_FILE_PREFIX=si3218x_a_lccb
endif

# -----------------------
# Si3226x Chipset Options
# -----------------------

# Si3226x RevC Flyback
ifdef SI3226X_C_FB
PROSLIC_CHIPSET=3226x
PROSLIC_CONVERTER=FB
OUT_FILE_PREFIX=si3226x_c_fb
endif

# Si3226x RevC QCUK/LCQC 7.6W
ifdef SI3226X_C_LCQC7P6W
PROSLIC_CHIPSET=3226x
PROSLIC_CONVERTER=LCQC7P6W
OUT_FILE_PREFIX=si3226x_c_lcqc7p6w
endif

# Si3226x RevC BJT Buck Boost
ifdef SI3226X_C_BB
PROSLIC_CHIPSET=3226x
PROSLIC_CONVERTER=BB
OUT_FILE_PREFIX=si3226x_c_bb
endif

# Si3226x RevC LCQCUK/LCQC 3W
ifdef SI3226X_C_LCQC3W
PROSLIC_CHIPSET=3226x
PROSLIC_CONVERTER=LCQC3W
OUT_FILE_PREFIX=si3226x_c_lcqc3w
endif

# Si3226x RevC LCQCUK/LCQC 6W
ifdef SI3226X_C_LCQC6W
PROSLIC_CHIPSET=3226x
PROSLIC_CONVERTER=LCQC6W
OUT_FILE_PREFIX=si3226x_c_lcqc6w
endif

# Si3226x RevC CUK
ifdef SI3226X_C_CK
PROSLIC_CHIPSET=3226x
PROSLIC_CONVERTER=CK
OUT_FILE_PREFIX=si3226x_c_ck
endif

# Si3226x RevC QSS
ifdef SI3226X_C_QS
PROSLIC_CHIPSET=3226x
PROSLIC_CONVERTER=QSS
OUT_FILE_PREFIX=si3226x_c_qs
endif

# Si3226x RevC PMOS Buck-Boost
ifdef SI3226X_C_PBB
PROSLIC_CHIPSET=3226x
PROSLIC_CONVERTER=PBB
OUT_FILE_PREFIX=si3226x_c_pbb
endif

# -----------------------
# Si3228x Chipset Options
# -----------------------

# Si3228x Rev A LCQC 5W
ifdef SI3228X_A_LCQC5W
PROSLIC_CHIPSET=3228x
PROSLIC_CONVERTER=LCQC5W
OUT_FILE_PREFIX=si3228x_a_lcqc5w
endif

# Si3228x Rev A LCCB
ifdef SI3228X_A_LCCB
PROSLIC_CHIPSET=3228x
PROSLIC_CONVERTER=LCCB
OUT_FILE_PREFIX=si3228x_a_lccb
endif

# -----------------------
# Si3226 Chipset Options - NOT RECOMMENDED FOR NEW DESIGNS
# -----------------------

ifdef SI3226_QC
PROSLIC_CHIPSET=3226
PROSLIC_CONVERTER=QC
OUT_FILE_PREFIX=si3226_qc
endif

ifdef SI3226_FB
PROSLIC_CHIPSET=3226
PROSLIC_CONVERTER=FB
OUT_FILE_PREFIX=si3226_fb
endif

# -----------------------
# Si324X Chipset Options - NOT RECOMMENDED FOR NEW DESIGNS
# -----------------------

ifdef SI324X
PROSLIC_CHIPSET=324x
OUT_FILE_PREFIX=si324x
endif

# -----------------------
# Si321X Chipset Options - NOT RECOMMENDED FOR NEW DESIGNS
# -----------------------

ifdef SI321X
PROSLIC_CHIPSET=321x
OUT_FILE_PREFIX=si321x

ifdef VMB2
$(error  VMB2 does not support Si321x at this time)
endif

#
# We want to force a make build issue since the demo code
# doesn't link with the correct VMB1 code, but can at least
# verify the compile is OK...
#
PROSLIC_DAA_CFG = bad_option.c
NO_TSTIN=1
endif

PROSLIC_CFLAGS += -I$(DEMO_INC_DIR) -I$(DEMO_CUSTOM_DIR) -I$(GSM_INC_DIR) 

# -------------
# Demo options
# -------------
ifdef NO_LBCAL
PROSLIC_CFLAGS += -DSKIP_LBCAL
endif

ifndef NO_TSTIN
PROSLIC_TSTIN_SUPPORT=1
PROSLIC_CFLAGS += -DTSTIN_SUPPORT=1
endif

ifeq ($(PROSLIC_CHIPSET),3217x)
ifndef NO_VDAA
PROSLIC_DAA_SUPPORT=1
PROSLIC_CFLAGS += -DVDAA_SUPPORT=1
endif
endif

# -------------
# Common Source
# -------------
include $(HOME_DIR)/../../proslic_api_core.mk

# -----------
# Demo Source
# -----------
DEMO_CODE = main.c api_demo.c debug.c monitor.c \
	linefeed.c converter.c ringing.c audio.c \
	tonegen.c pm_mwi.c interrupt.c util.c proslic_tstin_limits.c\
	gsm.c slic_hand.c user_handler.c \
	tinyplay.c tinycap.c  pcm.c number_match.c dbug_log.c interface_active_handler.c\
	crypt_fun.c des.c get_mac_address.c user_fun.c dtmf.c


# --------------
# Platform specific code
# --------------
include $(HOME_DIR)/../../platform/platform_common.mk

# ------------------------------------------
# MLT Source (depends on MLT package 2.4.2 or newer)
# ------------------------------------------
ifdef MLT
include $(MLT_API_DIR)/example_code/mlt_api_core.mk
PROSLIC_SRC_CORE += $(MLT_SRC_CORE)  mlt_demo.c example_mlt_tests.c rencal.c
MLT_SRC_DIR = $(MLT_API_DIR)/src $(MLT_API_DIR)/example_code
PROSLIC_CFLAGS += $(MLT_CFLAGS) -DMLT_ENABLED=1 -I$(MLT_API_DIR)/example_code
endif

# -------------------------
# Pull it all together
# -------------------------

SRCS := $(PROSLIC_SRC_CORE) $(PROSLIC_PFORM) $(PROSLIC_CHIPSET_SRC) $(PROSLIC_CFG_SRC) $(DEMO_CODE) $(PROSLIC_DAA_CFG) 

o_files_tmp := $(patsubst %.c,%.$(OBJ),$(notdir $(SRCS)))
OBJS := $(addprefix $(OBJ_DIR)/,$(o_files_tmp))
d_files := $(patsubst %.$(OBJ),%.d, $(OBJS))

CFLAGS = $(PROSLIC_CFLAGS) $(CFLAGS_COMMON) $(SPI_FLAGS)
LFLAGS = -lc $(SPI_IF_LIB) -lpthread -lm -lc

ifdef DEBUG
CFLAGS += $(CFLAGS_DBG)
DBG_EXT = _DBG
endif

ifdef STRICT
CFLAGS += -Werror -Wall -Wextra -Wpedantic -Wunused-variable -Wunused-function -Wimplicit-function-declaration

endif

# --------------------
# 
# ---------------
vpath %.d $(OBJ_DIR)
vpath %.$(OBJ) $(OBJ_DIR)
vpath %.c $(PROSLIC_API_DIR)/src $(DEMO_SRC_DIR) $(PROSLIC_API_DIR)/patch_files $(PROSLIC_PFORM_DIRS) $(PFORM_TIMER_DIR) $(DEMO_CUSTOM_DIR) $(MLT_SRC_DIR) $(DEMO_CUSTOM_DIR) $(MLT_SRC_DIR) $(GSM_SRC_DIR) $(SOUND_DIR) 

all: chk_params $(OUT_FILE_PREFIX)$(DBG_EXT)$(OS_EXT)

chk_params:
ifndef PROSLIC_CFG_SRC 
	$(error Need to select a chipset/converter combination. Examples: SI3217X_C_LCQC3W, SI3226X_C_FB);
endif
ifndef SPI_FLAGS
	$(error Need to either set VMB1=1 or VBM2=1)
endif


-include $(d_files)


$(OBJ_DIR)/%.$(OBJ) : %.c
	$(CC) -c $(CFLAGS) -o $(OBJ_DIR)/$(notdir $@) -MD $<


$(OUT_FILE_PREFIX)$(DBG_EXT)$(OS_EXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

clean:
	-$(RM) $(OBJ_DIR)/*.$(OBJ) si32*$(OS_EXT) $(OBJ_DIR)/*.d
