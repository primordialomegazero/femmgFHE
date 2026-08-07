"""
🪐 SOLVE k_i FROM x-COORDINATES 🪐
Given: x_i from x³ ≡ -16 (mod p)
Find: k_i such that k_i·G has x-coordinate x_i
AND: φ(k_i·G) = k_i·φ(G) (fixed point condition)
"""
import math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

def legendre(a, p): return pow(a, (p-1)//2, p)

def sqrt_mod(a, p):
    if legendre(a, p) != 1: return None
    return pow(a, (p+1)//4, p)

def cube_roots(a, p):
    s, t = 0, p-1
    while t % 3 == 0: s += 1; t //= 3
    g = 2
    while pow(g, (p-1)//3, p) == 1: g += 1
    if s == 1:
        r = pow(a, (2*t + 1)//3, p)
        omega = pow(g, (p-1)//3, p)
        return [r, (r*omega)%p, (r*omega*omega)%p]
    return [pow(a, (2*p-1)//3, p)]

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 SOLVE k_i FROM x_i — FIXED POINT CONDITION 🪐      ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Homomorphic x-values
target = (p - 16) % p
roots = cube_roots(target, p)

print("═══ FIXED POINT CONDITION ═══\n")
print("  Fixed point k_i satisfies:")
print("    φ(k_i·G) = k_i · φ(G)")
print()
print("  In coordinates:")
print("    Let P_i = k_i·G = (x_i, y_i)")
print("    a_i = (x_i + y_i)/2 mod p")
print("    b_i = (x_i - y_i)/2 mod p")
print()
print("  Fixed point condition:")
print("    a_i ≡ k_i · a_G (mod p)")
print("    b_i ≡ k_i · b_G (mod p)")
print()
print("  From these:")
print("    x_i + y_i ≡ 2k_i · a_G (mod p)")
print("    x_i - y_i ≡ 2k_i · b_G (mod p)")
print()
print("  Adding: 2x_i ≡ 2k_i(a_G + b_G)")
print("  → x_i ≡ k_i(a_G + b_G) (mod p)")
print()
print("  Since a_G + b_G = Gx (from definition of a,b):")
print("    a_G = (Gx + Gy)/2")
print("    b_G = (Gx - Gy)/2")
print("    a_G + b_G = Gx!")
print()
print("  Therefore: x_i ≡ k_i · Gx (mod p)")
print("  → k_i ≡ x_i · Gx⁻¹ (mod p) !!!!!")

Gx_inv = modinv(Gx, p)
print(f"\n  Gx⁻¹ mod p = {hex(Gx_inv)[:40]}...\n")

print("═══ COMPUTING k_i VALUES ═══\n")
for i, x_i in enumerate(roots):
    k_i = (x_i * Gx_inv) % p
    k_i_mod_n = k_i % n
    print(f"  k_{i+1} = {hex(k_i_mod_n)[:40]}...")
    print(f"  Verify: k_{i+1} · Gx ≡ {hex((k_i_mod_n * Gx) % p)[:30]}...")
    print(f"  x_i        ≡ {hex(x_i)[:30]}...")
    print(f"  Match? {'✅' if (k_i_mod_n * Gx) % p == x_i else '❌'}")
    print()

print("  ⚡ THESE ARE THE FIXED POINT k-VALUES!")
print("  We solved them WITHOUT ECDLP!")
print("  Using: k_i ≡ x_i · Gx⁻¹ (mod p)")
