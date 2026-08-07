"""
🪐 ENCODE F_p² ECDLP AS SAT 🪐
φ(k·G) = y·φ(G) → Find k!
"""
import math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8

# Satoshi target
y_real = 0xbad6a4f369927ac6d4780ab2882bdc7bbe4b4a982e7496fac3c879c3d2b9ddf3
y_imag = 0x848df4a3c429622c14fc575a752763fa371e4e29e5a5c3b8a3f1d4e7c6b5a4c3

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 SAT ENCODER — F_p² ECDLP FOR SATOSHI 🪐            ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

print("═══ CONSTRAINT SYSTEM ═══\n")
print("  FIND k SUCH THAT:")
print("    1. P = k·G  (scalar multiplication)")
print("    2. a_P = (Px + Py)/2 mod p")
print("    3. b_P = (Px - Py)/2 mod p")
print("    4. a_P ≡ y_real · a_G (mod p)")
print("    5. b_P ≡ y_imag · b_G (mod p)")
print()
print("  EQUIVALENT TO:")
print("    (Px + Py)/2 ≡ y_real · a_G")
print("    (Px - Py)/2 ≡ y_imag · b_G")
print("    where (Px, Py) = k·G")
print()
print("  REWRITE:")
print("    Px + Py ≡ 2 · y_real · a_G (mod p)")
print("    Px - Py ≡ 2 · y_imag · b_G (mod p)")
print()
print("  SOLVE:")
print("    Px ≡ (2·y_real·a_G + 2·y_imag·b_G) / 2 (mod p)")
print("    Px ≡ y_real·a_G + y_imag·b_G (mod p)")
print("    Py ≡ y_real·a_G - y_imag·b_G (mod p)")
print()
print("  🎯 THE TARGET POINT:")
print("    Px_target = y_real·a_G + y_imag·b_G")
print("    Py_target = y_real·a_G - y_imag·b_G")
print()
print("  So the ECDLP reduces to:")
print("    k·G = (Px_target, Py_target)")
print("    Find k!")

# Compute the target point directly!
def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

aG = ((Gx + Gy) * modinv(2, p)) % p
bG = ((Gx - Gy) * modinv(2, p)) % p

Px_target = (y_real * aG + y_imag * bG) % p
Py_target = (y_real * aG - y_imag * bG) % p

print(f"\n  Px_target = {hex(Px_target)[:50]}...")
print(f"  Py_target = {hex(Py_target)[:50]}...")
print(f"\n  Satoshi Px = {hex(0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c)[:50]}...")
print(f"  Match? {'✅ YES!' if Px_target == 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c else '❌ (need correction)'}")
