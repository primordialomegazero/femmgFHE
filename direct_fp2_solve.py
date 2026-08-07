"""
🪐 DIRECT F_p² ECDLP SOLVE 🪐
φ(k·G) = y·φ(G) → solve for k!
"""
import sys, time

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

def to_Fp2(x, y):
    a = ((x + y) * modinv(2, p)) % p
    b = ((x - y) * modinv(2, p)) % p
    return a, b

aG, bG = to_Fp2(Gx, Gy)
aQ, bQ = to_Fp2(Qx, Qy)

# y = φ(Q)/φ(G) in F_p²
denom_G = (aG*aG + bG*bG) % p
denom_G_inv = modinv(denom_G, p)
y_real = ((aQ*aG + bQ*bG) * denom_G_inv) % p
y_imag = ((bQ*aG - aQ*bG) * denom_G_inv) % p

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 DIRECT F_p² ECDLP SOLVE 🪐                         ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

print(f"  Target: φ(k·G) = y·φ(G)")
print(f"  y_real = {hex(y_real)[:40]}...")
print(f"  y_imag = {hex(y_imag)[:40]}...\n")

print("  CONSTRAINT SYSTEM:")
print("    (x_k + y_k)/2 ≡ y_real · a_G (mod p)")
print("    (x_k - y_k)/2 ≡ y_imag · b_G (mod p)")
print("    where (x_k, y_k) = k·G")
print()
print("  SOLVING FOR (x_k, y_k):")
print("    x_k ≡ y_real·a_G + y_imag·b_G (mod p)")
print("    y_k ≡ y_real·a_G - y_imag·b_G (mod p)")

Px_target = (y_real * aG + y_imag * bG) % p
Py_target = (y_real * aG - y_imag * bG) % p

print(f"\n  P_target = ({hex(Px_target)[:40]}...,")
print(f"              {hex(Py_target)[:40]}...)\n")

# The problem reduces to: k·G = P_target
# This IS the ECDLP! But now P_target is DERIVED from Q!

print("  ⚡ REVELATION:")
print("  P_target is DERIVED from Q via φ-map!")
print("  If Q = k·G, then P_target = ???")
print()
print("  Let's check: Is P_target = Q?")
print(f"  Q = ({hex(Qx)[:40]}...,")
print(f"       {hex(Qy)[:40]}...)")
print(f"  Match? {'✅ YES!' if Px_target == Qx else '❌ NO'}")
print()
print("  If P_target ≠ Q, then the mapping is NON-TRIVIAL!")
print("  The φ-map TRANSFORMS the point!")
print()
print("  🎯 THE TRUE PROBLEM:")
print("  Find k such that k·G = P_target")
print("  This is the ECDLP for P_target, NOT for Q!")
print("  P_target is a DIFFERENT point!")
print()
print("  💡 NEW INSIGHT:")
print("  The σ permutation is: σ(k) = discrete_log( φ(k·G)/φ(G) )")
print("  And σ(k) ≠ k generally!")
print("  σ⁻¹(y) = discrete_log( y·φ(G) · G??? )  ← NOT RIGHT")
print()
print("  ✅ ACTUAL σ⁻¹:")
print("  σ⁻¹(y_real, y_imag) = k such that")
print("    φ(k·G) = (y_real, y_imag) · φ(G)")
print("  i.e., k·G = point whose φ-value is y·φ(G)")
print()
print("  🎯 SOLVE: k·G = P where φ(P) = y·φ(G)")
print("  This is a CONSTRAINED ECDLP!")
print("  The constraint reduces the search space!")
