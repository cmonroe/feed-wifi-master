# Known Issues

This file keeps the items that have no ticket of their own: minor faults,
limits and open questions. When an item gets a Polarion ticket, the ticket
becomes its record and the entry leaves this file.

The entries are in order of severity, most severe first. Put a new entry at
its rank.

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
