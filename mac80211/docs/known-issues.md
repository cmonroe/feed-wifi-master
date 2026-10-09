# Known Issues

This file keeps the items that have no ticket of their own: minor faults,
limits and open questions. When an item gets a Polarion ticket, the ticket
becomes its record and the entry leaves this file.

The entries are in order of severity, most severe first. Put a new entry at
its rank.

## A Second Channel Switch Can Free a Context That a Link Still Uses

**Severity:** low. Since patch 917
(`patches/subsys/917-smartrg-wifi-mac80211-keep-the-radio-index-of-a-replaced-chanctx.patch`),
no known sequence on a SmartOS model reaches this state. Found in the
analysis of SDG-9903, 2026-10-09, from the source and from the kernel log of
one test. No trace shows the path.

### What happens

- `ieee80211_vif_use_reserved_switch()` in `net/mac80211/chan.c` counts a
  link of the old context as reserved when the link has any reservation. The
  reservation can belong to another replacement context.
- The replacement then looks complete too early. mac80211 can free the old
  context while a link and the driver still use it.
- On an SDG-8732v without patch 917, a second uplink channel switch during an
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
patch can differ from the version that lands, and patch 917 removes the only
known way to reach the state on SmartOS.

### If it is fixed later

At the next mac80211 update, check whether the backports version carries both
patches. If upstream applies them before that update, add them to
`patches/subsys/`. When both are in, remove this entry.
