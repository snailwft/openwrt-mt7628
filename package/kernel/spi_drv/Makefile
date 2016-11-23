#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=spi_drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/spi_drv
  SUBMENU:=Other modules
  TITLE:=Simple spi driver
  FILES:=$(PKG_BUILD_DIR)/spi_drv.ko
  AUTOLOAD:=$(call AutoLoad,30,spi_drv,1)
  KCONFIG:=
endef

define KernelPackage/spi_drv/description
 This is a replacement for the following in-kernel drivers:
 1) gpio_keys (KEYBOARD_GPIO)
 2) gpio_keys_polled (KEYBOARD_GPIO_POLLED)

 Instead of generating input events (like in-kernel drivers do) it generates
 uevent-s and broadcasts them. This allows disabling input subsystem which is
 an overkill for OpenWrt simple needs.
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,spi_drv))
