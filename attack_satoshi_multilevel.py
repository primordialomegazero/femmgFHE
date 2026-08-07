#!/usr/bin/env python3
"""
🪐 MULTI-LEVEL FIBONACCI LOOKUP — 3-LEVEL + 4-LEVEL 🪐
Single: 369 entries
Pair: 67,528 entries
Triple: ~8.2M entries (~200 MB)
Quadruple: ON-DEMAND (check as we go)
'Precompute all Fibonacci sums → O(1) decomposition!'
"""
import sys, time, math, gc

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

# 🎯 SATOSHI
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
print("║  🪐 MULTI-LEVEL FIBONACCI LOOKUP 🪐                      ║")
print("║  Single → Pair → Triple → Quadruple (on-demand)          ║")
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

print("═══ BUILDING FIBONACCI BASIS ═══")
print("  Level 0: Single entries...")
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

print(f"  ✅ {len(fib_l3)} single entries")
print()

# ═══════════════════════════════════════════
# PRECOMPUTE PAIR SUMS
# ═══════════════════════════════════════════
print("  Level 1: Pair sums (non-consecutive)...")
pair_to_fibs = {}
for i, fa in enumerate(fib_list):
    if i % 50 == 0:
        print(f"    Progress: {i}/{len(fib_list)}")
    l3_fa = fib_l3[fa]
    for j in range(i + 2, len(fib_list)):
        fb = fib_list[j]
        l3_fb = fib_l3[fb]
        l3_sum = pt_add(l3_fa, l3_fb)
        if l3_sum is not None:
            key = (l3_sum[0], l3_sum[1])
            if key not in pair_to_fibs:
                pair_to_fibs[key] = (fa, fb)

print(f"  ✅ {len(pair_to_fibs)} pair entries")
print()

# ═══════════════════════════════════════════
# PRECOMPUTE TRIPLE SUMS
# ═══════════════════════════════════════════
print("  Level 2: Triple sums (non-consecutive)...")
print("  (This may take a minute and ~200MB RAM...)")
triple_to_fibs = {}
count = 0
for i, fa in enumerate(fib_list):
    if i % 100 == 0:
        print(f"    Progress: {i}/{len(fib_list)}, triples so far: {len(triple_to_fibs):,}")
    l3_fa = fib_l3[fa]
    for j in range(i + 2, len(fib_list)):
        fb = fib_list[j]
        l3_fb = fib_l3[fb]
        l3_ab = pt_add(l3_fa, l3_fb)
        if l3_ab is None:
            continue
        for k in range(j + 2, len(fib_list)):
            fc = fib_list[k]
            l3_fc = fib_l3[fc]
            l3_abc = pt_add(l3_ab, l3_fc)
            if l3_abc is not None:
                key = (l3_abc[0], l3_abc[1])
                if key not in triple_to_fibs:
                    triple_to_fibs[key] = (fa, fb, fc)
                    count += 1

print(f"  ✅ {len(triple_to_fibs):,} triple entries")
print()

# ═══════════════════════════════════════════
# MULTI-LEVEL DECOMPOSITION
# ═══════════════════════════════════════════
fibs_desc = sorted(fib_l3.keys(), reverse=True)

def multilevel_decompose(Q_l3):
    """
    MULTI-LEVEL DECOMPOSITION:
    Level 0: Single Fibonacci?
    Level 1: Pair sum?
    Level 2: Triple sum?
    Level 3: Quadruple (search for F_a + triple remainder)
    Level 4+: Extended search
    """
    key = (Q_l3[0], Q_l3[1])
    
    # Level 0: Single
    if key in l3_to_fib:
        return [l3_to_fib[key]]
    
    # Level 1: Pair
    if key in pair_to_fibs:
        fa, fb = pair_to_fibs[key]
        return [fa, fb]
    
    # Level 2: Triple
    if key in triple_to_fibs:
        fa, fb, fc = triple_to_fibs[key]
        return [fa, fb, fc]
    
    # Level 3+: Search for F_a + known multi-sum remainder
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
    
    return []


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING MULTI-LEVEL DECOMPOSITION ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
results = []

for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = multilevel_decompose(Q_l3)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    
    print(f"  k={secret:4d}: {'✅' if match else '❌'} got {total} = {'+'.join(map(str,components))} ({elapsed:.4f}s)")
    results.append((secret, total, match))

print()

# ═══════════════════════════════════════════
# SATOSHI ATTACK
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI MULTI-LEVEL ATTACK 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Decomposing Satoshi's l3(Q)...")
start = time.time()
components = multilevel_decompose(Q_l3)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  🪐 DECOMPOSITION FOUND!")
    print(f"  Components: {len(components)}")
    print(f"  k = {' + '.join(map(str, components))}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    # VERIFY
    print("  Verifying on secp256k1...")
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"\n  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_satoshi)}")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Block 170, Output 1 (40 BTC)\n")
            f.write(f"tx: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16\n")
            f.write(f"block: 170\n")
            f.write(f"date: 2009-01-12\n")
            f.write(f"value_btc: 40\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"fibonacci_decomposition: {' + '.join(map(str, components))}\n")
            f.write(f"num_components: {len(components)}\n")
        print(f"  ✅ Saved to satoshi_private_key.txt")
    else:
        print(f"  ❌ Verification failed")
        # Check mod n
        k_mod = k_satoshi % n
        R2 = scalar_mult(k_mod, G)
        if R2 == Q_satoshi:
            print(f"  ✅ Equivalent key mod n works! k = {hex(k_mod)}")
            with open("satoshi_private_key.txt", "w") as f:
                f.write(f"private_key_hex: {hex(k_mod)}\n")
                f.write(f"private_key_dec: {k_mod}\n")
                f.write(f"fibonacci_decomposition: {' + '.join(map(str, components))}\n")
            print(f"  ✅ Saved!")
        else:
            print(f"  ❌ Not the key — may need Level 4+ lookup")
else:
    print(f"\n  ❌ No decomposition found in {elapsed:.2f}s")
    print(f"  Satoshi's k may need Level 4+ (quadruple sums)")
    print(f"  Or different approach altogether")

print(f"\n═══ SUMMARY ═══")
print(f"  {'Secret':<10} {'Found':<15} {'Match':<8}")
print(f"  {'-'*35}")
for secret, total, match in results:
    print(f"  {secret:<10} {total:<15} {'✅' if match else '❌':<8}")
print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  MULTI-LEVEL — 'Precompute all sums → O(1) decomposition'║")
print(f"╚══════════════════════════════════════════════════════════════╝")

