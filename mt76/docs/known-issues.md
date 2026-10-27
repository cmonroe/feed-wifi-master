# Known Issues

This file keeps the items that have no ticket of their own: minor faults,
limits and open questions. When an item gets a Polarion ticket, the ticket
becomes its record and the entry leaves this file.

The entries are in order of severity, most severe first. Put a new entry at
its rank.

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
