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
- The Pixel 3a answered the BSS transition requests in both settings. In a
  repeat with its screen on, it answered 8 of 10 with the default settings
  and 9 of 9 with `ieee80211w=1`. A first pair of runs gave 6 of 9 and 2 of
  10, but the phone rebooted in one run and went to sleep in the other.

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

## A Client With a MAC-Bound PSK Cannot Use the Main Passphrase for SAE

**Severity:** minor. It affects only a client whose MAC address an
Additional PSK names, and only for SAE. Found in the work on SDG-13120
on 2026-11-11. The SAE part was seen on an SDG-8614 with SmartOS
27.2.0.101; the WPA2 part comes from the source.

### What happens

- For SAE, hostapd uses the `sae_password` entry for the address of the
  client and no other password. hostapd patch
  `999-smartrg-SAE-Let-each-STA-reach-its-password-with-sae_track_password.patch`
  keeps that rule when `sae_track_password` is on.
- For WPA2-PSK, `hostapd_get_psk()` also gives the PSKs that apply to
  every client, the main passphrase included. So over WPA2, the same
  client can also use the main passphrase.
- On a `sae-mixed` BSS, such a client therefore joins with the main
  passphrase over WPA2, but gets status 15 over SAE.

### If it matters later

- The web UI can say that a PSK with a MAC replaces the main passphrase
  for that client over WPA3.

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

## A Key That Contains `|mac=` Breaks the SAE Password File

**Severity:** minor. The fault needs a key that contains the text `|mac=`.
Found in the work on SDG-13120 on 2026-11-11, from the source; not tested
on a unit.

### What happens

- `iface_sae_stations()` in `files-ucode/usr/share/ucode/wifi/ap.uc`
  writes each key as `<key>|mac=<address>`, with `|vlanid=<vid>` when the
  station has a VLAN.
- `parse_sae_password()` of hostapd ends the password at the first
  `|mac=`. A key that contains `|mac=` therefore gives a wrong password
  and an address that `hwaddr_aton()` can refuse. A refused line makes
  hostapd refuse the whole file, and the BSS does not start.
- This applies to the key of each `wifi-station` section. When a station
  has no MAC, it also applies to the main key of the BSS, which then goes
  into the same file.
- The text `|vlanid=`, `|pk=` or `|id=` in a key does no harm: hostapd
  searches for them only after the address.

### If it matters later

- A check of the key in `bss_refusal()` can refuse such a key with a
  reason, instead of a BSS that does not start.
- The web UI can refuse `|mac=` in a key.

## A Backport of OpenWrt PR 24641 Would Turn Hairpin Off Under Patch 035

**Severity:** none in the current tree. It becomes a fault at the first
backport from OpenWrt master after a merge of
https://github.com/openwrt/openwrt/pull/24641 in its current form. The PR
was open on 2026-11-20. Found on 2026-11-20 in a review of that PR.

### What happens

- Patch `035-wifi-scripts-isolate-bridged-AP-stations-by-default.patch`
  gives a bridged AP `ap_isolate=1` unless `multicast_to_unicast` is 0. An
  unset option counts as on, as in the shell scripts. With `ap_isolate=1`,
  frames between two stations of one BSS pass through the bridge, so the
  client isolation of bridger applies to them.
- netifd sets the hairpin mode of the bridge port from the same option:
  `hairpin = mcast_to_ucast || proxyarp` in `system_bridge_set_wireless()`.
  For an unset option, netifd uses its own default, which is on for an AP
  port. `files/lib/netifd/wireless-device.uc` sends the option to netifd
  only when it is set (`config.multicast_to_unicast != null`).
- So today `ap_isolate` and the hairpin mode have the same value. With the
  isolation off in bridger, two stations of one BSS reach each other through
  the bridge.
- PR 24641 removes the `!= null` guard and sends 0 for an unset option.
  netifd then turns the hairpin mode off, while patch 035 still sets
  `ap_isolate=1`. Two stations of one BSS then cannot reach each other,
  also with the isolation off.
- This applies to each AP section without an explicit
  `multicast_to_unicast`. A search of the smartrg-master branch of
  internal-feed-smartrg, internal-feed-bsp and cdt on 2026-11-20 found no
  code that sets the option in the wireless configuration.

### No conflict shows it

PR 24641 changes `wireless-device.uc`, and patch 035 changes `ap.uc`. The
backport and the patch apply without a conflict, so the build gives no
sign of the fault.

### If it matters later

- Before each backport of wifi-scripts, check the state of PR 24641.
- If it merged in its current form, keep the two values the same. One
  option: set `multicast_to_unicast` to 1 on each AP section, which keeps
  `ap_isolate` and the hairpin mode on. A second option: keep the
  `!= null` guard with a local patch.
- For the first option: `set_fast_leave.sh` in internal-feed-bsp writes the
  bridge port `multicast_to_unicast` itself, 1 on a hub and 0 on a
  satellite, and does not change the hairpin mode. Check that the script
  still runs after netifd adds the port.
- A change of patch 035 that sets `ap_isolate=0` for an unset option also
  keeps the two values the same. It opens again the fault that patch 035
  closes: mac80211 forwards between two stations of one BSS without the
  bridge.
- A comment of 2026-10-05 on PR 24641 proposes the change of patch 035
  for OpenWrt. If OpenWrt takes that change instead of PR 24641, patch 035
  can go.
