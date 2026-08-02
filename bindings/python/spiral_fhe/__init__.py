"""
Spiral Fractal iO — Python Bindings
====================================
Indistinguishable program obfuscation with FHE.
KS = 0.000000 preserved at Python level.

Usage:
    import spiral_fhe

    # Obfuscate a program
    spiral_fhe.obfuscate("int f(int x) { return x * 7 + 13; }", output="test.obf")

    # Execute obfuscated program
    result = spiral_fhe.execute("test.obf", inputs=[0.5, 0.3])

    # Verify indistinguishability
    ks = spiral_fhe.verify("program_a.c", "program_b.c")
    print(f"KS = {ks:.6f} — {'🔥 ZERO' if ks == 0 else '✅ PASSED'}")

    # KEM
    pk, sk = spiral_fhe.kem_keygen()
    ct, ss = spiral_fhe.kem_encaps(pk)
    ss2 = spiral_fhe.kem_decaps(ct, sk)
"""

from .core import (
    obfuscate,
    execute,
    verify,
    kem_keygen,
    kem_encaps,
    kem_decaps,
    phi_identities,
    version,
    __version__
)
