# Known Issues

This file keeps the items that have no ticket of their own: minor faults,
limits and open questions. When an item gets a Polarion ticket, the ticket
becomes its record and the entry leaves this file.

The entries are in order of severity, most severe first. Put a new entry at
its rank.

## A Concurrent Scan Makes the Reconnect to an Expired BSS Entry Fail

**Severity:** low. One connection attempt fails, and wpa_supplicant ignores
the BSS for 10 s. Seen once in six runs on 2026-10-20, in the tests of
SDG-10080, on an SDG-8732v satellite with SmartOS 26.11.0.101 r36846.

### What happens

- When cfg80211 has no current entry for the BSS, the authentication gets
  ENOENT. `wpa_driver_nl80211_authenticate()` then starts a scan of that
  channel for the SSID, and sends the authentication again when the scan
  ends.
- It returns the result of the scan trigger. If another scan runs on the
  interface, the trigger fails, and SME counts a failed connection and puts
  the BSSID on the ignore list.
- On a satellite, the other scan is mostly the `iw ... scan freq <f> ssid`
  of `mesh-client-besthop.sh`, every 10 s.
- In the test, the satellite then joined another satellite. With only one
  parent, wpa_supplicant clears the ignore list after its next scan, which
  also refreshes the entry.
- cfg80211 has no current entry when the entry expired, or, with patch 922
  of mac80211, when a channel switch moved it. So the case comes at the first
  reconnect after a channel switch of the parent.

### If it is fixed later

When the scan trigger fails because a scan runs, keep the authentication
parameters and start the scan when the other scan ends, instead of an error
to SME. The finding comes from the hostapd source 2026.08.07 of the build;
upstream hostap was not checked.

## The First FT Between Two APs Waits for a Key Pull That Gets No Answer

**Severity:** low. The roam completes, 1 s late over the DS and 0.1 s late
over the air. Found in the tests of SDG-9913 on 2026-10-12, on a mesh of an
SDG-9000 with eight WiFi 6 and WiFi 7 satellites, WPA2 with FT-PSK, with
SmartOS 26.9.0.101 and with 26.11.0.101. Release 26.6.9.1 has the same
setting.

### What happens

- `servicevlans.sh` sets `ft_psk_generate_local='0'` on each AP interface
  when roaming is on. The wifi scripts then give hostapd wildcard key holders
  (`r0kh=ff:ff:ff:ff:ff:ff *`, `r1kh=00:00:00:00:00:00 00:00:00:00:00:00`).
- At the first FT of a station to an AP, that AP has no PMK-R1.
  `wpa_ft_pull_pmk_r1()` asks the AP of the first association (the R0KH) for
  it. With a wildcard R0KH, the sequence number request and the pull go to
  the broadcast address.
- On the R0KH, `wpa_ft_rrb_oui_rx()` sets `no_defer` for a frame with a
  multicast destination. With `no_defer`, `wpa_ft_rrb_rx_pull()` drops a pull
  from a sender whose sequence number it does not know yet. The R0KH answers
  only the sequence number request.
- The target AP learns the address of the R0KH from that answer. It sends
  the pull again only when `rkh_pull_timeout` ends (1000 ms), or when the
  station repeats its request. The second pull has a unicast destination and
  gets its answer.
- Over the DS, the FT response came after 1005 to 1152 ms: 13 of 22 roams
  after a reboot of the mesh with 26.11.0.101, 11 of 22 with 26.9.0.101.
- Over the air, the first FT authentication got no answer. The station
  (wpa_supplicant) sent it again after 108 ms, and the answer came 2 to 12 ms
  later. This happened at 7 of 7 APs at their first contact with an R0KH, and
  at 0 of 7 for a second station 90 s later.
- The 1 s delay came back after a restart of hostapd on all nodes (3 of 16
  roams over the DS). By the code, the R0KH keeps the sender for
  `rkh_pos_timeout` (86400 s by default), so the same must happen after a day
  without a pull between two APs. Not measured.

### What removes it

- With `ft_psk_generate_local=1`, an AP derives the PMK-R1 of an FT-PSK
  station from the passphrase, and no pull exists. A test on the same mesh
  with 26.11.0.101 gave 79 good roams of 79, no key holder frame on the
  bridge of any AP, and FT responses over the DS within 15 ms (53 of 54; one
  took 1.5 s on the way between two APs with a wireless uplink, cause not
  known).
- hostapd uses that option only for FT-PSK. An FT-SAE station still needs
  the pull, and the wifi scripts write no key holders when the option is 1.
  So the option fits only a BSS without SAE. The default of the wifi scripts
  is such a rule (`ft_psk_local_default()` in `ap.uc`); the explicit 0 of
  `servicevlans.sh` overrides it.
- The 0 comes from the uci defaults of intellifi. OWRT-6801 set the option
  to 1 in 2021, and OWRT-7263 set it to 0 in 2023 "to better support WPA3".
  The commit gives no other reason; an FT-SAE station needs the pull, which
  fits that subject. SDG-3585 copied the 0 into
  `servicevlans.sh`. The rule of `ap.uc` now chooses the value by
  authentication type, so the explicit 0 is no longer needed.
- A fix must delete the option, not only remove the command that sets it.
  `servicevlans.sh reload` runs at each boot, and the `'0'` stays in
  `/etc/config/wireless` of each upgraded unit until the script deletes it.
- The hub copies the servicevlans config to its satellites, and
  `servicevlans.sh` on each unit writes the wireless option from it. During a rollout, an AP with 1 stores no PMK-R0, so an AP with
  0 that pulls from it gets no key, and the station falls back to a full
  association. Not checked: whether the cloud or a CDT sets the option.

### What is not known

- What a client does that does not repeat its FT request.
- How often a roam in the field is a first contact of two APs.

## An iPhone Reassociates After Most Channel Switches of Its AP

**Severity:** low. The iPhone is back within 0.3 s. Found in the client test
of SDG-9903 on 2026-10-09, with an iPhone 13 Pro on the iOS release that was
current on that date, on an SDG-8712v with SmartOS 26.9.0.101. The cause is
not known.

### What happens

- The iPhone, with its screen off, was on a 5 GHz AP with `dtim_period=3`.
  The AP followed its mesh uplink to a new channel with a channel switch
  announcement.
- After 28 of 40 switches, the iPhone sent a disassociation 0.6 to 15 s
  after the switch. Then it authenticated and associated again, and the AP
  had it back within 0.09 to 0.30 s.
- The AP received the disassociation on the new channel, so the iPhone
  followed the switch before it left.
- It happened at every count that the test used: count 4 in 8 of 11
  switches, count 3 in 13 of 20, count 2 in 3 of 5, count 1 in 4 of 4.
- A Mac on the same AP, awake, stayed connected in 24 of 25 switches.

### What is not known

- The reason code of the disassociation. hostapd logs it only at debug
  level, in `hostapd_disassoc_sta()`. On SmartOS, `hostapd_cli log_level
  DEBUG` did not send that level to the syslog.
- Whether the iPhone does the same with its screen on, with another DTIM
  period, or on an AP outside a mesh.

### If it matters later

- A capture of the management frames on a monitor interface of another unit
  on the same channel gives the reason code.
- If the reason points to the AP, for example an element that changes with
  the channel, open a ticket.

## hostapd Logs a Station VLAN Error at Some Joins

**Severity:** low. No effect on the station is known. Seen in the tests of
SDG-9913 on 2026-10-11 and 2026-10-12, on each build of the tests: release
26.6.9.1 (10 lines in 156 joins on a mesh of an SDG-8733v), 26.9.0.101, and
26.11.0.101 with and without the commits of SDG-9913.

### What happens

- hostapd logs at error level `nl80211: NL80211_ATTR_STA_VLAN (addr=<station>
  ifname=<AP interface> vlan_id=0) failed: -2 (No such file or directory)`,
  and at debug level "could not bind the STA entry to vlan_id=0".
- The line comes from `ap_sta_bind_vlan()`. hostapd binds the station to its
  interface at a moment when the kernel has no entry for that station.
- In the one case that was read, the line came when the FT request of a
  station arrived over the DS, 0.3 s before its Reassociation Request. The
  station joined normally.

### What is not known

- Which other paths give the line. On 2026-10-20 the line also came at the
  removal of a station, in the same millisecond as "deauthenticated due to
  local deauth request". That was a satellite backhaul station on an
  SDG-8712v controller with 26.11.0.101.
- Whether a station with a VLAN other than 0 can lose its VLAN this way.

### If it matters later

Read the callers of `ap_sta_bind_vlan()` that run before the kernel has the
station entry.

## FT and FILS Authentication Replies Go to the MLD Address of the Client

**Severity:** low. The shipped settings cannot reach it: no SmartRG package
sets the `mlo` option of a `wifi-iface`, so no unit runs an AP MLD. Found
from the code during the investigation of SDG-10128 on 2026-10-20. Not tested;
the lab has no AP MLD.

### What happens

- Patches 192 and 193 address the Authentication replies of open system and
  SAE to the link address that sent the request. A client of an AP MLD can
  pick a new link address for each attempt, and the driver maps the MLD
  address back to a link only while it knows the current link address.
- The FT and FILS replies do not have this fix:
  - `handle_auth_ft_finish()` sends to the `dst` that the FT code passes,
    and `wpa_ft_process_auth()` and `ft_finish_pull()` pass `sm->addr`.
    `handle_auth()` creates the WPA state machine with `sta->addr`, which
    is the MLD address of such a client.
  - `handle_auth_fils_finish()` sends to `sta->addr`.
- So an FT or FILS reply to a client that changed its link address goes to
  an address that no station answers.

### What would close it

Use `sta->auth_link_addr` for the reply of an MLD client in both functions,
as patch 193 does for SAE.

## A usteer Ban Does Not Stop a Client of an AP MLD

**Severity:** low. The shipped settings cannot reach it, for the same reason
as the entry above. Found from the code during the investigation of
SDG-10128 on 2026-10-20. Not tested.

### What happens

- hostapd names a client of an AP MLD by its MLD address in the ubus
  notifications (`hostapd_ubus_sta_addr()`). usteer kicks that address
  with `del_client` and a `ban_time`, and `hostapd_bss_ban_client()` stores
  the ban under it.
- `hostapd_ubus_handle_event()` looks up the ban by the source address of
  the received frame. For an Authentication request of a client that is not
  associated, that is the link address (the header of patch 192 states this).
- When the link address of the client differs from its MLD address, the
  lookup misses. The client can authenticate and associate again at once,
  and the ban has no effect.

### What is not known

- How many clients use their MLD address as a link address. For such a
  client the ban works.

### What would close it

In `hostapd_ubus_handle_event()`, also look up the MLD address that
`hostapd_process_ml_auth()` reads from the Authentication frame.

## A Station That Changes Its MAC Address Gets No Holdoff

**Severity:** low. Each attempt of such a station writes one line to the
log. No such station is known. Found from the code in the work on SDG-10128
on 2026-10-25.

### What happens

- Patch 982 counts the repeats of a rejected answer for each MAC address,
  for 10 s, and writes them as one line.
- A station that uses a new random MAC address for each attempt gets a new
  entry each time, so each attempt writes one line.
- The count is kept for 64 stations per BSS. With more addresses, each new
  address replaces the oldest entry, which writes its pending count.

### What would close it

- A limit for each BSS on the lines of rejected answers per second, in
  addition to the count for each MAC address.

## wpa_supplicant Logs a 4addr Failure at Each Reconfiguration of an MLO Station

**Severity:** low. The station works. Seen on 2026-10-29 and 2026-10-30 on an
SDG-8734v satellite with SmartOS 26.11.0.101, smartrg-master of 2026-10-29.
The cause is not investigated.

### What happens

- At each "Set MLD interface w56g-Client radio <n> config", wpa_supplicant
  logs `nl80211: Failed to enable/disable 4addr`.
- `iw dev w56g-Client info` shows `4addr: on`, and the controller creates
  the WDS interface of the satellite as usual. Data flows.

### What is not known

- The error code, and whether the call goes to a link or to the MLD netdev.
- Whether the call fails because the netdev is up, or a member of a bridge,
  at that time.

### If it matters later

- A debug log of wpa_supplicant around `nl80211_set_4addr_mode()` gives the
  error code and the netdev.

## Some Removals by the AP Report No Reason Code

**Severity:** low. The `disassoc` notification still names the AP as the
side that ended the association (`local` true). Only `reason` is missing.
Found in the review of SDG-10058 on 2026-10-19, from the code.

### What happens

- Patch 993 adds `reason` to `disassoc`. A removal stores the cause on the
  station before the authorized flag clears, and the notification gives it.
- These removals free the station with `ap_free_sta()` and store no cause.
  Their `disassoc` has `local` true and no `reason`, although hostapd sends
  a reason code to the station:
  - `ieee80211_tkip_countermeasures_start()`, reason 14.
  - `del_client` on ubus with the broadcast address, through
    `hostapd_free_stas()`: the reason of the request.
  - The stop of a BSS: `hostapd_bss_deinit_no_free()` frees the stations
    first, then sends the broadcast deauthentication with reason 3.
    `broadcast_deauth` is 1 by default.
- Patch 994 logs its disconnect line where the cause is stored, so these
  removals give no syslog line either.
- `deauthenticate` and `disassociate` of the control interface with
  `tx=0` send no frame, so no `reason` is correct there.

### What would close it

- Store the cause before `ap_free_sta()` in
  `ieee80211_tkip_countermeasures_start()`.
- Give `hostapd_free_stas()` a reason parameter, and pass it from each
  caller. The callers are upstream code, so each one adds a hunk that a
  later update of hostapd can conflict with.

## An AP-Initiated Disconnect Leaves the Link Station of an AP MLD

**Severity:** low. The station rejoins, and a later authentication removes the
entry. Seen on 2026-10-30 on an SDG-8733v with an SDG-8734v MLO station, with
SmartOS 26.11.0.101 r37973. Only the disconnect by the AP is affected.

### What happens

- `ap_sta_ml_disconnect()` looks for the link stations in
  `interfaces->iface[i]->bss[0]`. The AP MLD `w56gM` is a later BSS of its
  radios, so `hostapd_is_ml_partner()` never matches, and only the association
  link is handled.
- The link station keeps `WLAN_STA_ASSOC` until the next authentication.
- A second `del_client` in that time acts on the stale entry.
- If the MLD authenticates again at once, `ap_sta_re_add()` runs on that entry
  and patch 994 logs `disconnect origin=sta frame=auth`, about 0.2 s after the
  correct line of the disconnect.
- A disconnect by the station is not affected: `for_each_mld_link()` finds
  every link.

### What would close it

- Walk the links in `ap_sta_ml_disconnect()` with `for_each_mld_link()`.

## Some Authentications Give No Disconnect Line

**Severity:** low. The notification still reports the end of the association.
Found in the review of SDG-10058 on 2026-10-30, from the code.

- `handle_auth()` skips `ap_sta_re_add()` for an authorized MFP station, so a
  new authentication of such a station logs no `frame=auth` line. Its later
  `disassoc` has no `reason` and no `local`.
- An MLD without MFP that authenticates again on a link that is not its
  association link: patch 994 logs nothing, because the line belongs to the
  association link. The `disassoc` still has `local` false.
- A new Association Request of a still associated MLD also reaches
  `ap_sta_re_add()`, so the line reads `frame=auth`.
