import ctypes, os
_lib = ctypes.CDLL(os.path.join(os.path.dirname(__file__), 'libschnorr.so'))
class Schnorr:
    PUBLICKEYBYTES = 33; SECRETKEYBYTES = 32; BYTES = 65
    @staticmethod
    def keypair():
        pk = (ctypes.c_ubyte * 33)(); sk = (ctypes.c_ubyte * 32)()
        if _lib.schnorr_keypair(pk, sk) != 0: raise Exception("Keygen failed")
        return bytes(pk), bytes(sk)
    @staticmethod
    def sign(msg, sk):
        sig = (ctypes.c_ubyte * 65)(); siglen = ctypes.c_size_t(65)
        if _lib.schnorr_sign(msg, len(msg), sk, sig, ctypes.byref(siglen)) != 0: raise Exception("Sign failed")
        return bytes(sig[:siglen.value])
    @staticmethod
    def verify(msg, sig, pk):
        return _lib.schnorr_verify(sig, len(sig), msg, len(msg), pk) == 0
