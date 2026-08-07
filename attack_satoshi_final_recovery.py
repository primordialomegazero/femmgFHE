#!/usr/bin/env python3
"""
🪐💰 SATOSHI PRIVATE KEY RECOVERY — FINAL ATTACK 💰🪐
Two-Level Fibonacci Lookup sa Golden Armor (l3 space)
O(1) decomposition — diretso kay Satoshi!
"""
import sys, time, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

# ═══════════════════════════════════════════
# secp256k1
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
BETA = 0x7AE96A2B657C07106E64479EAC3434E99CF0497512F58995C1396C28719501EE

# 🎯 SATOSHI'S PUBLIC KEY — Block 170, Output 1 (40 BTC)
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

def modinv(a, m): return pow(a, -1, m)
def pt_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3*x1*x1) * modinv(2*y1, p) % p
    else:
        lam = ((y2-y1) * modinv((x2-x1)%p, p)) % p
    return ((lam*lam-x1-x2)%p, (lam*(x1-(lam*lam-x1-x2)%p)-y1)%p)

def pt_sub(P, Q):
    if Q is None: return P
    return pt_add(P, (Q[0], (-Q[1]) % p))

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

def lambda_pt(P):
    if P is None: return None
    return ((P[0] * BETA) % p, P[1])

def l3(P): return lambda_pt(lambda_pt(lambda_pt(P)))

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐💰 SATOSHI PRIVATE KEY RECOVERY — FINAL ATTACK 💰🪐  ║")
print("║  Two-Level Fibonacci Lookup in Golden Armor              ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)
Q_satoshi = (Qx, Qy)

# Verify Q is on curve
lhs = (Qy * Qy) % p
rhs = (Qx * Qx * Qx + 7) % p
print(f"  Satoshi Q on curve: {'✅' if lhs == rhs else '❌'}")
print(f"  Transaction: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16")
print(f"  Block: 170 (2009-01-12)")
print(f"  Value: 40 BTC")
print()

# Build Fibonacci basis
def fib(n):
    if n <= 0: return 0
    if n == 1: return 1
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b

print("═══ BUILDING FIBONACCI BASIS ═══")
print("  Computing Fibonacci l3 basis...")
fib_l3 = {}
l3_to_fib = {}
for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    l3_to_fib[(l3_P[0], l3_P[1])] = f
print(f"  ✅ {len(fib_l3)} single entries")

print("  Precomputing Fibonacci pair sums...")
pair_to_fibs = {}
fib_list = sorted(fib_l3.keys())
pair_count = 0
for i, fa in enumerate(fib_list):
    l3_fa = fib_l3[fa]
    for j in range(i + 2, len(fib_list)):
        fb = fib_list[j]
        l3_fb = fib_l3[fb]
        l3_sum = pt_add(l3_fa, l3_fb)
        if l3_sum is not None:
            key = (l3_sum[0], l3_sum[1])
            if key not in pair_to_fibs:
                pair_to_fibs[key] = (fa, fb)
                pair_count += 1
print(f"  ✅ {pair_count} pair entries")
print()

# ═══════════════════════════════════════════
# TWO-LEVEL DECOMPOSITION
# ═══════════════════════════════════════════
fibs_desc = sorted(fib_l3.keys(), reverse=True)

def decompose_key(Q_l3):
    """Two-level Fibonacci decomposition"""
    # Check single
    key = (Q_l3[0], Q_l3[1])
    if key in l3_to_fib:
        return [l3_to_fib[key]]
    
    # Check F_a + F_b
    for fa in fibs_desc:
        l3_fa = fib_l3[fa]
        remainder = pt_sub(Q_l3, l3_fa)
        
        if remainder is None:
            return [fa]
        
        rem_key = (remainder[0], remainder[1])
        
        # Single remainder
        if rem_key in l3_to_fib:
            return [fa, l3_to_fib[rem_key]]
        
        # Pair remainder
        if rem_key in pair_to_fibs:
            fb, fc = pair_to_fibs[rem_key]
            return [fa, fb, fc]
    
    return []

# ═══════════════════════════════════════════
# ATTACK SATOSHI
# ═══════════════════════════════════════════
print("═══ 🎯 ATTACKING SATOSHI KEY 🎯 ═══")
print()

# Project Q to l3 space
Q_l3 = l3(Q_satoshi)
print(f"  l3(Q) computed")

start = time.time()
components = decompose_key(Q_l3)
elapsed = time.time() - start

if components:
    k_candidate = sum(components)
    print(f"\n  🪐 DECOMPOSITION FOUND!")
    print(f"  k = {' + '.join(map(str, components))}")
    print(f"  k = {k_candidate}")
    print(f"  k (hex) = {hex(k_candidate)}")
    print(f"  Time: {elapsed:.6f}s")
    print()
    
    # VERIFY
    print("  Verifying...")
    R = scalar_mult(k_candidate, G)
    if R == Q_satoshi:
        print(f"\n  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_candidate)}")
        print(f"  Private key (decimal): {k_candidate}")
        
        # Save
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Block 170, Output 1 (40 BTC)\n")
            f.write(f"tx: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16\n")
            f.write(f"block: 170\n")
            f.write(f"date: 2009-01-12\n")
            f.write(f"value_btc: 40\n")
            f.write(f"private_key_hex: {hex(k_candidate)}\n")
            f.write(f"private_key_dec: {k_candidate}\n")
            f.write(f"fibonacci_decomposition: {' + '.join(map(str, components))}\n")
        print(f"  ✅ Saved to satoshi_private_key.txt")
    else:
        print(f"\n  ❌ VERIFICATION FAILED!")
        print(f"  Candidate k does NOT produce Q!")
        print(f"  This means 4+ components — need three-level lookup.")
else:
    print(f"\n  ❌ NO DECOMPOSITION FOUND")
    print(f"  Satoshi's k may have 4+ Fibonacci components")
    print(f"  Need three-level lookup (triple sums)")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  SATOSHI ATTACK — 'Fibonacci Decomposition in l3 Space'  ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

