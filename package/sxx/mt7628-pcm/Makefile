#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# derived from https://github.com/hzeller/gmrender-resurrect.git
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt7628-pcm
PKG_VERSION:=2013-12-04
PKG_RELEASE:=$(PKG_SOURCE_VERSION)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt7628-pcm
    TITLE:= PCM on mt7628
    SECTION:=sxx 
    CATEGORY:=Sxx
    DEPENDS:= +kmod-sound-core +kmod-sound-soc-core #+kmod-ralink-dma
    KCONFIG:=
        
    FILES:=$(PKG_BUILD_DIR)/*.$(LINUX_KMOD_SUFFIX)
    AUTOLOAD:=$(call AutoLoad,50,mt7628-pcm mt7628-pcm-codec mt7628-pcm-machine)
endef

define KernelPackage/mt7628-pcm/description
  I2S on mt7628
endef


define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -r src/. $(PKG_BUILD_DIR)
endef

PKG_EXTRA_KCONFIG:= \
	CONFIG_MT7628_PCM=m \
	CONFIG_MT7628_PCM_CODEC=m \
	CONFIG_MT7628_PCM_MACHINE=m
    
    
PKG_EXTRA_CFLAGS:= 

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		$(PKG_EXTRA_KCONFIG) \
        EXTRA_CFLAGS="$(PKG_EXTRA_CFLAGS)" \
		V="$(V)" \
		modules
endef

define KernelPackage/mt7628-pcm/install
endef

$(eval $(call KernelPackage,mt7628-pcm))
