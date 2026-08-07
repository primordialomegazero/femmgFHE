#!/usr/bin/env python3
"""
🪐 FRACTAL FIBONACCI LOOKUP 🪐
Self-similar decomposition — same algorithm at every scale!
'Fractal: ang parehong algorithm, mas maliit na remainder'
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
print("║  🪐 FRACTAL FIBONACCI LOOKUP — SELF-SIMILAR DECOMPOSE 🪐 ║")
print("║  'Same algorithm, smaller remainder — fractal!'          ║")
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

print("  Building Fibonacci l3 basis...")
fib_l3 = {}
l3_to_fib = {}
for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    l3_to_fib[(l3_P[0], l3_P[1])] = f

fibs_desc = sorted(fib_l3.keys(), reverse=True)
print(f"  ✅ {len(fib_l3)} entries")
print()

# ═══════════════════════════════════════════
# FRACTAL DECOMPOSITION
# ═══════════════════════════════════════════
def fractal_decompose(current_l3, start_idx=0, max_depth=10):
    """
    FRACTAL FIBONACCI DECOMPOSITION:
    
    Self-similar: the SAME algorithm works at EVERY scale!
    
    1. Check if current_l3 is a single Fibonacci point → DONE!
    2. Try each F_a (from start_idx, largest first)
    3. Subtract: remainder = current_l3 - l3(F_a * G)
    4. RECURSE: fractal_decompose(remainder, idx+2)
       (idx+2 = skip next Fibonacci for non-consecutive)
    5. If recursion succeeds → F_a is correct!
    6. If recursion fails → try next F_a
    
    This is OPTIMIZED recursion:
    - Each level reduces the problem size
    - Non-consecutive constraint limits branching
    - Direct lookup catches single/identity cases
    """
    # Base case: identity
    if current_l3 is None:
        return []
    
    # Base case: single Fibonacci point
    key = (current_l3[0], current_l3[1])
    if key in l3_to_fib:
        return [l3_to_fib[key]]
    
    # Base case: max depth reached
    if max_depth <= 0:
        return None
    
    # Try each F_a from start_idx
    for idx in range(start_idx, len(fibs_desc)):
        fa = fibs_desc[idx]
        l3_fa = fib_l3[fa]
        
        # Subtract
        remainder = pt_sub(current_l3, l3_fa)
        
        # RECURSE: decompose remainder (skip next Fibonacci)
        sub_components = fractal_decompose(remainder, idx + 2, max_depth - 1)
        
        if sub_components is not None:
            # Success! F_a is correct
            return [fa] + sub_components
    
    # No valid decomposition
    return None


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING FRACTAL DECOMPOSITION ═══")
print()

# Test muna sa known keys
test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = fractal_decompose(Q_l3, 0, 10)
    elapsed = time.time() - start
    
    if components:
        total = sum(components)
        match = total == secret
        print(f"  k={secret:4d}: {'✅' if match else '❌'} got {total} = {'+'.join(map(str,components))} ({elapsed:.4f}s)")
    else:
        print(f"  k={secret:4d}: ❌ no decomposition ({elapsed:.4f}s)")

print()

# ═══════════════════════════════════════════
# SATOSHI ATTACK
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI FRACTAL ATTACK 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Starting fractal decomposition...")
print("  (This may take a while — fractal search)")
print()

start = time.time()
components = fractal_decompose(Q_l3, 0, 10)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"  🪐 DECOMPOSITION FOUND!")
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
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Block 170, Output 1 (40 BTC)\n")
            f.write(f"tx: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"fibonacci_decomposition: {' + '.join(map(str, components))}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Verification failed")
        # Maybe equivalent key (mod n)?
        k_mod = k_satoshi % n
        R2 = scalar_mult(k_mod, G)
        if R2 == Q_satoshi:
            print(f"  ✅ Equivalent key mod n works! k = {hex(k_mod)}")
else:
    print(f"  ❌ No decomposition found in {elapsed:.2f}s")
    print(f"  May need deeper fractal search or 4+ components")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  FRACTAL LOOKUP — 'Self-similar: same algorithm, any k'  ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

