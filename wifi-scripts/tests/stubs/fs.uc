// In-memory fs: generate() writes vlan, psk and mac files under /var/run.
export const files = {};

export function open(path, mode) {
	files[path] ??= '';
	return {
		write: function(d) { files[path] += d; return length(d); },
		read: function(n) { return length(files[path]) ? files[path] : null; },
		close: function() { return true; }
	};
};
export function readfile(path) { return files[path]; };
export function writefile(path, d) { files[path] = `${d}`; return length(d); };
export function realpath(path) { return path; };
export function error() { return null; };
