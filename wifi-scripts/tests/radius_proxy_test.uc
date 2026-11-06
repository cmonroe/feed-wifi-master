// Host test of iface_radius_proxy() in ap.uc, through generate(), with the
// stub modules in tests/stubs. The search path must be absolute, because ucode
// resolves a relative one from the directory of each importing file. From the
// wifi-scripts package directory:
//   ucode -L "$PWD/tests/stubs/*.uc" -L "$PWD/files-ucode/usr/share/ucode/*.uc" \
//     tests/radius_proxy_test.uc
import * as ap from 'wifi.ap';
import * as common from 'wifi.common';
import { opts } from 'uci';

let checks = 0, fails = 0;
function check(cond, what) {
	checks++;
	if (!cond) { fails++; printf("FAIL: %s\n", what); }
}

const PROXY = { '.type': 'instance', address: '192.168.1.1', auth_port: '61812', acct_port: '61813' };

function bss(extra) {
	return { mode: 'ap', ifname: 'w5g-iot', ssid: 'T', encryption: 'wpa2+ccmp', network: [ 'lan' ],
	         auth_server_addr: [ '172.20.14.16' ], auth_server_port: 1812, auth_server_shared_secret: 'op',
	         ...extra };
}

function lines(config, role, configs) {
	opts.configs = { intellifi: { global: { role, proxy_secret: 'px' } }, ...configs };
	common.flush_config();
	let data = { config: { band: '5g', htmode: 'HE80' }, ap_start_disabled: null };
	let refusal = ap.generate({}, data, config, {}, {}, {});
	if (refusal)
		return { refusal };
	let r = {};
	for (let l in split(common.dump_config(), '\n')) {
		let m = match(l, /^(auth_server_addr|auth_server_port|auth_server_shared_secret|acct_server_addr|acct_server_port|acct_server_shared_secret)=(.*)$/);
		if (m)
			r[m[1]] ??= m[2];
	}
	return r;
}

let r;
r = lines(bss({}), 'hub', { radsecproxy: { lan: PROXY } });
check(r.auth_server_addr == '192.168.1.1' && r.auth_server_port == '61812' && r.auth_server_shared_secret == 'px', "hub, instance: auth to the proxy");
check(r.acct_server_addr == null, "hub, instance, no acct_server: no accounting line");

r = lines(bss({ acct_server_addr: [ '172.20.14.16' ], acct_server_port: 1813, acct_server_shared_secret: 'op' }), 'hub', { radsecproxy: { lan: PROXY } });
check(r.acct_server_addr == '192.168.1.1' && r.acct_server_port == '61813' && r.acct_server_shared_secret == 'px', "hub, instance, acct_server: accounting to the proxy");

r = lines(bss({}), 'satellite', { radsecproxy: { lan: PROXY } });
check(r.auth_server_addr == '172.20.14.16' && r.auth_server_shared_secret == 'op', "satellite with a stale instance: wireless values");

r = lines(bss({}), 'hub', { radsecproxy: { guest: PROXY } });
check(r.auth_server_addr == '172.20.14.16', "hub, instance of another network only: wireless values");

r = lines(bss({}), 'hub', {});
check(r.auth_server_addr == '172.20.14.16', "hub, no radsecproxy config: wireless values");

r = lines(bss({ network: [ 'lan', 'guest' ] }), 'hub', { radsecproxy: { lan: PROXY, guest: PROXY } });
check(r.auth_server_addr == '172.20.14.16', "hub, BSS in two networks: wireless values");
check(index(opts.asked, 'radsecproxy.lan guest') >= 0, "two networks: lookup by the joined name");

r = lines(bss({ network: null }), 'hub', { radsecproxy: { lan: PROXY } });
check(r.auth_server_addr == '172.20.14.16', "hub, BSS without a network: wireless values");

r = lines(bss({}), 'hub', { radsecproxy: { lan: { ...PROXY, address: null } } });
check(r.auth_server_addr == '172.20.14.16', "hub, instance without address: wireless values");

r = lines(bss({}), 'hub', { radsecproxy: { lan: { ...PROXY, '.type': 'server' } } });
check(r.auth_server_addr == '172.20.14.16', "hub, section of another type: wireless values");

opts.configs = {};
r = lines(bss({}), 'hub', { radsecproxy: { lan: PROXY }, intellifi: { global: { role: 'hub' } } });
check(r.auth_server_addr == '172.20.14.16', "hub, no proxy_secret: wireless values");

r = lines(bss({ encryption: 'psk2+ccmp', key: '12345678', auth_server_addr: null, auth_server_port: null, auth_server_shared_secret: null }), 'hub', { radsecproxy: { lan: PROXY } });
check(r.auth_server_addr == null, "hub, PSK BSS without RADIUS: no RADIUS line");

printf("%s: %d checks, %d failed\n", fails ? 'FAIL' : 'ok', checks, fails);
exit(fails ? 1 : 0);
