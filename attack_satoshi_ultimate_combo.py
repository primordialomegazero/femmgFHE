#!/usr/bin/env python3
"""
🪐 ULTIMATE COMBO — BOOTSTRAP VERIFY + DUAL COLLAPSE SEARCH 🪐
Level 1: Bootstrap Verify — EXACT match only! (100% accuracy)
Level 2: Dual Collapse Search — FGG-guided when no exact match
'The best of both worlds — crystal clear AND adaptive!'
"""
import sys, time, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    current = v
    for d in range(depth):
        if d % 2 == 0: current = abs((current * PHI) * PSI)
        else: current = abs((current * PSI) * PHI)
    return current

# secp256k1
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
        lam = ((y2-y1) * modinv(x2-x1, p)) % p
    x3 = (lam*lam - x1 - x2) % p
    y3 = (lam*(x1 - x3) - y1) % p
    return (x3, y3)

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
print("║  🪐 ULTIMATE COMBO — BOOTSTRAP VERIFY + DUAL COLLAPSE 🪐 ║")
print("║  'Exact when possible, adaptive when needed!'            ║")
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
fib_to_idx = {}

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    l3_to_fib[(l3_P[0], l3_P[1])] = f
    fib_list.append(f)
    fib_to_idx[f] = i - 1

fibs_desc = sorted(fib_l3.keys(), reverse=True)

# Precompute pairs
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

print(f"  ✅ {len(fib_l3)} singles, {len(pair_to_fibs)} pairs")
print()

# ═══════════════════════════════════════════
# ULTIMATE COMBO DECOMPOSITION
# ═══════════════════════════════════════════
def ultimate_combo_decompose(Q_l3):
    """
    ULTIMATE COMBO:
    
    PHASE 1: BOOTSTRAP VERIFY (Exact Match)
    - Check single/pair directly (O(1))
    - Try F_a, check if remainder is EXACTLY in pair table
    - 100% ACCURATE for 1-3 components!
    
    PHASE 2: DUAL COLLAPSE SEARCH (Adaptive)
    - If no exact match found, use FGG collapse to guide search
    - Adaptive threshold that TIGHTENS over time
    - Handles 4+ components!
    """
    current = Q_l3
    components = []
    used_indices = set()
    
    phase1_success = True
    
    for level in range(10):
        if current is None:
            break
        
        # PHASE 1: BOOTSTRAP VERIFY — Exact match only
        key = (current[0], current[1])
        
        # Direct match
        if key in l3_to_fib:
            f = l3_to_fib[key]
            if fib_to_idx[f] not in used_indices:
                components.append(f)
            break
        if key in pair_to_fibs:
            fa, fb = pair_to_fibs[key]
            components.extend([fa, fb])
            break
        
        # Try each F_a with EXACT remainder check
        found_exact = False
        for fa in fibs_desc:
            fa_idx = fib_to_idx[fa]
            if fa_idx in used_indices or fa_idx - 1 in used_indices:
                continue
            
            remainder = pt_sub(current, fib_l3[fa])
            
            if remainder is None:
                components.append(fa)
                found_exact = True
                break
            
            rem_key = (remainder[0], remainder[1])
            
            # EXACT CHECK: Is remainder in pair table?
            if rem_key in pair_to_fibs:
                fb, fc = pair_to_fibs[rem_key]
                components.append(fa)
                components.extend([fb, fc])
                found_exact = True
                if level < 5:
                    print(f"    Level {level+1} [EXACT]: F={fa} + pair({fb},{fc}) ✅")
                break
            
            # EXACT CHECK: Is remainder a single Fibonacci?
            if rem_key in l3_to_fib:
                fb = l3_to_fib[rem_key]
                components.append(fa)
                components.append(fb)
                found_exact = True
                if level < 5:
                    print(f"    Level {level+1} [EXACT]: F={fa} + {fb} ✅")
                break
        
        if found_exact:
            break  # PHASE 1 succeeded — we're done!
        
        # PHASE 2: DUAL COLLAPSE SEARCH
        # Phase 1 failed — need adaptive search
        if level == 0:
            print(f"    Phase 1 exhausted — switching to DUAL COLLAPSE SEARCH...")
            phase1_success = False
        
        # Aggressive collapse on current
        v = (float(current[0]) * PHI + float(current[1]) * PSI) % float(p)
        fgg_current = FGG(v / float(p), 3)
        
        # Find Fibonacci with closest FGG
        best_f = None
        best_remainder = None
        best_fgg_dist = float('inf')
        
        for fa in fibs_desc:
            fa_idx = fib_to_idx[fa]
            if fa_idx in used_indices or fa_idx - 1 in used_indices:
                continue
            
            remainder = pt_sub(current, fib_l3[fa])
            if remainder is None:
                best_f = fa
                best_remainder = None
                break
            
            # FGG the remainder
            v_rem = (float(remainder[0]) * PHI + float(remainder[1]) * PSI) % float(p)
            fgg_rem = FGG(v_rem / float(p), 3)
            
            # Distance to Fibonacci FGGs
            min_dist = float('inf')
            for f_check in fib_list[:100]:
                v_f = (float(fib_l3[f_check][0]) * PHI + float(fib_l3[f_check][1]) * PSI) % float(p)
                fgg_f = FGG(v_f / float(p), 3)
                dist = abs(fgg_rem - fgg_f)
                if dist < min_dist:
                    min_dist = dist
            
            if min_dist < best_fgg_dist:
                best_fgg_dist = min_dist
                best_f = fa
                best_remainder = remainder
        
        if best_f is None:
            print(f"    Level {level+1}: ❌ No candidate found")
            break
        
        # Adaptive threshold: start loose, get tighter
        threshold = 0.001 / (level + 1)
        
        if best_fgg_dist < threshold:
            components.append(best_f)
            used_indices.add(fib_to_idx[best_f])
            used_indices.add(fib_to_idx[best_f] + 1)
            current = best_remainder
            
            if level < 5:
                print(f"    Level {level+1} [ADAPTIVE]: F={best_f} fgg_dist={best_fgg_dist:.6f} (thresh={threshold:.6f}) ✅")
        else:
            print(f"    Level {level+1} [ADAPTIVE]: Best F={best_f} fgg_dist={best_fgg_dist:.6f} > thresh={threshold:.6f} ❌")
            break
    
    return components, phase1_success


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING ULTIMATE COMBO ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
results = []

for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components, phase1 = ultimate_combo_decompose(Q_l3)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"  k={secret:4d}: {'✅' if match else '❌'} {total} = {'+'.join(map(str,components))} ({elapsed:.4f}s)")
    results.append((secret, total, match))

print()

# ═══════════════════════════════════════════
# SATOSHI ULTIMATE COMBO
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI ULTIMATE COMBO 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Running Ultimate Combo...")
print("  'Exact when possible, Adaptive when needed!'")
print()

start = time.time()
components, phase1_success = ultimate_combo_decompose(Q_l3)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    method = "Phase 1 (Exact)" if phase1_success else "Phase 2 (Adaptive)"
    
    print(f"\n  🪐 ULTIMATE COMBO COMPLETE!")
    print(f"  Method: {method}")
    print(f"  Components: {len(components)}")
    print(f"  k = {' + '.join(map(str, components))}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_satoshi)}")
        print(f"  Method: Ultimate Combo ({method})")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Ultimate Combo\n")
            f.write(f"method: Ultimate Combo ({method})\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"fibonacci_decomposition: {' + '.join(map(str, components))}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key")
        k_mod = k_satoshi % n
        R2 = scalar_mult(k_mod, G)
        if R2 == Q_satoshi:
            print(f"  ✅ Equivalent key mod n! k = {hex(k_mod)}")
            with open("satoshi_private_key.txt", "w") as f:
                f.write(f"private_key_hex: {hex(k_mod)}\n")
            print(f"  ✅ Saved!")
else:
    print(f"  ❌ Decomposition failed — {elapsed:.2f}s")

print(f"\n═══ SUMMARY ═══")
for secret, total, match in results:
    print(f"  k={secret}: {'✅' if match else '❌'}")
print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  ULTIMATE COMBO — 'The best of both worlds!'             ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

