include $(TOPDIR)/rules.mk

PKG_NAME:=u80211d
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/blogic/u80211d.git
PKG_SOURCE_DATE:=20200319
PKG_SOURCE_VERSION:=3963f84122b3d9659a79ddc14487d507a332f900
CMAKE_INSTALL:=1

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=John Crispin <john@phrozen.org>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/u80211d
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libubus +libblobmsg-json +ubusd +libjson-c +libnl-tiny +libuci
  TITLE:=Wifi scanning and introspection daemon
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/libnl-tiny

define Package/u80211d/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) ./files/* $(1)/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/u80211d $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/u80211 $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,u80211d))
