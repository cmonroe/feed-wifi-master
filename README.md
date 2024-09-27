# SmartOS WiFi feed

WiFi packages from OpenWrt master branch with some modifications for SmartOS.

## Description

The goal is to maintain synchronization with OpenWrt master while providing
a sandbox for support of new WiFi chips and features.

Currently supported WiFi chips include:
* MT7622
* MT7615
* MT7915
* MT7981
* MT7986
* MT7992
* MT7996

## Usage

This feed can be applied and used on top of OpenWrt master. Many Adtran/SmartRG
SDG targets are supported via upstream OpenWrt and this feed can be used with
any of them.

### Clone OpenWrt
```
git clone https://github.com/openwrt/openwrt.git
cd openwrt
```

### Setup Feeds
```
echo "src-git --force 0wifi https://github.com/cmonroe/feed-wifi-master.git" > feeds.conf
cat feeds.conf.default >> feeds.conf
./scripts/feeds update -a
./scripts/feeds install -a
```

### Config
At minimum, the following menuconfig changes are necessary:
* Target System -> MediaTek ARM
* Subtarget -> Filogic 8x0 (MT798x)
* Target Profile -> Multiple devices
* [enable] Target Devices -> <all Adtran SDG devices>
* [enable] Kernel Modules -> Wireless Drivers -> Enable testmode command support
* [disable] Firmware -> mt7986-wo-firmware
* Network -> WirelessAPD
  * [enable] hostapd-openssl
  * [enable] hostapd-utils
  * [enable] wpa-supplicant-openssl
  * [disable] wpad-basic-mbedtls
  * [enable] wpa-cli

For simplicity, there are two diffconfig variants in the build directory.

1. diffconfig.basic - just the changes described above.
2. diffconfig.full - adds more tools such as LuCi, tcpdump, iperf3, etc.

```
cp feeds/0wifi/build/diffconfig.full .config
make defconfig
```

### Build
```
make -j$(nproc) || make V=s
```

