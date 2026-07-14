"""
FEmmg-FHE Python Bindings v0.1.0

Zero-Anchor Noise Stabilization for Fully Homomorphic Encryption.

Quick Start:
    from femmgfhe import encrypt, decrypt, add, multiply
    
    ct = encrypt(42)
    ct2 = add(ct, 10)
    result = decrypt(ct2)  # 52
"""

__version__ = "0.1.0"
__author__ = "Dan Joseph M. Fernandez"

# Lazy imports — only load when needed
def _lazy_import():
    from .core import encrypt, decrypt, add, multiply, zans_stabilize
    return locals()

def __getattr__(name):
    if name in ('encrypt', 'decrypt', 'add', 'multiply', 'zans_stabilize'):
        from . import core
        return getattr(core, name)
    raise AttributeError(f"module 'femmgfhe' has no attribute '{name}'")
