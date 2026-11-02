# Known Issues

This file keeps the items that have no ticket of their own: minor faults,
limits and open questions. When an item gets a Polarion ticket, the ticket
becomes its record and the entry leaves this file.

The entries are in order of severity, most severe first. Put a new entry at
its rank.

## WiFi 7 and WiFi 6 Nodes of One Mesh Send Different RSNEs for a WPA2 SSID

**Severity:** not known. Roams over the air work. Found in the tests of
SDG-9913 on 2026-10-12, SmartOS 26.11.0.101, on a mesh with four WiFi 7
satellites (SDG-8733A, SDG-8734v, SDG-8712v, SDG-9712o) and four WiFi 6
satellites (SDG-8632, SDG-8614, SDG-8610, 854-v6). The SSID had
`encryption='psk2+ccmp'` and FT.

### What happens

- `iface_auth_type()` in `files-ucode/usr/share/ucode/wifi/ap.uc` gives
  `ieee80211w` the default 1 when the radio has an EHT htmode and the BSS
  uses WPA2. Its comment names IEEE 802.11be-2024 12.12.9: beacon protection
  on EHT needs PMF. The rule came with the backport from OpenWrt master of
  2026-10-08.
- A WiFi 7 node then writes `ieee80211w=1`, `beacon_prot=1` and
  `wpa_key_mgmt=WPA-PSK FT-PSK WPA-PSK-SHA256`. A WiFi 6 node with the same
  uci section writes `ieee80211w=0` and `wpa_key_mgmt=WPA-PSK FT-PSK`.
- The RSNE of the WiFi 7 nodes has three AKMs and the capabilities 0x008c.
  The RSNE of the WiFi 6 nodes has two AKMs and the capabilities 0x000c. On
  SmartOS 26.9.0.101 of 2026-10-07, all eight nodes sent the second form.
- Since the SDG-9935 fix, an AP leaves out WPA-PSK-SHA256 when its
  `ieee80211w=1` comes from the EHT default. By the code, both families then
  send the same two AKMs, and only the capabilities differ. Not measured.
  By the code, the FT over the DS still fails: `wpa_compare_rsn_ie()` also
  compares the capabilities. Not tested.
- A wpa_supplicant station (v2.12-devel) failed each FT over the DS between a
  WiFi 6 and a WiFi 7 node, 10 of 10, with "FT: RSNE mismatch between
  Beacon/ProbeResp and FT protocol Reassociation Response frame".
  `ft_rx_action()` of wpa_supplicant calls `sme_associate()` and does not
  load the RSNE of the target AP, so `wpa_ft_validate_reassoc_resp()`
  compares the response with the RSNE of the AP that the station leaves.
- Over the air, the same station roamed between the two families 15 of 15
  times, with and without PMF.

### Phones

On 2026-11-02, SmartOS 26.11.0.101 r37973, on a mesh with three WiFi 7
satellites (SDG-8712v, SDG-8733A, SDG-P72v-Plus) and three WiFi 6 satellites
(SDG-8632, SDG-8610, 841-t6). The 5 GHz RSNEs of the two families differed
only in the capabilities. BSS transition requests steered each phone to a
node of the other family.

- An iPhone 13 Pro roamed between the families 4 times with FT over the DS,
  each in 50 to 70 ms, and 2 times with FT over the air. None failed, and
  none fell back to a full association.
- A Pixel 3a used FT over the air: 4 of 4 roams between the families worked.
  Its full associations happened within one family too.
- No Android phone used FT over the DS, so that case stays open.

Later on the same day, on the same mesh, after the tests of SDG-13132, the
iPhone 13 Pro joined the SSID again with a new private address. It had used
SAE on this SSID during those tests. Ten steer rounds per setting:

- Default settings: the iPhone did a full association in 10 of 10 roams,
  also in the 4 roams between two WiFi 6 nodes, whose RSNEs are the same. So
  the RSNE difference does not cause this.
- `ieee80211w=1` on the 2.4 and 5 GHz BSSes of every node, with
  `wpa_key_mgmt=WPA-PSK FT-PSK` set through `hostapd_bss_options`: the
  iPhone used FT in 10 of 10 roams, 8 of them over the DS.
- The Pixel 3a answered fewer BSS transition requests with PMF on every
  node: 6 of 9 with the default settings, 2 of 10 with `ieee80211w=1`. It
  rebooted once, and it went to sleep at the end, so the counts are weak.

### What is not known

- Whether an Android client that uses FT over the DS fails between the two
  families, as wpa_supplicant does.
- Why the iPhone 13 Pro used FT in one session and not in the other, with
  the same default settings.

### If it matters later

- An explicit `ieee80211w` in the uci section overrides the default on each
  node. One value for the whole mesh gives one RSNE again: 0 gives the form
  of 26.9.0.101, without beacon protection on EHT.
- By the code, the value 1 on all nodes gives the three AKMs and
  `beacon_prot=1` on WiFi 6 nodes too (`iface_mfp()`). Not tested with
  the three AKMs; the test above removed WPA-PSK-SHA256 through
  `hostapd_bss_options`.

## A `wifi up` During a Setup of the Same Radio Can Leave the 5 GHz AP Down

**Severity:** minor. Only a test tool started the overlap, and no product
code that starts it is known (see Tests). Found on 2026-10-18 in the
teardown of the shakedown test
`dfs_startup_radar`, on an SDG-8712vlite controller (`mt7996e`), SmartOS
26.11.0.101 r36841 of sos-2602.

### What happens

- The teardown turned `background_radar` on again with `wifi reload`.
  netifd started a setup of radio1 at 07:01:40.188.
- 4.3 s later, the restart undo of the test ran `wifi up radio1`, because
  netifd reported radio1 down and pending. netifd started a second setup at
  07:01:44.513.
- At 07:01:45.9, hostapd logged "Failed to set beacon parameters" and
  "hostapd.add_iface failed for phy phy0.1 ifname=wifi5g".
- After that, hostapd had no `wifi5g` and no `w5gM`: `ubus list` showed only
  `hostapd.wifi2g`. netifd reported radio1 up. Nothing set up the radio
  again, and the AP was still down 7.5 min later.
- "Failed to set beacon parameters" also appears in runs where the AP comes
  up, so that line alone does not mark the fault.

### Tests

- 2026-10-18, same build: `wifi up radio1` 0.5 s to 1.5 s after a
  `wifi reload` that restarts radio1, with radio1 down and pending, 5 tries:
  no failure. With the two test runs, 7 overlaps are on record and 1 failed.
- 2026-10-19, r36845-6b6d3c7d8d of sos-2610, with `autochannel` 1 and
  `background_radar` 1: two `wifi reload` calls 0.3 s to 3 s apart, 26 tries
  with radio1 down and pending at the second reload: no failure.
- No code in the smartrg-master branch of internal-feed-smartrg runs
  `wifi up` with a radio or a `network.wireless` up during a reload (search
  of 2026-10-19). The product restarts a radio with `wifi reload` after a
  uci commit.

### What is not known

- Which of the two setups failed. The log lines of the second setup appear
  after the failure.
- Why no setup ran after the failed `hostapd.add_iface`, and whether the
  setup script reports that failure to netifd.

### If it matters later

- The trigger is a `wifi up <radio>` while
  `ubus call network.wireless status` reports the radio with `up` false and
  `pending` true.
- shakedown commit "lib: wait while the radio is pending before the restart
  undo" removed the trigger from the test tool.
