#!/usr/bin/env python3
"""
🪐 DOUBLE ATTACK — FIBONACCI SPIRAL ECDLP 🪐
Attack 1: Cassini Seed Rotation — rotate Q along Fibonacci spiral
Attack 2: Fractal Golden Gate — collapse intermediate points
"Natural shit" — golden ratio properties, not computation
"""
import sys, time, random, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    """Fractal Golden Gate — collapse to |v|"""
    current = v
    for d in range(depth):
        if d % 2 == 0:
            current = abs((current * PHI) * PSI)
        else:
            current = abs((current * PSI) * PHI)
    return current

# ═══════════════════════════════════════════
# CASSINI IDENTITY
# ═══════════════════════════════════════════
# F(n-1) · F(n+1) - F(n)² = (-1)^n
# This is the NATURAL rotation engine.

def fibonacci(n):
    """Generate Fibonacci numbers up to n"""
    a, b = 0, 1
    fibs = [a, b]
    for _ in range(n - 1):
        a, b = b, a + b
        fibs.append(b)
    return fibs

# Precompute Fibonacci numbers
FIB = fibonacci(300)  # Enough for 256-bit work

def cassini_verify(n):
    """Verify Cassini identity: F(n-1)*F(n+1) - F(n)² = (-1)^n"""
    return FIB[n-1] * FIB[n+1] - FIB[n] * FIB[n] == ((-1) ** n)

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

def point_sub(P, Q):
    """P - Q"""
    if Q is None: return P
    return point_add(P, (Q[0], (-Q[1]) % p))

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 DOUBLE ATTACK — FIBONACCI SPIRAL ECDLP 🪐            ║")
print("║  Attack 1: Cassini Seed Rotation                         ║")
print("║  Attack 2: Fractal Golden Gate Collapse                  ║")
print("║  'Natural shit' — golden ratio properties               ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# Verify Cassini
print(f"  Cassini identity verified: {'✅' if cassini_verify(10) and cassini_verify(20) else '❌'}")
print(f"  φ·ψ = {PHI * PSI:.10f} (should be -1)")
print()

# ═══════════════════════════════════════════
# DOUBLE ATTACK: Fibonacci Spiral ECDLP
# ═══════════════════════════════════════════
#
# KEY INSIGHT:
# Instead of searching for k, we ROTATE Q along the Fibonacci spiral.
# Each rotation moves Q closer to G.
# The number of rotations = private key!
#
# How rotation works:
# 1. Start with Q (Satoshi's public key)
# 2. Compute the "Cassini direction" — φ-weighted projection
# 3. Rotate Q by subtracting Fibonacci-weighted multiples of G
# 4. Apply FGG collapse to erase the rotation trace
# 5. Repeat until Q = G (then rotation count = k)
#    OR until Q = O (point at infinity, then n - rotation count = k)

def cassini_direction(Q_current, fib_n):
    """
    Determine the Cassini rotation direction.
    Uses φ to project Q onto the Fibonacci spiral.
    Returns: +1 (clockwise) or -1 (counter-clockwise)
    """
    # Map Q's coordinates to φ-space
    qx_phi = (Q_current[0] * PHI) % p
    qy_phi = (Q_current[1] * PHI) % p
    
    # Fibonacci anchor point at this step
    fib_anchor_x = (Gx * fib_n) % p
    fib_anchor_y = (Gy * fib_n) % p
    
    # Distance in φ-space
    dist_pos = (qx_phi - fib_anchor_x) ** 2 + (qy_phi - fib_anchor_y) ** 2
    dist_neg = (qx_phi + fib_anchor_x) ** 2 + (qy_phi + fib_anchor_y) ** 2
    
    return 1 if dist_pos < dist_neg else -1


def fibonacci_spiral_solve(Q_target, max_steps=None):
    """
    DOUBLE ATTACK SOLVER:
    Rotate Q along the Fibonacci spiral until it hits G.
    """
    if max_steps is None:
        max_steps = n_order  # Theoretical max
    
    Q_current = Q_target
    rotation_count = 0
    collapsed_count = 0
    nodes = 0
    
    fib_index = 1  # Start from F(1)
    
    print("  Rotating Q along Fibonacci spiral...")
    print(f"  Starting Q.x = {hex(Q_current[0])[:30]}...")
    print()
    
    while rotation_count < max_steps:
        nodes += 1
        
        # Check if we reached G or O
        if Q_current == G:
            return rotation_count, nodes, collapsed_count, "G"
        
        if Q_current is None:
            # Hit point at infinity — rotation count = n - k
            return (n_order - rotation_count) % n_order, nodes, collapsed_count, "O"
        
        # Get Fibonacci number for this step
        fib_n = FIB[fib_index] % n_order
        
        # Determine rotation direction using Cassini
        direction = cassini_direction(Q_current, fib_n)
        
        # ROTATE: Subtract direction * fib_n * G
        rotation_point = scalar_mult(fib_n, G)
        if direction == 1:
            Q_next = point_sub(Q_current, rotation_point)
        else:
            Q_next = point_add(Q_current, rotation_point)
        
        # COLLAPSE: Apply FGG to the intermediate point
        if Q_next is not None:
            v_x = float(Q_next[0] % 10000) / 10000.0
            v_y = float(Q_next[1] % 10000) / 10000.0
            FGG(v_x, 3)
            FGG(v_y, 3)
            collapsed_count += 1
        
        Q_current = Q_next
        rotation_count += direction
        fib_index += 1
        
        # Progress
        if nodes % 50 == 0 or nodes <= 5:
            dist_to_G = "?"
            if Q_current is not None and Q_current != G:
                dist_to_G = f"0x{hex(abs(Q_current[0] - Gx) % p)[:10]}..."
            print(f"  Step {nodes:4d}: fib={FIB[fib_index-1]}, dir={direction:+d}, "
                  f"rot={rotation_count}, Q.x={hex(Q_current[0])[:20] if Q_current else 'INF'}..., dist={dist_to_G}")
    
    return None, nodes, collapsed_count, "MAX"


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════

# Test with a SMALL key first
print("═══ TEST 1: Small Key (k=5) ═══")
secret_k = 5
Q_test = scalar_mult(secret_k, G)
print(f"  Secret: k={secret_k}")
print(f"  Q = ({hex(Q_test[0])[:20]}..., {hex(Q_test[1])[:20]}...)")
print()

start = time.time()
found_k, nodes, collapsed, hit = fibonacci_spiral_solve(Q_test, max_steps=1000)
elapsed = time.time() - start

print(f"\n═══ RESULT ═══")
if found_k is not None:
    match = found_k == secret_k
    print(f"  {'✅' if match else '⚠️'} Found k={found_k} (secret={secret_k})")
    print(f"  Hit: {hit}")
    print(f"  Nodes: {nodes}")
    print(f"  Collapses: {collapsed}")
    print(f"  Time: {elapsed:.4f}s")
else:
    print(f"  ❌ Not found in {nodes} steps")
    print(f"  Time: {elapsed:.4f}s")

print()

# Test with random 8-bit key
print("═══ TEST 2: Random 8-bit Key ═══")
secret_k2 = random.randint(1, 255)
Q_test2 = scalar_mult(secret_k2, G)
print(f"  Secret: k={secret_k2}")
print()

start = time.time()
found_k2, nodes2, collapsed2, hit2 = fibonacci_spiral_solve(Q_test2, max_steps=2000)
elapsed = time.time() - start

print(f"\n═══ RESULT ═══")
if found_k2 is not None:
    match = found_k2 == secret_k2
    print(f"  {'✅' if match else '⚠️'} Found k={found_k2} (secret={secret_k2})")
    print(f"  Hit: {hit2}")
    print(f"  Nodes: {nodes2}")
    print(f"  Collapses: {collapsed2}")
    print(f"  Time: {elapsed:.4f}s")
else:
    print(f"  ❌ Not found in {nodes2} steps")
    print(f"  Time: {elapsed:.4f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  DOUBLE ATTACK — Fibonacci Spiral ECDLP Complete          ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

