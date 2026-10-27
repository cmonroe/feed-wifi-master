# DFS Radar Detection Test

This procedure runs a radar detection rate test: the AP holds its channel
after a radar, and every radar event is counted. Root access is required.
The commands apply to mt7915 and mt7996 radios.

## Build requirement

Builds from a SmartOS superproject at or after commit `41bdeb20d11d`
(2026-10-11) report every radar event. Older builds report once per
channel set: before superproject commit `8adef7c23c24` (2026-10-06) the
mt7996 firmware stops after the first event, and before `41bdeb20d11d`
the driver drops every repeat on the same channel.

## Procedure

1. Configure the 5 GHz AP on a DFS channel and wait for the CAC to end.
   Alternative: write `1` to `/sys/kernel/debug/ieee80211/<phy>/dfs_skip_cac`.
2. Hold the channel: `hostapd_cli -i <ifname> raw DFS_DETECT_MODE 1`.
   Mode 1 holds the AP chain, 2 the background chain, 3 both. The mode
   lives in the running hostapd: a restart of hostapd clears it, and a
   `wifi reload` that keeps the interface keeps the mode.
3. Zero the counters: write `1` to
   `/sys/kernel/debug/ieee80211/<phy>/dfs_radar_count_reset`. Record the
   value of `/sys/kernel/debug/ieee80211/<phy>/mt76/dfs_hw_pattern`.
4. Run the radar trials.
5. Read `/sys/kernel/debug/ieee80211/<phy>/dfs_status`. The file has
   three parts. The first line gives the number of radar events on the
   wiphy and the last event. The link table lists each 5 GHz link by
   interface name and type, and the background chain, with the channel,
   the state (`cac`, `operating`, `no-dfs`, `idle` or `monitoring`) and
   the CAC timer in seconds. The channel table lists each radar channel
   once: the DFS state, the running timer (`nop` or `cac`) and its
   seconds left, the radar events that covered the channel on the
   operating chain (`radar`) and on the background chain (`bg-radar`),
   and the chain that covers it (`op`, `bg` or `-`). An event covers
   every 20 MHz sub-channel of the operating width. Every row has the
   same number of fields, so `awk '$1 == 64 {print $6}'` prints the radar
   count of channel 64.
6. Restore the channel switch: `hostapd_cli -i <ifname> raw DFS_DETECT_MODE 0`.

## Writes from bash

The bash on the unit writes `echo 1 > <file>` as two writes, `1` and a
newline. The kernel takes the value from the first write and rejects the
newline alone, so bash prints a write error after the value took effect.
Write with `printf 1 > <file>`, or run the command in `/bin/sh`.

## Cross checks

- `/sys/kernel/debug/ieee80211/<phy>/mt76/dfs_hw_pattern` counts the radar
  reports of the firmware for the whole device, on every chain, and has no
  reset. Compare its change over the trials with the wiphy count. A larger
  change can mean that two reports arrived before mac80211 delivered the
  first, or that another chain of the device reported.
- `dmesg` shows one `RADAR DETECTED` line per firmware report.
- `logread` shows one `DFS-RADAR-DETECTED` line from hostapd per event.
- Write `1` to `/sys/kernel/debug/ieee80211/<phy>/mt76/radar_trigger` to
  emulate a radar on the operating chain without a signal generator, or
  `2` for the background chain.
