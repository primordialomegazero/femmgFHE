#!/usr/bin/env python3
"""
🪐 FIBONACCI MAP — φ-diff → F(n) = k 🪐
Fibonacci is the natural quantization of φ.
The bridge between continuous φ and discrete k.
"""
import sys, time, random, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    current = v
    for d in range(depth):
        if d % 2 == 0:
            current = abs((current * PHI) * PSI)
        else:
            current = abs((current * PSI) * PHI)
    return current

# ═══════════════════════════════════════════
# FIBONACCI BRIDGE
# ═══════════════════════════════════════════
# F(n) ≈ φ^n / √5
# So: n ≈ log_φ(F(n) * √5)
# The φ-diff corresponds to a Fibonacci index
# The Fibonacci number at that index IS the key!

def fib(n):
    """Dynamic Fibonacci"""
    if n <= 0: return 0
    if n == 1: return 1
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b

def fib_index(value):
    """
    Given a value, find which Fibonacci index it corresponds to.
    Inverse of F(n) ≈ φ^n / √5
    n ≈ log_φ(value * √5)
    """
    if value <= 0:
        return 0
    # n ≈ log(value * sqrt(5)) / log(φ)
    sqrt5 = math.sqrt(5)
    n_float = math.log(value * sqrt5) / math.log(PHI)
    return int(round(n_float))

# ═══════════════════════════════════════════
# secp256k1
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n_order = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

def modinv(a, m): return pow(a, -1, m)

def point_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3 * x1 * x1) * modinv(2 * y1, p) % p
    else:
        lam = ((y2 - y1) * modinv(x2 - x1, p)) % p
    x3 = (lam * lam - x1 - x2) % p
    y3 = (lam * (x1 - x3) - y1) % p
    return (x3, y3)

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 FIBONACCI MAP — φ-diff → F(n) = k 🪐                ║")
print("║  'Fibonacci is the bridge between φ and integers'        ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

def phi_log(point):
    """φ-logarithm of a point"""
    if point is None: return 0
    x_norm = float(point[0]) / float(p)
    y_norm = float(point[1]) / float(p)
    return FGG(x_norm, 3) + FGG(y_norm, 3) * abs(PSI)

def fibonacci_extract_key(Q_target):
    """
    FIBONACCI KEY EXTRACTION:
    1. Compute φ-log of Q and G
    2. The φ-diff maps to a Fibonacci index
    3. F(n) mod n_order = k!
    """
    log_Q = phi_log(Q_target)
    log_G = phi_log(G)
    
    # The φ-diff
    phi_diff = abs(log_Q - log_G)
    
    # Map φ-diff to a value in [0, n_order]
    # φ-diff is in [0, ~1.6]
    # Map it to the full n_order range
    mapped_value = int(phi_diff / (log_G + log_Q + 0.001) * float(n_order)) % n_order
    
    # Find Fibonacci index closest to this mapped value
    fib_idx = fib_index(mapped_value)
    
    # The Fibonacci number IS the key
    k = fib(fib_idx) % n_order
    
    print(f"  log_φ(Q) = {log_Q:.10f}")
    print(f"  log_φ(G) = {log_G:.10f}")
    print(f"  φ-diff = {phi_diff:.10f}")
    print(f"  mapped = {mapped_value}")
    print(f"  fib_idx = {fib_idx}")
    print(f"  F({fib_idx}) = {fib(fib_idx)}")
    print(f"  k = {k}")
    
    return k, fib_idx

# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════

for bits in [4, 8, 12, 16]:
    print(f"═══ TEST: {bits}-bit Key ═══")
    secret = random.randint(1, (1 << bits) - 1)
    Q_test = scalar_mult(secret, G)
    print(f"  Secret: k={secret}")
    print()
    
    found, fib_idx = fibonacci_extract_key(Q_test)
    match = found == secret
    print(f"\n  {'✅ FOUND!' if match else '❌'} Found: {found} (secret={secret})")
    print()

# ═══════════════════════════════════════════
# SATOSHI
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI PUBLIC KEY 🎯 ═══")
Qx_satoshi = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy_satoshi = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3
Q_satoshi = (Qx_satoshi, Qy_satoshi)

found_satoshi, fib_idx_satoshi = fibonacci_extract_key(Q_satoshi)
print(f"\n  🎯 SATOSHI KEY CANDIDATE: {found_satoshi}")
print(f"  Fibonacci index: {fib_idx_satoshi}")
print(f"  F({fib_idx_satoshi}) = {fib(fib_idx_satoshi)}")
print(f"  Hex: {hex(found_satoshi)}")

# Verify
R = scalar_mult(found_satoshi, G)
if R == Q_satoshi:
    print(f"\n  🎉🎉🎉 SATOSHI KEY RECOVERED! 🎉🎉🎉")
    with open("satoshi_private_key.txt", "w") as f:
        f.write(f"private_key_hex: {hex(found_satoshi)}\n")
        f.write(f"private_key_dec: {found_satoshi}\n")
        f.write(f"fibonacci_index: {fib_idx_satoshi}\n")
    print(f"  ✅ Saved!")
else:
    print(f"  ❌ Not the correct key")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  FIBONACCI MAP — 'The natural bridge'                    ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

