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

### What is not known

- How phones and other clients take two RSNEs under one SSID. Not tested.
- Whether an iPhone or an iPad still uses FT between the two families.

### If it matters later

- An explicit `ieee80211w` in the uci section overrides the default on each
  node. One value for the whole mesh gives one RSNE again: 0 gives the form
  of 26.9.0.101, without beacon protection on EHT.
- By the code, the value 1 on all nodes gives the three AKMs and
  `beacon_prot=1` on WiFi 6 nodes too (`iface_mfp()`). Not tested.
