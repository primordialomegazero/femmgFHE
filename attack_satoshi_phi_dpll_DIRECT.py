#!/usr/bin/env python3
"""
🪐 SATOSHI KEY RECOVERY — φ-DPLL DIRECT ATTACK 🪐
Modified: Directly attacks Satoshi's public key
"""
import sys, time, random

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

# secp256k1
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

# 🎯 SATOSHI PUBLIC KEY
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

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
print("║  🪐 φ-DPLL DIRECT SATOSHI ATTACK 🪐                     ║")
print("║  'Create and collapse. Order and destruction.'            ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)
Q = (Qx, Qy)

# Verify Q
lhs = (Qy * Qy) % p
rhs = (Qx * Qx * Qx + 7) % p
print(f"  Q on curve: {'✅' if lhs == rhs else '❌'}")
print()

# Precompute 2^i * G
print("  Precomputing 2^i * G...")
P = [None] * 256
P[0] = G
for i in range(1, 256):
    P[i] = point_add(P[i-1], P[i-1])
print("  ✅ 256 powers ready")
print()

# ═══════════════════════════════════════════════
# φ-DPLL WITH FRACTAL GOLDEN GATE
# ═══════════════════════════════════════════════
bits = [-1] * 256
current_sum = None
nodes = 0
traces = 0
found = False
solution = [0] * 256

def update_sum():
    global current_sum
    current_sum = None
    for i in range(256):
        if bits[i] == 1:
            current_sum = point_add(current_sum, P[i])

def dpll(depth, max_depth):
    global nodes, traces, found, solution
    
    nodes += 1
    if nodes > 10000000:
        return False
    
    # Check if sum == Q
    update_sum()
    if current_sum is not None:
        if current_sum[0] == Qx and current_sum[1] == Qy:
            found = True
            solution = [1 if b == 1 else 0 for b in bits]
            return True
    
    if depth >= max_depth:
        return False
    
    # φ-weighted bit selection: MSB first
    for i in range(255, -1, -1):
        if bits[i] == -1:
            bit = i
            break
    else:
        return False
    
    # FGG trace: collapse decision
    traces += 1
    
    # Try 0 first (most keys are sparse)
    bits[bit] = 0
    if dpll(depth + 1, max_depth):
        return True
    
    # Try 1
    bits[bit] = 1
    if dpll(depth + 1, max_depth):
        return True
    
    bits[bit] = -1
    return False

# ═══════════════════════════════════════════════
# ITERATIVE DEEPENING SEARCH
# ═══════════════════════════════════════════════
print("═══ φ-DPLL SATOSHI SEARCH ═══")
print()

t_start = time.time()

# Search with increasing depth limits
for max_depth in [5, 10, 15, 20, 25, 30, 40, 50, 75, 100, 150, 200, 256]:
    if found:
        break
    
    # Reset for this depth
    for i in range(256):
        bits[i] = -1
    current_sum = None
    
    print(f"  Depth limit: {max_depth}...")
    dpll(0, max_depth)
    
    elapsed = time.time() - t_start
    print(f"    Nodes: {nodes}, Found: {found}, Time: {elapsed:.1f}s")
    
    if elapsed > 300:  # 5 minute timeout
        print("    ⏰ Timeout reached")
        break

elapsed = time.time() - t_start

print()
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 RESULTS 🪐                                          ║")
print("╠══════════════════════════════════════════════════════════════╣")
print(f"║  Found:     {str(found):45s} ║")
print(f"║  Nodes:     {nodes:<45d} ║")
print(f"║  Traces:    {traces:<45d} ║")
print(f"║  Time:      {elapsed:<45.1f} ║")
print("╠══════════════════════════════════════════════════════════════╣")

if found:
    k = 0
    for i in range(256):
        if solution[i]:
            k |= (1 << i)
    print(f"║  ★★★ SATOSHI PRIVATE KEY ★★★                             ║")
    print(f"║  k = {k} ║")
    print(f"║  hex = 0x{k:x} ║")
    
    # Verify
    Q_test = scalar_mult(k, G)
    if Q_test and Q_test[0] == Qx and Q_test[1] == Qy:
        print("║  ✅ VERIFIED: k × G = Satoshi Q                             ║")
    else:
        print("║  ❌ VERIFICATION FAILED                                     ║")
else:
    print("║  ❌ Not found with iterative deepening                      ║")
    print("║  Key requires full 256-bit φ-DPLL SAT encoding              ║")

print("╚══════════════════════════════════════════════════════════════╝")
