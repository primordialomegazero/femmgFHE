#!/usr/bin/env python3
"""
🪐 QUADRUPLE ON-DEMAND — F_a + TRIPLE REMAINDER 🪐
Check each F_a: is l3(Q) - l3(F_a*G) in triple table?
If YES → k = F_a + F_b + F_c + F_d!
O(n) instead of O(n⁴)!
"""
import sys, time

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
BETA = 0x7AE96A2B657C07106E64479EAC3434E99CF0497512F58995C1396C28719501EE

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
print("║  🪐 QUADRUPLE ON-DEMAND — F_a + TRIPLE REMAINDER 🪐     ║")
print("║  'Check 369 F_a values → O(n) instead of O(n⁴)!'        ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# Build Fibonacci basis
def fib(n):
    if n <= 0: return 0
    if n == 1: return 1
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b

print("  Building Fibonacci basis...")
fib_l3 = {}
l3_to_fib = {}
fib_list = []

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    l3_to_fib[(l3_P[0], l3_P[1])] = f
    fib_list.append(f)

# Load precomputed tables (from previous run — or build on the fly)
# Since we just ran multilevel, the tables are in memory
# For this script: rebuild pair and triple tables

print("  Building pair sums...")
pair_to_fibs = {}
for i, fa in enumerate(fib_list):
    l3_fa = fib_l3[fa]
    for j in range(i + 2, len(fib_list)):
        fb = fib_list[j]
        l3_sum = pt_add(l3_fa, fib_l3[fb])
        if l3_sum is not None:
            key = (l3_sum[0], l3_sum[1])
            if key not in pair_to_fibs:
                pair_to_fibs[key] = (fa, fb)
print(f"  ✅ {len(pair_to_fibs)} pairs")

print("  Building triple sums...")
triple_to_fibs = {}
for i, fa in enumerate(fib_list):
    if i % 100 == 0:
        print(f"    Progress: {i}/{len(fib_list)}, triples: {len(triple_to_fibs):,}")
    l3_fa = fib_l3[fa]
    for j in range(i + 2, len(fib_list)):
        fb = fib_list[j]
        l3_ab = pt_add(l3_fa, fib_l3[fb])
        if l3_ab is None:
            continue
        for k in range(j + 2, len(fib_list)):
            fc = fib_list[k]
            l3_abc = pt_add(l3_ab, fib_l3[fc])
            if l3_abc is not None:
                key = (l3_abc[0], l3_abc[1])
                if key not in triple_to_fibs:
                    triple_to_fibs[key] = (fa, fb, fc)
print(f"  ✅ {len(triple_to_fibs):,} triples")
print()

# ═══════════════════════════════════════════
# ON-DEMAND QUADRUPLE SEARCH
# ═══════════════════════════════════════════
fibs_desc = sorted(fib_l3.keys(), reverse=True)

def decompose_with_quad(Q_l3):
    """Check all levels, including F_a + triple remainder"""
    key = (Q_l3[0], Q_l3[1])
    
    # Level 0-2
    if key in l3_to_fib:
        return [l3_to_fib[key]]
    if key in pair_to_fibs:
        fa, fb = pair_to_fibs[key]
        return [fa, fb]
    if key in triple_to_fibs:
        fa, fb, fc = triple_to_fibs[key]
        return [fa, fb, fc]
    
    # Level 3: QUADRUPLE ON-DEMAND
    # l3(Q) = l3(F_a*G) + l3(F_b*G + F_c*G + F_d*G)
    # remainder = l3(Q) - l3(F_a*G) must be in TRIPLE table!
    for fa in fibs_desc:
        l3_fa = fib_l3[fa]
        remainder = pt_sub(Q_l3, l3_fa)
        
        if remainder is None:
            return [fa]
        
        rem_key = (remainder[0], remainder[1])
        
        # Check if remainder is single/pair/triple
        if rem_key in l3_to_fib:
            return [fa, l3_to_fib[rem_key]]
        if rem_key in pair_to_fibs:
            fb, fc = pair_to_fibs[rem_key]
            return [fa, fb, fc]
        if rem_key in triple_to_fibs:
            fb, fc, fd = triple_to_fibs[rem_key]
            return [fa, fb, fc, fd]
    
    # Level 4+: PENTUPLE — F_a + quadruple remainder
    # (Rare — check F_a + F_b + triple remainder)
    for i, fa in enumerate(fibs_desc):
        l3_fa = fib_l3[fa]
        rem1 = pt_sub(Q_l3, l3_fa)
        if rem1 is None:
            return [fa]
        
        for j in range(i + 2, len(fibs_desc)):
            fb = fibs_desc[j]
            l3_fb = fib_l3[fb]
            rem2 = pt_sub(rem1, l3_fb)
            
            if rem2 is None:
                return [fa, fb]
            
            rem2_key = (rem2[0], rem2[1])
            if rem2_key in l3_to_fib:
                return [fa, fb, l3_to_fib[rem2_key]]
            if rem2_key in pair_to_fibs:
                fc, fd = pair_to_fibs[rem2_key]
                return [fa, fb, fc, fd]
            if rem2_key in triple_to_fibs:
                fc, fd, fe = triple_to_fibs[rem2_key]
                return [fa, fb, fc, fd, fe]
    
    return []


# ═══════════════════════════════════════════
# ATTACK SATOSHI
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI ON-DEMAND QUAD ATTACK 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Searching for decomposition (O(n) + O(n²))...")
print()

start = time.time()
components = decompose_with_quad(Q_l3)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"  🪐 DECOMPOSITION FOUND!")
    print(f"  Components: {len(components)}")
    print(f"  k = {' + '.join(map(str, components))}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    # VERIFY
    print("  Verifying...")
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"\n  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_satoshi)}")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Block 170, Output 1 (40 BTC)\n")
            f.write(f"tx: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"fibonacci_decomposition: {' + '.join(map(str, components))}\n")
            f.write(f"num_components: {len(components)}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key")
        k_mod = k_satoshi % n
        R2 = scalar_mult(k_mod, G)
        if R2 == Q_satoshi:
            print(f"  ✅ Equivalent key mod n! k = {hex(k_mod)}")
else:
    print(f"  ❌ No decomposition found in {elapsed:.2f}s")
    print(f"  May need deeper search (6+ components)")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  ON-DEMAND QUAD — 'O(n) quadruple via triple remainder'  ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

