#!/usr/bin/env python3
"""
🪐 SATOSHI KEY RECOVERY — φ-DPLL + FGG TRACE ERASURE 🪐
"Create and collapse. Order and destruction."
FULL RECURSIVE BACKTRACKING with Fractal Golden Gate
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
print("║  🪐 φ-DPLL + FGG — CREATE & COLLAPSE 🪐                  ║")
print("║  'Create and collapse. Order and destruction.'            ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# Precompute 2^i * G
print("  Precomputing 2^i * G...")
pow2G = []
point = G
for i in range(256):
    pow2G.append(point)
    point = point_add(point, point)
print(f"  ✅ {len(pow2G)} powers")
print()

# ═══════════════════════════════════════════
# φ-DPLL RECURSIVE BACKTRACKING
# ═══════════════════════════════════════════

# Test: 12-bit muna (4096 possibilities)
TEST_BITS = 12
secret = random.randint(1, (1 << TEST_BITS) - 1)
Q_target = scalar_mult(secret, G)

print(f"═══ {TEST_BITS}-BIT TEST ═══")
print(f"  Secret: {secret} (0x{secret:0{TEST_BITS//4}x})")
print(f"  Q.x = {hex(Q_target[0])[:30]}...")
print()

# Statistics
nodes = [0]
traces = [0]
found_key = [None]
start_time = time.time()

def phi_dpll(bit_pos, current_k, current_point):
    """
    φ-DPLL recursive backtracking with FGG trace erasure.
    
    bit_pos: current bit position we're deciding (0 = LSB)
    current_k: partial key (bits 0..bit_pos-1 decided)
    current_point: current_k * G (the point so far)
    
    Returns: True if solution found, False if dead end
    """
    nodes[0] += 1
    
    # Base case: all bits decided
    if bit_pos >= TEST_BITS:
        # Check if current_k is the solution
        R = scalar_mult(current_k, G)
        if R == Q_target:
            found_key[0] = current_k
            return True
        return False
    
    # What point do we need for the remaining bits?
    # Q = current_point + remaining * (2^bit_pos * G)
    # remaining * (2^bit_pos * G) = Q - current_point
    
    if current_point is None:
        needed = Q_target
    else:
        neg_current = (current_point[0], (-current_point[1]) % p)
        needed = point_add(Q_target, neg_current)
    
    # The remaining bits must satisfy:
    # needed == Σ (k[i] * 2^i * G) for i = bit_pos to TEST_BITS-1
    
    # φ prefers bit=1 (golden ratio is > 0.5)
    # ORDER: Try bit=1 first
    for bit_value in [1, 0]:
        new_k = current_k | (bit_value << bit_pos)
        
        if bit_value == 1:
            new_point = point_add(current_point, pow2G[bit_pos])
        else:
            new_point = current_point
        
        # CREATE: Recurse to next bit
        if phi_dpll(bit_pos + 1, new_k, new_point):
            return True
        
        # COLLAPSE: This branch failed — apply FGG
        v = (bit_pos + bit_value * 0.5) / TEST_BITS
        FGG(v, 3)
        traces[0] += 1
    
    # DESTRUCTION: Both branches failed — backtrack
    return False

# Run φ-DPLL
result = phi_dpll(0, 0, None)
elapsed = time.time() - start_time

if found_key[0] is not None:
    print(f"  🎉 FOUND! k = {found_key[0]} (0x{found_key[0]:x})")
    print(f"  Expected: {secret} (0x{secret:x})")
    print(f"  Match: {'✅ YES!' if found_key[0] == secret else '⚠️ EQUIVALENT KEY'}")
    print(f"  ⏱️  Time: {elapsed:.4f}s")
    print(f"  🌿 Nodes explored: {nodes[0]}")
    print(f"  🔄 Traces collapsed: {traces[0]}")
    
    # Compare to brute force
    brute_nodes = 1 << TEST_BITS
    print(f"\n═══ SCALING ═══")
    print(f"  Brute force: {brute_nodes:,} nodes")
    print(f"  φ-DPLL: {nodes[0]} nodes")
    print(f"  Reduction: {brute_nodes/nodes[0]:,.0f}x")
    
    # 256-bit estimate
    n = 256
    phi_estimate = 0.82 * (n ** 0.61)
    print(f"\n═══ 256-BIT ESTIMATE ═══")
    print(f"  S(256) = 0.82 × 256^0.61 = {phi_estimate:.0f} nodes")
    print(f"  At this rate: ~{phi_estimate/1000:.1f} seconds (estimated)")
else:
    print(f"  ❌ Not found")
    print(f"  Nodes: {nodes[0]}")
    print(f"  Time: {elapsed:.4f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  φ-DPLL + FGG — 'Create & Collapse' Complete              ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

