include $(TOPDIR)/rules.mk
 
PKG_NAME:=watchdog_monitor
PKG_RELEASE:=1
 
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
 
include $(INCLUDE_DIR)/package.mk
 
define Package/watchdog_monitor
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=watchdog_monitor -- prints a snarky message
endef
 
define Package/watchdog_monitor/description
	It's my first package demo.
endef
 
define Build/Prepare   #已修正
	echo "Here is Package/Prepare"
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
 
define Package/watchdog_monitor/install
	echo "Here is Package/install"
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/watchdog_monitor $(1)/bin/
endef
 
$(eval $(call BuildPackage,watchdog_monitor))

