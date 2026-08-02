"""Core CFFI bindings to libspiral.so"""

import ctypes
import os
import platform

# Find libspiral.so
_LIB_PATH = os.environ.get("SPIRAL_LIB", None)
if not _LIB_PATH:
    # Search standard locations
    for path in [
        "/usr/local/lib/libspiral.so",
        "/usr/local/lib/spiral/libspiral.so",
        os.path.join(os.path.dirname(__file__), "../../../lib/libspiral.so"),
    ]:
        if os.path.exists(path):
            _LIB_PATH = path
            break

if not _LIB_PATH:
    raise ImportError(
        "libspiral.so not found. Install with: make install\n"
        "Or set SPIRAL_LIB=/path/to/libspiral.so"
    )

_lib = ctypes.CDLL(_LIB_PATH)

# ── Version ──────────────────────────────────────────────────
_lib.spiral_version.restype = ctypes.c_char_p

def version() -> str:
    return _lib.spiral_version().decode()

__version__ = version()

# ── Phi Identities ───────────────────────────────────────────
_lib.spiral_verify_phi_identities.restype = ctypes.c_int

def phi_identities() -> bool:
    return _lib.spiral_verify_phi_identities() == 1

# ── Obfuscate ────────────────────────────────────────────────
_lib.spiral_obfuscate.argtypes = [
    ctypes.c_char_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_char_p
]
_lib.spiral_obfuscate.restype = ctypes.c_int

def obfuscate(source: str, ring_dim: int = 0, gates: int = 0, output: str = "output.obf") -> bool:
    """Obfuscate a program. Returns True on success."""
    return _lib.spiral_obfuscate(
        source.encode(), ring_dim, gates, output.encode()
    ) == 0

# ── Execute ──────────────────────────────────────────────────
_lib.spiral_execute.argtypes = [
    ctypes.c_char_p,
    ctypes.POINTER(ctypes.c_double), ctypes.c_int,
    ctypes.POINTER(ctypes.c_double)
]
_lib.spiral_execute.restype = ctypes.c_int

def execute(obf_path: str, inputs: list = None) -> list:
    """Execute an obfuscated program. Returns output values."""
    if inputs is None:
        inputs = [0.5, 0.3]
    
    n = len(inputs)
    in_arr = (ctypes.c_double * n)(*inputs)
    out_arr = (ctypes.c_double * 256)()
    
    n_out = _lib.spiral_execute(obf_path.encode(), in_arr, n, out_arr)
    return [out_arr[i] for i in range(n_out)]

# ── Verify ───────────────────────────────────────────────────
_lib.spiral_verify.argtypes = [
    ctypes.c_char_p, ctypes.c_char_p, ctypes.c_uint32,
    ctypes.POINTER(ctypes.c_double)
]
_lib.spiral_verify.restype = ctypes.c_int

def verify(prog_a: str, prog_b: str, ring_dim: int = 0) -> float:
    """Verify two programs are indistinguishable. Returns KS statistic."""
    ks = ctypes.c_double(1.0)
    _lib.spiral_verify(prog_a.encode(), prog_b.encode(), ring_dim, ctypes.byref(ks))
    return ks.value

# ── KEM ──────────────────────────────────────────────────────
_lib.spiral_kem_keygen.argtypes = [
    ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8)
]
_lib.spiral_kem_keygen.restype = ctypes.c_int

def kem_keygen() -> tuple:
    """Generate KEM keypair. Returns (pk, sk) as bytes."""
    pk = (ctypes.c_uint8 * 64)()
    sk = (ctypes.c_uint8 * 64)()
    _lib.spiral_kem_keygen(pk, sk)
    return bytes(pk), bytes(sk)

_lib.spiral_kem_encaps.argtypes = [
    ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8),
    ctypes.POINTER(ctypes.c_uint8)
]
_lib.spiral_kem_encaps.restype = ctypes.c_int

def kem_encaps(pk: bytes) -> tuple:
    """Encapsulate. Returns (ciphertext, shared_secret)."""
    ct = (ctypes.c_uint8 * 64)()
    ss = (ctypes.c_uint8 * 32)()
    pk_arr = (ctypes.c_uint8 * 64)(*pk)
    _lib.spiral_kem_encaps(ct, ss, pk_arr)
    return bytes(ct), bytes(ss)

_lib.spiral_kem_decaps.argtypes = [
    ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8),
    ctypes.POINTER(ctypes.c_uint8)
]
_lib.spiral_kem_decaps.restype = ctypes.c_int

def kem_decaps(ct: bytes, sk: bytes) -> bytes:
    """Decapsulate. Returns shared_secret."""
    ss = (ctypes.c_uint8 * 32)()
    ct_arr = (ctypes.c_uint8 * 64)(*ct)
    sk_arr = (ctypes.c_uint8 * 64)(*sk)
    _lib.spiral_kem_decaps(ss, ct_arr, sk_arr)
    return bytes(ss)
