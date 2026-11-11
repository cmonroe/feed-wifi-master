// Host test of the sae_password_file that ap.uc writes for wifi-station
// sections, through generate(), with the stub modules in tests/stubs. The code
// under test comes from a patch, so the test needs the ap.uc of a prepared
// build directory. Run make package/wifi-scripts/prepare. Then, from the
// wifi-scripts package directory, with BUILD_DIR set to the absolute path of
// build_dir/target-<arch> of the OpenWrt tree:
//   ucode -L "$PWD/tests/stubs/*.uc" \
//     -L "$BUILD_DIR/wifi-scripts-1.0/src/usr/share/ucode/*.uc" \
//     tests/sae_stations_test.uc
import * as ap from 'wifi.ap';
import * as common from 'wifi.common';
import { files } from 'fs';
import { opts } from 'uci';

let checks = 0, fails = 0;
function check(cond, what) {
	checks++;
	if (!cond) { fails++; printf("FAIL: %s\n", what); }
}

// hostapd links each sae_password entry before the earlier ones, so it
// tries the last line of the file first.
const MAIN = 'mainpass1';
const SAE_FILE = '/var/run/hostapd-w5g.sae';

function bss(extra) {
	return { mode: 'ap', ifname: 'w5g', ssid: 'T', encryption: 'sae-mixed', key: MAIN, network: [ 'lan' ], ...extra };
}

function sta(key, mac, vid) {
	return { config: { key, mac: [ mac ], vid } };
}

const WILD = sta('vlanpass1', '00:00:00:00:00:00', 100);
const BOUND = sta('boundpass', '02:00:00:00:00:01', 200);

function run(config, stas, band) {
	opts.configs = {};
	common.flush_config();
	delete files[SAE_FILE];
	let data = { config: { band: band ?? '5g', htmode: 'HE80' }, ap_start_disabled: null };
	let refusal = ap.generate({}, data, config, {}, stas, {});
	let text = files[SAE_FILE];
	let r = { refusal, sae: text == null ? null : (length(text) ? split(rtrim(text, '\n'), '\n') : []) };
	for (let l in split(common.dump_config(), '\n')) {
		let m = match(l, /^(sae_track_password|wpa_passphrase|sae_password_file)=(.*)$/);
		if (m)
			r[m[1]] = m[2];
	}
	return r;
}

let r;
r = run(bss({}), {});
check(!r.refusal, "no station: no refusal");
check(length(r.sae) == 0, "no station: empty .sae file");
check(r.sae_track_password == null, "no station: no sae_track_password");
check(r.wpa_passphrase == MAIN, "no station: wpa_passphrase");

r = run(bss({}), { s1: WILD });
check(join(',', r.sae) == `vlanpass1|mac=ff:ff:ff:ff:ff:ff|vlanid=100,${MAIN}|mac=ff:ff:ff:ff:ff:ff`, "wildcard station: the station, then the main passphrase last");
check(r.sae_track_password == '256', "wildcard station: sae_track_password 256");
check(r.wpa_passphrase == MAIN, "wildcard station: wpa_passphrase stays");

r = run(bss({}), { s1: sta('vlanpass1', 'ff:ff:ff:ff:ff:ff', 100) });
check(r.sae?.[-1] == `${MAIN}|mac=ff:ff:ff:ff:ff:ff` && r.sae_track_password == '256', "broadcast MAC station: counts as wildcard");

r = run(bss({}), { s1: BOUND });
check(join(',', r.sae) == 'boundpass|mac=02:00:00:00:00:01|vlanid=200', "MAC-bound station only: no main entry");
check(r.sae_track_password == null, "MAC-bound station only: no sae_track_password");

r = run(bss({}), { s1: BOUND, s2: WILD });
check(length(r.sae) == 3 && r.sae[-1] == `${MAIN}|mac=ff:ff:ff:ff:ff:ff`, "MAC-bound and wildcard: main entry last");
check(index(r.sae, 'boundpass|mac=02:00:00:00:00:01|vlanid=200') >= 0, "MAC-bound and wildcard: MAC-bound entry kept");
check(r.sae_track_password == '256', "MAC-bound and wildcard: sae_track_password 256");

r = run(bss({ sae_track_password: 5 }), { s1: WILD });
check(r.sae_track_password == '5', "uci sae_track_password wins");

r = run(bss({ encryption: 'sae' }), { s1: WILD });
check(r.sae?.[-1] == `${MAIN}|mac=ff:ff:ff:ff:ff:ff` && r.sae_track_password == '256', "sae: main entry last");

r = run(bss({ encryption: 'sae-mixed' }), { s1: WILD }, '6g');
check(r.sae?.[-1] == `${MAIN}|mac=ff:ff:ff:ff:ff:ff` && r.sae_track_password == '256', "sae-mixed on 6 GHz: main entry last");

r = run(bss({ encryption: 'psk2', key: MAIN }), { s1: WILD });
check(r.sae == null && r.sae_track_password == null, "psk2: no .sae file, no sae_track_password");

const HEX = 'ab0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd';
r = run(bss({ key: HEX }), { s1: WILD });
check(join(',', r.sae) == 'vlanpass1|mac=ff:ff:ff:ff:ff:ff|vlanid=100', "hex PSK: no main entry");
check(r.sae_track_password == null, "hex PSK: no sae_track_password");

r = run(bss({ sae_track_password: 0 }), { s1: WILD });
check(join(',', r.sae) == 'vlanpass1|mac=ff:ff:ff:ff:ff:ff|vlanid=100', "uci sae_track_password 0: no main entry");

r = run(bss({}), { s1: sta(MAIN, '00:00:00:00:00:00', 100) });
check(join(',', r.sae) == `${MAIN}|mac=ff:ff:ff:ff:ff:ff|vlanid=100`, "station with the main key: no second main entry");
check(r.sae_track_password == null, "station with the main key: no sae_track_password");

r = run(bss({}), { s1: sta('vlanpass1', 'FF:FF:FF:FF:FF:FF', 100) });
check(r.sae?.[-1] == `${MAIN}|mac=ff:ff:ff:ff:ff:ff` && r.sae_track_password == '256', "upper-case broadcast MAC: counts as wildcard");

r = run(bss({ key: HEX }), { s1: WILD, s2: sta('vlanpass2', '00:00:00:00:00:00', 200) });
check(length(r.sae) == 2 && index(r.sae, `${MAIN}|mac=ff:ff:ff:ff:ff:ff`) < 0, "hex PSK, two wildcard stations: no main entry");
check(r.sae_track_password == '256', "hex PSK, two wildcard stations: sae_track_password 256");

r = run(bss({ ppsk: 1, auth_server_addr: [ '192.0.2.1' ], auth_server_shared_secret: 's' }), { s1: WILD });
check(join(',', r.sae) == 'vlanpass1|mac=ff:ff:ff:ff:ff:ff|vlanid=100', "PPSK: no main entry");
check(r.sae_track_password == null, "PPSK, one wildcard station: no sae_track_password");

printf("%s: %d checks, %d failed\n", fails ? 'FAIL' : 'ok', checks, fails);
exit(fails ? 1 : 0);
