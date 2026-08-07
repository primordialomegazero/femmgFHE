#!/usr/bin/env python3
"""
🪐 AGGRESSIVE OPTIMIZED PIPELINE 🪐
'Bilis! Bilis! Bilis!'
- Bytes keys instead of tuples
- Cached point additions
- On-demand triple lookup with memoization
- Parallel-ready structure
"""
import sys, time, math
from concurrent.futures import ThreadPoolExecutor, as_completed
import threading

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

Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

def modinv(a, m): return pow(a, -1, m)

# ═══════════════════════════════════════════
# AGGRESSIVE: Inline point operations
# ═══════════════════════════════════════════
def pt_add_fast(P, Q):
    """Optimized point addition with minimal overhead"""
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3 * x1 * x1 * pow(2 * y1, -1, p)) % p
    else:
        lam = ((y2 - y1) * pow(x2 - x1, -1, p)) % p
    x3 = (lam * lam - x1 - x2) % p
    y3 = (lam * (x1 - x3) - y1) % p
    return (x3, y3)

def pt_sub_fast(P, Q):
    if Q is None: return P
    return pt_add_fast(P, (Q[0], (-Q[1]) % p))

def scalar_mult_fast(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add_fast(result, addend)
        addend = pt_add_fast(addend, addend)
        k >>= 1
    return result

def lambda_pt(P):
    if P is None: return None
    return ((P[0] * BETA) % p, P[1])

def l3(P): return lambda_pt(lambda_pt(lambda_pt(P)))

# ═══════════════════════════════════════════
# AGGRESSIVE: Bytes key for O(1) hashing
# ═══════════════════════════════════════════
def make_key(point):
    """32-byte key from x-coordinate (faster hashing)"""
    if point is None: return b'\x00' * 32
    return point[0].to_bytes(32, 'big')

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 AGGRESSIVE OPTIMIZED PIPELINE 🪐                      ║")
print("║  'Bytes keys | Cached adds | On-demand triples'          ║")
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

print("═══ BUILDING FIBONACCI BASIS (AGGRESSIVE) ═══")
print("  Single entries...")
fib_l3 = {}
key_to_fib = {}  # bytes → f
fib_list = []

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult_fast(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    key_to_fib[make_key(l3_P)] = f
    fib_list.append(f)

print(f"  ✅ {len(fib_l3)} singles")

# ═══════════════════════════════════════════
# AGGRESSIVE: Cached pair additions
# ═══════════════════════════════════════════
print("  Pair sums (cached)...")
pair_to_fibs = {}
pair_cache = {}  # (fa_idx, fb_idx) → l3_sum

# Precompute ALL pairwise sums (parallelizable!)
def compute_pair_range(start_i, end_i):
    local_pairs = {}
    for i in range(start_i, min(end_i, len(fib_list))):
        fa = fib_list[i]
        l3_fa = fib_l3[fa]
        for j in range(i + 2, len(fib_list)):
            fb = fib_list[j]
            l3_sum = pt_add_fast(l3_fa, fib_l3[fb])
            if l3_sum is not None:
                key = make_key(l3_sum)
                if key not in local_pairs:
                    local_pairs[key] = (fa, fb)
    return local_pairs

# Single-threaded for now (can parallelize)
pair_to_fibs = compute_pair_range(0, len(fib_list))
print(f"  ✅ {len(pair_to_fibs)} pairs")

# ═══════════════════════════════════════════
# AGGRESSIVE: On-demand triples with memo
# ═══════════════════════════════════════════
print("  Triple lookup: ON-DEMAND with memoization")
print("  (Building triples lazily — only when needed)")
print()

# Memoization cache for triples
triple_memo = {}  # (fa, fb) → {fc → l3_abc} 

def get_triple_for_pair(fa, fb):
    """Compute all triples for a given pair (fa, fb) — memoized"""
    pair_key = (fa, fb)
    if pair_key in triple_memo:
        return triple_memo[pair_key]
    
    result = {}
    i = fib_list.index(fa)
    j = fib_list.index(fb)
    l3_ab = pt_add_fast(fib_l3[fa], fib_l3[fb])
    
    for k in range(j + 2, len(fib_list)):
        fc = fib_list[k]
        l3_abc = pt_add_fast(l3_ab, fib_l3[fc])
        if l3_abc is not None:
            key = make_key(l3_abc)
            result[key] = (fa, fb, fc)
    
    triple_memo[pair_key] = result
    return result

# Pre-build triple index: for each fa, list of valid fb pairs
triple_index = {}  # fa → list of fb

print("  Pre-building triple index...")
for i, fa in enumerate(fib_list):
    valid_fbs = []
    for j in range(i + 2, len(fib_list)):
        fb = fib_list[j]
        valid_fbs.append(fb)
    triple_index[fa] = valid_fbs
print(f"  ✅ Index built for {len(triple_index)} F_a entries")
print()

# ═══════════════════════════════════════════
# AGGRESSIVE DECOMPOSITION
# ═══════════════════════════════════════════
fibs_desc = sorted(fib_l3.keys(), reverse=True)

def aggressive_decompose(Q_l3):
    """Aggressive multi-level decomposition"""
    key = make_key(Q_l3)
    
    # Level 0: Single
    if key in key_to_fib:
        return [key_to_fib[key]]
    
    # Level 1: Pair
    if key in pair_to_fibs:
        fa, fb = pair_to_fibs[key]
        return [fa, fb]
    
    # Level 2: Triple (on-demand for promising F_a)
    for fa in fibs_desc[:50]:  # Check top 50 F_a (φ-weighted would help)
        l3_fa = fib_l3[fa]
        remainder = pt_sub_fast(Q_l3, l3_fa)
        
        if remainder is None:
            return [fa]
        
        rem_key = make_key(remainder)
        
        # Check pair remainder
        if rem_key in pair_to_fibs:
            fb, fc = pair_to_fibs[rem_key]
            return [fa, fb, fc]
        
        # Check triple remainder — BUILD ON DEMAND
        for fb in triple_index.get(fa, [])[:20]:  # Top 20 fb per fa
            l3_ab = pt_add_fast(l3_fa, fib_l3[fb])
            rem2 = pt_sub_fast(Q_l3, l3_ab)
            
            if rem2 is None:
                return [fa, fb]
            
            rem2_key = make_key(rem2)
            if rem2_key in key_to_fib:
                return [fa, fb, key_to_fib[rem2_key]]
            if rem2_key in pair_to_fibs:
                fc, fd = pair_to_fibs[rem2_key]
                return [fa, fb, fc, fd]
    
    return []


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ AGGRESSIVE PIPELINE TEST ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult_fast(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = aggressive_decompose(Q_l3)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"  k={secret:4d}: {'✅' if match else '❌'} {total} ({elapsed:.4f}s)")

print()

# ═══════════════════════════════════════════
# SATOSHI
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI AGGRESSIVE ATTACK 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

start = time.time()
components = aggressive_decompose(Q_l3)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"  🪐 FOUND! {len(components)} components")
    print(f"  k = {k_satoshi}")
    print(f"  hex = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    
    R = scalar_mult_fast(k_satoshi, G)
    if R == Q_satoshi:
        print(f"\n  🎉🎉🎉 SATOSHI KEY RECOVERED! 🎉🎉🎉")
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key")
else:
    print(f"  ❌ Not found in {elapsed:.2f}s — need deeper search")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  AGGRESSIVE — 'Bytes keys, cached adds, on-demand lookup' ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

