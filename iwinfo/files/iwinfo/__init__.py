from ctypes import *
from ctypes.util import find_library

def get_library_abi(lib):
    try:
        abi_sym = c_int.in_dll(lib, "libiwinfo_ABI")
        return abi_sym.value
    except ValueError:
        return None

libiwinfo = CDLL(find_library("iwinfo"))
_abi = get_library_abi(libiwinfo)
_current_abi = 20230701
assert _abi == _current_abi, f"Wrong ABI found: {_abi}, expected: {_current_abi}"
    
class IWInfoOps(Structure):
    _fields_ = [
        ("name", c_char_p),
        ("probe", CFUNCTYPE(c_int, c_char_p)),
        ("mode", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("channel", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("center_chan1", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("center_chan2", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("frequency", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("frequency_offset", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("txpower", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("txpower_offset", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("bitrate", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("signal", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("noise", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("quality", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("quality_max", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("mbssid_support", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("hwmodelist", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("htmodelist", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("htmode", CFUNCTYPE(c_int, c_char_p, POINTER(c_int))),
        ("ssid", CFUNCTYPE(c_int, c_char_p, c_char_p)),
        ("bssid", CFUNCTYPE(c_int, c_char_p, c_char_p)),
        ("country", CFUNCTYPE(c_int, c_char_p, c_char_p)),
        ("hardware_id", CFUNCTYPE(c_int, c_char_p, c_char_p)),
        ("hardware_name", CFUNCTYPE(c_int, c_char_p, c_char_p)),
        ("encryption", CFUNCTYPE(c_int, c_char_p, c_char_p)),
        ("phyname", CFUNCTYPE(c_int, c_char_p, c_char_p)),
        ("assoclist", CFUNCTYPE(c_int, c_char_p, c_char_p, POINTER(c_int))),
        ("txpwrlist", CFUNCTYPE(c_int, c_char_p, c_char_p, POINTER(c_int))),
        ("scanlist", CFUNCTYPE(c_int, c_char_p, c_char_p, POINTER(c_int))),
        ("freqlist", CFUNCTYPE(c_int, c_char_p, c_char_p, POINTER(c_int))),
        ("countrylist", CFUNCTYPE(c_int, c_char_p, c_char_p, POINTER(c_int))),
        ("survey", CFUNCTYPE(c_int, c_char_p, c_char_p, POINTER(c_int))),
        ("lookup_phy", CFUNCTYPE(c_int, c_char_p, c_char_p)),
        ("phy_path", CFUNCTYPE(c_int, c_char_p, POINTER(c_char_p))),
        ("close", CFUNCTYPE(None))
    ]

# Define the function prototype
libiwinfo.iwinfo_backend_by_name.restype = POINTER(IWInfoOps)
libiwinfo.iwinfo_backend_by_name.argtypes = [c_char_p]

libiwinfo.iwinfo_htmode_name.restype = c_char_p
libiwinfo.iwinfo_htmode_name.argtypes = [c_int]

def get_backend(name) -> IWInfoOps:
    name_bytes = name.encode('utf-8')
    iwinfo_ops_ptr = libiwinfo.iwinfo_backend_by_name(name_bytes)
    if not iwinfo_ops_ptr:
        raise NotImplementedError()
    return iwinfo_ops_ptr.contents

def get_channel(backend, interface_name):
    channel = c_int()
    result = backend.channel(interface_name.encode('utf-8'), byref(channel))
    if result == 0:
        return channel.value
    else:
        return None
    
def get_freq(backend, interface_name):
    freq = c_int()
    result = backend.frequency(interface_name.encode('utf-8'), byref(freq))
    if result == 0:
        return freq.value
    else:
        return None
    
def get_noise(backend, interface_name):
    noise = c_int()
    result = backend.noise(interface_name.encode('utf-8'), byref(noise))
    if result == 0:
        return noise.value
    else:
        return None
    
def get_htmode(backend, interface_name):
    htmode = c_int()
    result = backend.htmode(interface_name.encode('utf-8'), byref(htmode))
    if result != 0:
        return "unknown"

    htmode_name = libiwinfo.iwinfo_htmode_name(htmode.value)
    return htmode_name.decode('utf-8') if htmode_name else "unknown"
