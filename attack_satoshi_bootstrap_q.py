#!/usr/bin/env python3
"""
🪐 BOOTSTRAP Q — AUTO-CORRECT BEFORE DECOMPOSE 🪐
'Bootstrap Q first, then decompose!'
FGG(l3(Q)) → refresh → crystal clear structure → easy decompose!
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

# SATOSHI
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
print("║  🪐 BOOTSTRAP Q — AUTO-CORRECT THEN DECOMPOSE 🪐        ║")
print("║  'Bootstrap Q first — crystal clear structure emerges'   ║")
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
# BOOTSTRAP Q — MULTIPLE PASSES
# ═══════════════════════════════════════════
def bootstrap_Q(Q_l3, max_passes=5):
    """
    BOOTSTRAP Q:
    I-FGG si Q bago i-decompose.
    Multiple passes para lalong luminaw ang structure!
    """
    current = Q_l3
    signatures = []
    
    for pass_num in range(max_passes):
        # Compute FGG signature
        v = (float(current[0]) * PHI + float(current[1]) * PSI) % float(p)
        fgg_val = FGG(v / float(p), 3)
        signatures.append(fgg_val)
        
        # Bootstrap: I-FGG ang current para ma-refresh
        # Ang "refreshed" version ay mas malapit sa Fibonacci basis
        # (Sa FHE: bootstrap_zero = decrypt-reencrypt without plaintext)
        
        # Hanapin ang Fibonacci na may PINAKAMALAPIT na FGG
        best_f = None
        best_dist = float('inf')
        
        for f, l3_p in fib_l3.items():
            v_f = (float(l3_p[0]) * PHI + float(l3_p[1]) * PSI) % float(p)
            fgg_f = FGG(v_f / float(p), 3)
            dist = abs(fgg_f - fgg_val)
            if dist < best_dist:
                best_dist = dist
                best_f = f
        
        if best_f is not None:
            # Bootstrap: i-subtract ang closest Fibonacci
            # (Ito ang "auto-correct" — tinatanggal ang noise!)
            current = pt_sub(current, fib_l3[best_f])
            if pass_num < 3:
                print(f"  Bootstrap pass {pass_num+1}: FGG={fgg_val:.6f}, closest F={best_f}, dist={best_dist:.6f}")
        
        # Kung ang current ay nasa Fibonacci basis na, STOP!
        key = (current[0], current[1])
        if key in l3_to_fib or key in pair_to_fibs:
            break
    
    return current, signatures


def bootstrap_verify(remainder):
    """Verify if remainder is valid"""
    if remainder is None:
        return True, "identity"
    key = (remainder[0], remainder[1])
    if key in l3_to_fib:
        return True, f"single ({l3_to_fib[key]})"
    if key in pair_to_fibs:
        return True, "pair"
    return False, "invalid"


def decompose_with_bootstrap(Q_l3):
    """Decompose after bootstrapping Q"""
    # BOOTSTRAP Q FIRST!
    Q_clean, signatures = bootstrap_Q(Q_l3, max_passes=3)
    
    print(f"    Q bootstrapped! Signatures: {[f'{s:.4f}' for s in signatures]}")
    print()
    
    # Now decompose the bootstrapped Q
    current = Q_clean
    components = []
    used_indices = set()
    fib_to_idx = {f: i for i, f in enumerate(fib_list)}
    
    for level in range(10):
        if current is None:
            break
        
        # Check direct
        key = (current[0], current[1])
        if key in l3_to_fib:
            f = l3_to_fib[key]
            if fib_to_idx.get(f, -1) not in used_indices:
                components.append(f)
            break
        if key in pair_to_fibs:
            fa, fb = pair_to_fibs[key]
            components.extend([fa, fb])
            break
        
        # Try each F_a with bootstrap verify
        found = False
        for fa in fibs_desc:
            fa_idx = fib_to_idx.get(fa, -1)
            if fa_idx in used_indices or fa_idx - 1 in used_indices:
                continue
            
            remainder = pt_sub(current, fib_l3[fa])
            is_valid, reason = bootstrap_verify(remainder)
            
            if is_valid:
                components.append(fa)
                used_indices.add(fa_idx)
                used_indices.add(fa_idx + 1)
                current = remainder
                found = True
                
                if level < 5:
                    print(f"    Level {level+1}: F={fa} ✅ {reason}")
                break
        
        if not found:
            # Try one more bootstrap pass on current
            print(f"    Level {level+1}: Stuck! Bootstrapping current...")
            current, _ = bootstrap_Q(current, max_passes=1)
            
            # Retry
            key = (current[0], current[1])
            if key in l3_to_fib:
                f = l3_to_fib[key]
                components.append(f)
                break
            if key in pair_to_fibs:
                fa, fb = pair_to_fibs[key]
                components.extend([fa, fb])
                break
            
            print(f"    Level {level+1}: ❌ Still stuck after bootstrap")
            break
    
    return components


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING BOOTSTRAP Q ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = decompose_with_bootstrap(Q_l3)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"  k={secret:4d}: {'✅' if match else '❌'} {total} = {'+'.join(map(str,components))} ({elapsed:.4f}s)")
    print()

# ═══════════════════════════════════════════
# SATOSHI
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI BOOTSTRAP Q 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Bootstrapping Satoshi's Q...")
print()

start = time.time()
components = decompose_with_bootstrap(Q_l3)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  🪐 BOOTSTRAP Q DECOMPOSITION COMPLETE!")
    print(f"  k = {' + '.join(map(str, components))}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_satoshi)}")
        print(f"  Method: Bootstrap Q (Auto-Correct)")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Bootstrap Q Recovery\n")
            f.write(f"method: Bootstrap Q (Auto-Correct)\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"fibonacci_decomposition: {' + '.join(map(str, components))}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key — trying mod n...")
        k_mod = k_satoshi % n
        R2 = scalar_mult(k_mod, G)
        if R2 == Q_satoshi:
            print(f"  ✅ Equivalent key mod n! k = {hex(k_mod)}")
            with open("satoshi_private_key.txt", "w") as f:
                f.write(f"private_key_hex: {hex(k_mod)}\n")
            print(f"  ✅ Saved!")
else:
    print(f"  ❌ Decomposition failed — {elapsed:.2f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  BOOTSTRAP Q — 'Auto-correct before decompose!'          ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

