include $(TOPDIR)/rules.mk
     
PKG_NAME:=telpo_lte
PKG_RELEASE:=1
 
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
 
include $(INCLUDE_DIR)/package.mk
 
define Package/telpo_lte
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=telpo_lte -- prints a snarky message
	DEPENDS:=+libpthread +libc
endef
 
define Package/telpo_lte/description
	It's my first package demo.
endef
 
define Build/Prepare   #已修正
	echo "Here is Package/Prepare"
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
 
define Package/telpo_lte/install
	echo "Here is Package/install"
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/telpo_lte $(1)/bin/
endef
 
$(eval $(call BuildPackage,telpo_lte))

