// Stub of ubus for the host tests: common.uc connects at load time.
export function connect() { return { call: () => null }; };
export function error() { return null; };
