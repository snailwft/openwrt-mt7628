########################################################################
#
# Generic Makefile include for ProSLIC API - defines dependencies.
# Copyright 2014-2015, Silicon Labs
#
# $Id: proslic_api_core.mk 4998 2015-05-06 17:21:13Z elgeorge $
#
# Customers are encouraged to include this file in their makefiles since
# it defines the dependencies for the various chipsets for several of the 
# commonly used converter topologies.  The PROSLIC_CFG_SRC variable
# can be overwritten to use whichever constants file one wants.
#
########################################################################

########################################################################
#
# 3217x options
# 

ifeq ($(PROSLIC_CHIPSET),3217x)
PROSLIC_CHIPSET_SRC = si3217x_intf.c si3217x_revb_intf.c si3217x_revc_intf.c 

ifeq ($(PROSLIC_DAA_SUPPORT),1)
PROSLIC_CHIPSET_SRC += vdaa.c 
PROSLIC_DAA_CFG ?= vdaa_constants.c
endif

PROSLIC_CFLAGS += -DSI3217X

#
# Common Patch file definitions...
#
PROSLIC_17X_PATCH_FB ?=	si3217x_patch_B_FB_2012DEC10.c \
			si3217x_patch_C_FB_2014JUN18.c 

PROSLIC_17X_PATCH_BB ?= si3217x_patch_B_BB_2012DEC10.c \
			si3217x_patch_C_FB_2014JUN18.c

ifeq ($(PROSLIC_CONVERTER),FB_GD)
PROSLIC_CFG_SRC ?= si3217x_FLBK_GDRV_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_17X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),BBB)
PROSLIC_CFG_SRC ?= si3217x_BKBT_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_17X_PATCH_BB)
endif

ifeq ($(PROSLIC_CONVERTER),BBC)
PROSLIC_CFG_SRC ?= si3217x_BKBT_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_17X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),FB_NOGD)
PROSLIC_CFG_SRC ?= si3217x_FLBK_NO_GDRV_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_17X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),LCQC3W)
PROSLIC_CFG_SRC ?= si3217x_LCQC3W_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_17X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),LCQC6W)
PROSLIC_CFG_SRC ?= si3217x_LCQC6W_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_17X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),PBB_GD)
PROSLIC_CFG_SRC ?= si3217x_PBB_VDC_9P0_24P0_GDRV_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_17X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),PBB_NOGD)
PROSLIC_CFG_SRC ?= si3217x_PBB_VDC_9P0_24P0_NO_GDRV_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_17X_PATCH_FB)
endif


endif # 3217x


########################################################################
#
# 3218x options
# 

ifeq ($(PROSLIC_CHIPSET),3218x)
PROSLIC_CHIPSET_SRC = si3218x_intf.c

PROSLIC_CFLAGS += -DSI3218X

PROSLIC_18X_PATCH =	si3218x_patch_A_2015FEB17.c

ifeq ($(PROSLIC_CONVERTER),LCQC5W)
PROSLIC_CFG_SRC ?= si3218x_LCQC5W_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_18X_PATCH)
endif

ifeq ($(PROSLIC_CONVERTER),LCCB)
PROSLIC_CFG_SRC ?= si3218x_LCCB_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_18X_PATCH)
endif

endif # 3218x


########################################################################
#
# 3226x options
# 

ifeq ($(PROSLIC_CHIPSET),3226x)
PROSLIC_CHIPSET_SRC = si3226x_intf.c
PROSLIC_CFLAGS += -DSI3226X

PROSLIC_26X_PATCH_FB = si3226x_patch_C_FB_2014JUN18.c
PROSLIC_26X_PATCH_TSS = si3226x_patch_C_TSS_2014JUN18.c

ifeq ($(PROSLIC_CONVERTER),FB)
PROSLIC_CFG_SRC ?= si3226x_FLBK_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),BB)
PROSLIC_CFG_SRC ?= si3226x_BB_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),LCQC3W)
PROSLIC_CFG_SRC ?= si3226x_LCQC3W_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),LCQC6W)
PROSLIC_CFG_SRC ?= si3226x_LCQC6W_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),LCQC7P6W)
PROSLIC_CFG_SRC ?= si3226x_LCQC7P6W_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26X_PATCH_FB)
endif


ifeq ($(PROSLIC_CONVERTER),CK)
PROSLIC_CFG_SRC ?= si3226x_CUK_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26X_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),QSS)
PROSLIC_CFG_SRC ?= si3226x_QSS_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26X_PATCH_TSS)
endif

ifeq ($(PROSLIC_CONVERTER),PBB)
PROSLIC_CFG_SRC ?= si3226x_PBB_7P0_20P0_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26X_PATCH_FB)
endif

endif # si3226x


########################################################################
#
# 3228x options
# 

ifeq ($(PROSLIC_CHIPSET),3228x)
PROSLIC_CHIPSET_SRC = si3228x_intf.c

PROSLIC_CFLAGS += -DSI3228X

PROSLIC_28X_PATCH =	si3228x_patch_A_DEFAULT_2015MAR17.c

ifeq ($(PROSLIC_CONVERTER),LCQC5W)
PROSLIC_CFG_SRC ?= si3228x_LCQC5W_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_28X_PATCH)
endif

ifeq ($(PROSLIC_CONVERTER),LCCB)
PROSLIC_CFG_SRC ?= si3228x_LCCB_constants.c 
PROSLIC_CHIPSET_SRC += $(PROSLIC_28X_PATCH)
endif

endif # 3228x

########################################################################
#
# 3226 options - NOT RECOMMENDED FOR NEW DESIGNS
# 

ifeq ($(PROSLIC_CHIPSET),3226)
PROSLIC_CHIPSET_SRC = si3226_intf.c
PROSLIC_CFLAGS += -DSI322X

PROSLIC_26_PATCH_FB = si3226_patch_C_FLBK_2010AUG10.c\
	si3226_patch_D_FLBK_2010AUG10.c \
	si3226_patch_E_FLBK_2010AUG10.c

PROSLIC_26_PATCH_QCUK = si3226_patch_C_QCUK_2010AUG10.c\
	si3226_patch_D_QCUK_2010AUG10.c \
	si3226_patch_E_FLBK_2010AUG10.c

ifeq ($(PROSLIC_CONVERTER),FB)
PROSLIC_CFG_SRC ?= si3226_FLBK_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26_PATCH_FB)
endif

ifeq ($(PROSLIC_CONVERTER),QC)
PROSLIC_CFG_SRC ?= si3226_QCUK_constants.c
PROSLIC_CHIPSET_SRC += $(PROSLIC_26_PATCH_QCUK)
endif

endif # si3226

########################################################################
#
# 324x options  - NOT RECOMMENDED FOR NEW DESIGNS
# 

ifeq ($(PROSLIC_CHIPSET),324x)
PROSLIC_CHIPSET_SRC = si324x_intf.c
PROSLIC_CFLAGS += -DSI324X

PROSLIC_CFG_SRC ?= si324x_constants.c
PROSLIC_CHIPSET_SRC += si324x_patch_D_2010AUG15.c
endif #si324x

########################################################################
#
# 321x options  - NOT RECOMMENDED FOR NEW DESIGNS
# 

ifeq ($(PROSLIC_CHIPSET),321x)
PROSLIC_CHIPSET_SRC = si321x_intf.c
PROSLIC_CFLAGS += -DSI321X
PROSLIC_CFG_SRC ?= si321x_constants.c
endif # Si321x - NOTE: no patch file!

########################################################################
#
# Common
#

PROSLIC_SRC_CORE= si_voice.c si_voice_version.c proslic.c 

ifeq ($(PROSLIC_TSTIN_SUPPORT),1)
PROSLIC_SRC_CORE += proslic_tstin.c
endif

PROSLIC_CFLAGS += -I$(PROSLIC_API_DIR)/inc 

