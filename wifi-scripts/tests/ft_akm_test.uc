// Host test of 802.11r on BSSes without an FT AKM, through generate(), with
// the stub modules in tests/stubs. The code under test comes from a patch, so
// the test needs the ap.uc of a prepared build directory. Run make
// package/wifi-scripts/prepare. Then, from the wifi-scripts package directory,
// with BUILD_DIR set to the absolute path of build_dir/target-<arch> of the
// OpenWrt tree:
//   ucode -L "$PWD/tests/stubs/*.uc" \
//     -L "$BUILD_DIR/wifi-scripts-1.0/src/usr/share/ucode/*.uc" \
//     tests/ft_akm_test.uc
import * as ap from 'wifi.ap';
import * as common from 'wifi.common';
import { opts } from 'uci';

let checks = 0, fails = 0;
function check(cond, what) {
	checks++;
	if (!cond) { fails++; printf("FAIL: %s\n", what); }
}

function bss(encryption, extra) {
	opts.configs = {};
	common.flush_config();
	let config = { mode: 'ap', ifname: 'w5g-guest', ssid: 'G', encryption, network: [ 'guest' ],
	               ieee80211r: 1, ...(extra ?? {}) };
	let data = { config: { band: '5g', htmode: 'HE80' }, ap_start_disabled: null };
	let refusal = ap.generate({}, data, config, {}, {}, {});
	let lines = {};
	for (let l in split(common.dump_config(), '\n')) {
		let m = match(l, /^([a-z0-9_]+)=(.*)$/);
		if (m)
			lines[m[1]] ??= m[2];
	}
	return { refusal, lines };
}

let r = bss('owe');
check(r.refusal == null, 'OWE with ieee80211r: BSS starts');
check(r.lines.wpa_key_mgmt == 'OWE', 'OWE with ieee80211r: wpa_key_mgmt OWE only');
check(!('mobility_domain' in r.lines) && !('r0kh' in r.lines), 'OWE with ieee80211r: no FT options');

r = bss('psk2', { key: '12345678' });
check(r.refusal == null, 'WPA2 with ieee80211r: BSS starts');
check(index(split(r.lines.wpa_key_mgmt ?? '', ' '), 'FT-PSK') >= 0, 'WPA2 with ieee80211r: FT-PSK offered');
check('mobility_domain' in r.lines, 'WPA2 with ieee80211r: mobility_domain written');

r = bss('wpa2', { auth_server_addr: [ '192.0.2.1' ] });
check(r.refusal == 'FT_KEY_CANT_BE_DERIVED', 'WPA2-Enterprise with ieee80211r and no secret: refused');

r = bss('wpa2', { auth_server_addr: [ '192.0.2.1' ], auth_server_shared_secret: 's' });
check(r.refusal == null, 'WPA2-Enterprise with ieee80211r and a secret: BSS starts');
check('r0kh' in r.lines, 'WPA2-Enterprise with ieee80211r and a secret: r0kh written');

printf("%s: %d checks, %d failed\n", fails ? 'FAIL' : 'ok', checks, fails);
exit(fails ? 1 : 0);
