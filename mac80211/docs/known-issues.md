# Known Issues

This file keeps the items that have no ticket of their own: minor faults,
limits and open questions. When an item gets a Polarion ticket, the ticket
becomes its record and the entry leaves this file.

The entries are in order of severity, most severe first. Put a new entry at
its rank.

## A Late AP Swap Can Abort the Channel Switch of the Station

**Severity:** not known, possibly higher than the other entries. Seen twice
on 2026-10-09 in the work on SDG-9903, SmartOS 26.9.0.101: on an SDG-8733A
in a test that started the AP switch late, and on an SDG-8732v with patch 919
and the hostapd retry in 1 of 8 normal switches.

### What happens

- On a mesh follower, the station and the APs reserve one channel context.
  The station is ready at its switch time and waits until the APs are ready
  too (`ieee80211_vif_use_reserved_switch()`).
- If the AP countdown ends after the switch time of the station, the swap
  comes late. The uplink then already sends its beacons on the new channel,
  without a CSA element.
- `ieee80211_sta_process_chanswitch()` takes such a beacon, while
  `csa_active` is set and `waiting_bcn` is not, as an aborted switch. It
  calls `ieee80211_sta_abort_chanswitch()`, which ends in
  `drv_abort_channel_switch`.
- The APs and the radio are on the new channel, but the station keeps the old
  channel in its state, and wpa_supplicant logs no `CTRL-EVENT-CHANNEL-SWITCH`.
  On the SDG-8733A the state stayed wrong for more than 90 s. On the SDG-8732v
  the next uplink switch, back to that channel, ended it after 50 s.

### Why an AP countdown ends late

- A retune of the radio during the countdown uploads the AP beacons again and
  restarts the countdown (`mt7996_set_channel()`,
  `mt7996_mac_update_beacons()`). The return of a scan and a management frame
  on the target channel (`drv_remain_on_channel`) did this in the traces of
  SDG-9903.
- The late AP on the SDG-8732v had no trace, so its cause is not known.

### What is not known

- What the wrong state does to traffic and to later switches.
  `mesh-client-besthop.sh` scans the frequency that `wpa_cli status`
  reports; if that frequency stays old, it scans a channel without its
  uplink. Not tested.
- How often it happens in the field.

## A Second Channel Switch Can Free a Context That a Link Still Uses

**Severity:** low. Since patch 919
(`patches/subsys/919-smartrg-wifi-mac80211-keep-the-radio-index-of-a-replaced-chanctx.patch`),
no known sequence on a SmartOS model reaches this state. Found in the
analysis of SDG-9903, 2026-10-09, from the source and from the kernel log of
one test. No trace shows the path.

### What happens

- `ieee80211_vif_use_reserved_switch()` in `net/mac80211/chan.c` counts a
  link of the old context as reserved when the link has any reservation. The
  reservation can belong to another replacement context.
- The replacement then looks complete too early. mac80211 can free the old
  context while a link and the driver still use it.
- On an SDG-8732v without patch 919, a second uplink channel switch during an
  open AP channel switch reached this state. The kernel logged WARNs in
  `ieee80211_recalc_chanctx_chantype()` and in
  `ieee80211_link_use_reserved_reassign()` (err -22), then in the driver
  calls, and the station dropped the backhaul.

### Why it stays open

Two upstream patches by Zhiling Zou, posted to linux-wireless on 2026-08-03,
fix the count and the free. Both carry `Fixes: 5bcae31d9cb1 ("mac80211:
implement multi-vif in-place reservations")` and `Cc: stable`:

- "wifi: mac80211: count only matching reservations in reserved switch"
- "wifi: mac80211: keep paired old chanctx alive"

On 2026-10-09 the list archive showed no review and no note that either one
was applied, and backports 7.2 has neither. A local copy of an unreviewed
patch can differ from the version that lands, and patch 919 removes the only
known way to reach the state on SmartOS.

### If it is fixed later

At the next mac80211 update, check whether the backports version carries both
patches. If upstream applies them before that update, add them to
`patches/subsys/`. When both are in, remove this entry.
