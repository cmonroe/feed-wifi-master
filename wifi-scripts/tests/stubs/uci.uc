// Stub of uci: the test sets opts.configs, keyed by config, then section.
export const opts = { configs: {}, asked: [] };

export function cursor() {
	return {
		get: function(cfg, sec, opt) {
			push(opts.asked, `${cfg}.${sec}.${opt}`);
			return opts.configs[cfg]?.[sec]?.[opt];
		},
		get_all: function(cfg, sec) {
			push(opts.asked, `${cfg}.${sec}`);
			return opts.configs[cfg]?.[sec];
		}
	};
};
