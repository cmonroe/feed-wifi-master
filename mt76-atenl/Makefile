include $(TOPDIR)/rules.mk

PKG_NAME:=mt76-atenl
PKG_RELEASE=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Shayne Chen <shayne.chen@mediatek.com>
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_SOURCE_DIR:=$(PKG_BUILD_DIR)
CMAKE_BINARY_DIR:=$(PKG_BUILD_DIR)

define Package/mt76-atenl
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=testmode daemon for nl80211
  SUBMENU:=Applications
  DEPENDS:=+libnl-tiny
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

define Package/mt76-atenl/install
	mkdir -p $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/atenl $(1)/usr/sbin
	$(INSTALL_BIN) ./files/ated.sh $(1)/usr/sbin/ated
	$(INSTALL_BIN) ./files/iwpriv.sh $(1)/usr/sbin/iwpriv
	$(INSTALL_BIN) ./files/iwpriv.sh $(1)/usr/sbin/mwctl
endef

$(eval $(call BuildPackage,mt76-atenl))
