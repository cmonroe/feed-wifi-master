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

## A Retry of the First Management Frame Is Dropped as a Duplicate

**Severity:** low. The station sends its request again after about 0.1 s,
and the roam completes. Seen once in the tests of SDG-9913 on 2026-10-12, on
an SDG-8733A with SmartOS 26.11.0.101. The cause comes from the source and
from that one capture.

### What happens

- `__sta_info_alloc()` sets `last_seq_ctrl[]` to 0xffff for the 16 TIDs. The
  array has one more entry, for management frames and for data frames without
  QoS (`seqno_idx = IEEE80211_NUM_TIDS` in `ieee80211_parse_qos()`). That
  entry stays 0.
- `ieee80211_rx_h_check_dup()` drops a frame with the retry bit whose
  sequence control field equals the stored value.
- So on a new station entry, a retry with sequence number 0 counts as a
  duplicate when the AP did not receive the first transmission. The radio
  acknowledges the frame, and mac80211 drops it.
- hostapd makes a new station entry at each authentication and at each FT
  request over the DS. In the captures, wpa_supplicant on mt76 started its
  management frames to a new AP with sequence number 0.
- The case was an FT over the DS with PMF. The capture had the Reassociation
  Request three times, with sequence number 0, two of them with the retry
  bit, and an ACK. hostapd logged nothing. After 110 ms the station sent the
  request with sequence number 1, and the answer came in 4 ms.

### What is not known

- The duplicate counter of that station entry (`rx_stats.num_duplicates`). It
  was not read.
- Whether other clients start with sequence number 0.

### If it is fixed later

The loop in `__sta_info_alloc()` must cover all entries of the array. The
upstream kernel had the same lines on 2026-10-12, so the fix belongs
upstream first.

## A Hidden Beacon Entry Keeps a Reference for Each Nontransmitted Entry

**Severity:** low, a memory leak in a rare case. Found in the code of
backports 7.2 on 2026-10-19, in the review of patch 923. Not seen on a unit.

### What happens

- cfg80211 puts the entry with the SSID of a hidden BSS into the group of
  the beacon entry without the SSID (`hidden_beacon_bss`). `bss_ref_get()`
  and `bss_ref_put()` count each reference of a member on the beacon entry
  too.
- When cfg80211 makes a nontransmitted entry of a multi-BSSID set,
  `__cfg80211_bss_update()` calls `bss_ref_get()` on the transmitter. If the
  transmitter is a member of a hidden group, this call also counts on the
  beacon entry.
- `bss_ref_put()` of the nontransmitted entry gives its reference back to
  the transmitter only. So the beacon entry keeps one reference for each
  nontransmitted entry that cfg80211 made, and it is not freed when it
  expires.
- `cfg80211_combine_bsses()` adds the full refcount of a member, the
  references of its nontransmitted entries included, with the same result.
- The case needs a multi-BSSID AP whose transmitted BSS has a hidden SSID,
  and a station that knows that SSID.

### If it is fixed later

The release of a nontransmitted entry must also release the count on the
hidden beacon entry of its transmitter. The upstream kernel has the same
code, so the fix belongs upstream first. Patch 923 moves only the share that
the beacon entry mirrors, so it does not depend on this fix.
