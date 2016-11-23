include $(TOPDIR)/rules.mk
 
PKG_NAME:=spi_test
PKG_RELEASE:=1
 
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
 
include $(INCLUDE_DIR)/package.mk
 
define Package/spi_test
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=spi_test -- prints a snarky message
endef
 
define Package/spi_test/description
	It's my first package demo.
endef
 
define Build/Prepare   #已修正
	echo "Here is Package/Prepare"
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
 
define Package/spi_test/install
	echo "Here is Package/install"
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spi_test $(1)/bin/
endef
 
$(eval $(call BuildPackage,spi_test))

