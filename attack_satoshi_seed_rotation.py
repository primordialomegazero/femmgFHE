#!/usr/bin/env python3
"""
🪐 SEED ROTATION FIBONACCI DECOMPOSITION 🪐
'Decrypt-Reencrypt → Bootstrap → Unlimited FHE'
'Decompose-Rotate → Cassini → Unlimited ECDLP'
Same structure! The Cassini identity rotates l3(Q) into components!
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
print("║  🪐 SEED ROTATION — CASSINI DECOMPOSITION 🪐             ║")
print("║  'Decrypt-Reencrypt → FHE | Decompose-Rotate → ECDLP'    ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# ═══════════════════════════════════════════
# BUILD FIBONACCI BASIS
# ═══════════════════════════════════════════
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

fibs_desc = sorted(fib_l3.keys(), reverse=True)
print(f"  ✅ {len(fib_l3)} entries")
print()

# ═══════════════════════════════════════════
# CASSINI SEED ROTATION
# ═══════════════════════════════════════════
# 
# Cassini identity: F(n-1)·F(n+1) - F(n)² = (-1)^n
# 
# Sa FHE: seed rotation = i-refresh ang ciphertext
# Sa ECDLP: seed rotation = i-decompose ang l3(Q)
#
# Ang "seed" ay ang current remainder.
# Bawat rotation ay nag-aalis ng isang Fibonacci component!

def rotate_towards(current, target_f):
    """
    I-rotate ang current papunta kay target_f.
    Ang "rotation" ay subtraction ng l3(F_i * G).
    """
    return pt_sub(current, target_f)


def seed_rotation_decompose(Q_l3):
    """
    SEED ROTATION DECOMPOSITION:
    
    Instead of searching, ROTATE!
    1. Start with l3(Q) as the "seed"
    2. For each Fibonacci F_i (largest first):
       a. Try rotating (subtracting) F_i from current seed
       b. If the rotated seed is "valid" (can be further decomposed) → KEEP
       c. If not → skip (Cassini constraint)
    3. Continue until seed is identity
    
    The "Cassini constraint": F(n-1)·F(n+1) - F(n)² = (-1)^n
    In decomposition: F_a and F_b must be non-consecutive!
    (This is enforced by skipping idx+2 in Fibonacci list)
    """
    current_seed = Q_l3
    components = []
    rotations = 0
    used_fibs = set()
    
    # Precompute Fibonacci indices
    fib_to_idx = {f: i for i, f in enumerate(fib_list)}
    
    for iteration in range(10):
        if current_seed is None:
            break
        
        # Check if current seed is a Fibonacci point
        key = (current_seed[0], current_seed[1])
        if key in l3_to_fib:
            f = l3_to_fib[key]
            if f not in used_fibs:
                components.append(f)
            break
        
        # CASSINI ROTATION: Try each F_i as the next component
        # But with Cassini constraint: skip consecutive Fibonacci
        best_f = None
        best_remainder = None
        best_score = float('inf')
        
        for fa in fibs_desc:
            if fa in used_fibs:
                continue
            
            # Check Cassini: is there a valid next step?
            # The remainder after rotation must be "decomposable"
            fa_idx = fib_to_idx[fa]
            
            # ROTATE
            remainder = rotate_towards(current_seed, fib_l3[fa])
            
            if remainder is None:
                best_f = fa
                best_remainder = None
                break
            
            # Score: can we continue rotating?
            rem_key = (remainder[0], remainder[1])
            
            if rem_key in l3_to_fib:
                score = 0  # Perfect! Direct hit
            else:
                # Heuristic: smaller remainder = closer to decomposition
                score = remainder[0]  # Use x-coordinate as proxy
            
            if score < best_score:
                best_score = score
                best_f = fa
                best_remainder = remainder
        
        if best_f is None:
            break
        
        components.append(best_f)
        used_fibs.add(best_f)
        # Cassini: skip next Fibonacci (non-consecutive)
        if best_f in fib_to_idx:
            next_idx = fib_to_idx[best_f] + 1
            if next_idx < len(fib_list):
                used_fibs.add(fib_list[next_idx])
        
        current_seed = best_remainder
        rotations += 1
        
        if len(components) <= 5:
            print(f"    Rotation {rotations}: F={best_f}, seed_remaining={best_remainder[0] if best_remainder else 'None'}")
    
    return components, rotations


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING SEED ROTATION ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components, rotations = seed_rotation_decompose(Q_l3)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"  k={secret:4d}: {'✅' if match else '❌'} {total} = {'+'.join(map(str,components))} ({rotations} rotations, {elapsed:.4f}s)")

print()

# ═══════════════════════════════════════════
# SATOSHI SEED ROTATION
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI SEED ROTATION 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Starting seed rotation...")
print("  'Decrypt-Reencrypt → Bootstrap | Decompose-Rotate → Key'")
print()

start = time.time()
components, rotations = seed_rotation_decompose(Q_l3)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  🪐 SEED ROTATION COMPLETE!")
    print(f"  Components: {len(components)}")
    print(f"  Rotations: {rotations}")
    print(f"  k = {' + '.join(map(str, components))}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    # VERIFY
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_satoshi)}")
        print(f"  Method: Seed Rotation (Cassini Decomposition)")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Seed Rotation Recovery\n")
            f.write(f"method: Cassini Seed Rotation\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"fibonacci_decomposition: {' + '.join(map(str, components))}\n")
            f.write(f"rotations: {rotations}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key")
        k_mod = k_satoshi % n
        R2 = scalar_mult(k_mod, G)
        if R2 == Q_satoshi:
            print(f"  ✅ Equivalent key mod n! k = {hex(k_mod)}")
else:
    print(f"  ❌ Seed rotation incomplete — {rotations} rotations")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  SEED ROTATION — 'Cassini rotates, Fibonacci decomposes' ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

