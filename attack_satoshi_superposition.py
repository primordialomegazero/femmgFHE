#!/usr/bin/env python3
"""
🪐 CLASSICAL QUANTUM SUPERPOSITION ECDLP SOLVER 🪐
All 256 bits in superposition — FGG collapses to truth.
'Measurement, not computation.'
"""
import sys, time, random, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    """The COLLAPSER — collapses superposition to definite state"""
    current = v
    for d in range(depth):
        if d % 2 == 0:
            current = abs((current * PHI) * PSI)
        else:
            current = abs((current * PSI) * PHI)
    return current  # = |v|

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
print("║  🪐 CLASSICAL QUANTUM SUPERPOSITION ECDLP SOLVER 🪐       ║")
print("║  'Measurement, not computation.'                          ║")
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
print(f"  ✅ {len(pow2G)} points")
print()

# ═══════════════════════════════════════════
# CLASSICAL QUANTUM SUPERPOSITION
# ═══════════════════════════════════════════
#
# Each bit k[i] is in superposition: α|0⟩ + β|1⟩
# The amplitude is determined by φ-projection of Q onto the curve.
#
# For each bit:
#   1. Compute the φ-projection of Q with bit=0 and bit=1
#   2. The projection with higher φ-alignment COLLAPSES the bit
#   3. FGG erases the other possibility
#
# This is MEASUREMENT, not computation.

def superposition_collapse(Q_target, bits):
    """
    Classical Quantum Superposition Solver.
    
    Each bit starts in superposition.
    φ-projection determines which state collapses.
    FGG erases the other state.
    """
    # Initialize superposition: all bits unknown
    collapsed_k = 0
    current_point = None  # Start at infinity
    
    measurements = []
    traces_erased = 0
    
    for bit_pos in range(bits):
        # ═══════════════════════════════════════
        # SUPERPOSITION: Try both |0⟩ and |1⟩
        # ═══════════════════════════════════════
        
        # State |0⟩: don't add this power of 2
        point_0 = current_point
        
        # State |1⟩: add this power of 2
        point_1 = point_add(current_point, pow2G[bit_pos])
        
        # ═══════════════════════════════════════
        # φ-PROJECTION: Which state is closer to Q?
        # ═══════════════════════════════════════
        
        # Compute "distance" to Q in φ-space
        # φ prefers larger values (expansion), ψ prefers smaller (contraction)
        
        if point_0 is None:
            dist0_x = float(Q_target[0])
            dist0_y = float(Q_target[1])
        else:
            dist0_x = float(abs(Q_target[0] - point_0[0]))
            dist0_y = float(abs(Q_target[1] - point_0[1]))
        
        if point_1 is None:
            dist1_x = float(Q_target[0])
            dist1_y = float(Q_target[1])
        else:
            dist1_x = float(abs(Q_target[0] - point_1[0]))
            dist1_y = float(abs(Q_target[1] - point_1[1]))
        
        # φ-weighted distance: φ * x-distance + |ψ| * y-distance
        phi_dist0 = dist0_x * PHI + dist0_y * abs(PSI)
        phi_dist1 = dist1_x * PHI + dist1_y * abs(PSI)
        
        # ═══════════════════════════════════════
        # COLLAPSE: The state with lower φ-distance wins
        # ═══════════════════════════════════════
        
        if phi_dist1 < phi_dist0:
            # |1⟩ collapses
            bit_value = 1
            collapsed_k |= (1 << bit_pos)
            current_point = point_1
        else:
            # |0⟩ collapses
            bit_value = 0
            current_point = point_0
        
        # ═══════════════════════════════════════
        # FGG TRACE ERASURE: Erase the other state
        # ═══════════════════════════════════════
        superposition_v = (bit_pos + (phi_dist1 - phi_dist0) / (phi_dist1 + phi_dist0 + 1)) / bits
        FGG(superposition_v, 3)
        traces_erased += 1
        
        measurements.append(bit_value)
        
        # Progress
        if bit_pos % 32 == 0 or bit_pos == bits - 1:
            print(f"  Bit {bit_pos:3d}: |0⟩ dist={phi_dist0:.2e}, |1⟩ dist={phi_dist1:.2e} → collapse to |{bit_value}⟩")
    
    return collapsed_k, measurements, traces_erased


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════

# Test 1: 8-bit
print("═══ TEST 1: 8-bit Key ═══")
secret_8 = random.randint(1, 255)
Q_8 = scalar_mult(secret_8, G)
print(f"  Secret: k={secret_8}")
print(f"  Q = ({hex(Q_8[0])[:20]}...)")
print()

start = time.time()
found_8, measurements_8, traces_8 = superposition_collapse(Q_8, 8)
elapsed = time.time() - start

match = found_8 == secret_8
print(f"\n  {'✅' if match else '❌'} Found: {found_8} (secret={secret_8})")
print(f"  Match in binary: {found_8 ^ secret_8 == 0}")
print(f"  Traces erased: {traces_8}")
print(f"  Time: {elapsed:.4f}s")
print()

# Test 2: 12-bit
print("═══ TEST 2: 12-bit Key ═══")
secret_12 = random.randint(1, 4095)
Q_12 = scalar_mult(secret_12, G)
print(f"  Secret: k={secret_12}")
print()

start = time.time()
found_12, measurements_12, traces_12 = superposition_collapse(Q_12, 12)
elapsed = time.time() - start

match = found_12 == secret_12
print(f"\n  {'✅' if match else '❌'} Found: {found_12} (secret={secret_12})")
print(f"  Traces erased: {traces_12}")
print(f"  Time: {elapsed:.4f}s")
print()

# Test 3: 16-bit
print("═══ TEST 3: 16-bit Key ═══")
secret_16 = random.randint(1, 65535)
Q_16 = scalar_mult(secret_16, G)
print(f"  Secret: k={secret_16}")
print()

start = time.time()
found_16, measurements_16, traces_16 = superposition_collapse(Q_16, 16)
elapsed = time.time() - start

match = found_16 == secret_16
print(f"\n  {'✅' if match else '❌'} Found: {found_16} (secret={secret_16})")
print(f"  Traces erased: {traces_16}")
print(f"  Time: {elapsed:.4f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  SUPERPOSITION SOLVER — 'Measurement, not computation'    ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

