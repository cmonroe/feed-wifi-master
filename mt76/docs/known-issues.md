# Known Issues

This file keeps the items that have no ticket of their own: minor faults,
limits and open questions. When an item gets a Polarion ticket, the ticket
becomes its record and the entry leaves this file.

The entries are in order of severity, most severe first. Put a new entry at
its rank.

## mt7615 Calibrates Again at Each Return to the Operating Channel in a Scan

**Severity:** low: a 5 GHz scan takes 36 s on an 834-5 whose backhaul
station is associated. The effect on the backhaul traffic is not measured.
Seen on an 834-5 satellite with 26.11.0.101, 2026-11-06, during SDG-13169.

### What happens

- mac80211 scans the radio itself, because the mt7615 firmware of the 834-5
  has no scan offload. While a station interface is associated, mac80211
  returns to the operating channel after at most 125 ms away
  (`ieee80211_scan_state_decision()`). On DFS channels this is after each
  channel.
- `mt7615_mcu_set_chan_info()` sends `CH_SWITCH_SCAN_BYPASS_DPD` for a scan
  channel and `CH_SWITCH_NORMAL` for the return. The firmware calibrates on
  `CH_SWITCH_NORMAL`.
- A kprobe trace of one scan, with the operating channel 48 at 80 MHz:
  23 returns of 1073 ms each, 24.7 s of the 35.9 s scan. A change to a scan
  channel took about 4 ms.
- The radio probably cannot send during the calibration, so each scan can
  stop the backhaul 23 times for about 1 s. This is not measured.
- The mt7915 of an SDG-8610 takes at most 16 ms for each change in the same
  scan, so it does not have this fault.

### Options

Send a switch reason without calibration for the return from a scan
channel, if the firmware keeps the calibration of the operating channel
from before the scan. The firmware documents neither point; measure the
TX quality on the operating channel after such a scan first. Or scan an
associated 834-5 radio less often, or with fewer channels in each request.

## MT7986 Firmware Keeps Trigger-Based Sounding After an iPhone Leaves

**Severity:** low. The sounding rate of an idle HE client goes up about 10
times, and its tx duration does not go up. Seen on an SDG-8612 with
27.2.0.101 (WM firmware build 20260515120536), 2026-11-20, in the retest of
SDG-1168.

### What happens

- With one HE client on the 5 GHz AP (HE80), the firmware sounds it with a
  unicast NDP Announcement every 6 to 30 s.
- When an HE iPhone joins, the firmware changes to trigger-based sounding of
  the first client: a broadcast NDP Announcement, then a Beamforming Report
  Poll trigger. The rate was about one per second with a MediaTek MT7921
  client (NETGEAR A8000), and 0.4 per second with an Intel AX211-class
  client. An HT client, an Apple Watch, is not needed for the change.
- Each broadcast NDP Announcement holds two HE STA Info fields with the same
  AID, the AID of the first client. The cause is not known.
- The state stays after the iPhone leaves, for at least 50 minutes. A
  reconnect of the first client that takes about 1 s does not end it. A
  disconnect of 30 s ends it.
- The idle tx duration of the first client was 74 to 132 us per second, with
  and without the iPhone.
- In 2023, with an Intel AX211 client, the rate went to about 24 per second,
  and the tx duration went up about 10 times. Both points are in SDG-1168.

### Options

Report the duplicate STA Info and the state that stays to MediaTek, with the
over-the-air captures of the retest. The captures are attached to SDG-1168.

## mt7996e Does Not Recover from a Reload of the WiFi Modules

**Severity:** low, a limit for development work only. A unit loads the WiFi
modules once at boot and never reloads them. Seen on 2026-10-20 on an
SDG-8732v with SmartOS 26.11.0.101 r36846, in the tests of SDG-10080.

### What happens

- `wifi down`, a stop of wpad, `rmmod` of mt7996e, the other mt76 drivers,
  mac80211 and cfg80211, then `kmodloader`, a start of wpad and `wifi up`.
  Each `rmmod` returns 0, the modules load again, the firmware loads, and
  `phy0` comes back.
- The firmware does not answer after that. The driver logs
  `Message 001a0034 (seq 5) timeout` and `Hardware restart was requested`
  about every 40 s, and no interface comes up.
- `page_pool_release_retry() stalled pool shutdown: id 7, 260 inflight`
  shows DMA pages from before the unload that were never released.
- Only a reboot restores the radio.

### How to load a changed WiFi module on a unit

Put the ipk into `/FLASH/persist/hotfix` and reboot. The hotfix init script
(START=09) installs it before `kmodloader` loads `/etc/modules.d`. The boot
path calls `hotfix-install.sh` without `-f`, so the ipk needs a higher
version than the installed package. Remove the ipk and reboot to return to
the image.

## mt7915 Testmode Keeps the Background Radar Phy After an Off-Channel Test

**Severity:** low. Only a testmode session reaches it, and a module reload
ends it. Found on 2026-10-27 in a source review for SDG-12894; not seen on a
unit.

### What happens

- `mt7915_tm_set_offchan()` (vendor patch 1050) sets `dev->rdd2_phy` when a
  testmode off-channel radar test starts. Nothing in testmode clears it.
  Only `mt7915_set_radar_background()` with no channel does.
- After such a test, the driver treats the band as the carrier of the
  background radar chain. `mt7915_set_radar_background()` then refuses a
  background CAC on the other band with `-EBUSY`.

### Fix

Clear `dev->rdd2_phy` and `dev->rdd2_chandef` when testmode ends the
off-channel test, or when the phy leaves testmode.
