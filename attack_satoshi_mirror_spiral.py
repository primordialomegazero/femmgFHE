#!/usr/bin/env python3
"""
🪐 MIRROR SPIRAL ECDLP 🪐
φ·ψ = -1 as the MIRROR BRIDGE
Between elliptic curve and Fibonacci lattice
"Mirror, rotate, collapse."
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
# MIRROR: φ·ψ = -1 is the bridge
# ═══════════════════════════════════════════
# The mirror projects elliptic curve points
# onto the Fibonacci lattice and back.

def mirror_to_lattice(point, n):
    """
    MIRROR: Project elliptic curve point onto Fibonacci lattice.
    Uses φ·ψ = -1 as the reflection property.
    """
    if point is None:
        return 0  # Point at infinity → lattice origin
    
    x, y = point
    
    # Mirror transformation:
    # lattice_pos = (x * φ + y * ψ) mod F(n)
    # This maps the 2D curve point to a 1D Fibonacci lattice position
    fib_n = fib_dynamic(n)
    if fib_n == 0:
        fib_n = 1
    
    lattice_pos = int((x * PHI + y * abs(PSI)) % fib_n)
    return lattice_pos


def mirror_from_lattice(lattice_pos, n):
    """
    REVERSE MIRROR: Project Fibonacci lattice position back to curve.
    Maps lattice position to a multiple of G.
    """
    # The lattice position corresponds to a scalar multiple of G
    # k = lattice_pos * φ (mod n_order)
    scalar = int(lattice_pos * PHI) % n_order
    return scalar


def fib_dynamic(n):
    """Generate Fibonacci numbers dynamically"""
    if n <= 0:
        return 0
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b


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
print("║  🪐 MIRROR SPIRAL ECDLP — φ·ψ = -1 Bridge 🪐            ║")
print("║  'Mirror, rotate, collapse.'                             ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

print(f"  Mirror constant: φ·ψ = {PHI * PSI:.10f} (should be -1)")
print(f"  FGG test: FGG(0.5, 3) = {FGG(0.5, 3):.6f} (should be 0.5)")
print()

# ═══════════════════════════════════════════
# MIRROR SPIRAL SOLVER
# ═══════════════════════════════════════════

def mirror_spiral_solve(Q_target, max_steps=100):
    """
    MIRROR SPIRAL ECDLP SOLVER
    
    1. MIRROR: Project Q to Fibonacci lattice
    2. ROTATE: Walk the lattice using Fibonacci steps
    3. MIRROR BACK: Check if we hit G
    4. COLLAPSE: FGG at each step
    """
    best_k = 0
    best_dist = float('inf')
    nodes = 0
    collapses = 0
    
    # Mirror Q to lattice
    q_lattice = mirror_to_lattice(Q_target, 1)
    g_lattice = mirror_to_lattice(G, 1)  # Generator's lattice position
    
    print(f"  Q lattice: {q_lattice}")
    print(f"  G lattice: {g_lattice}")
    print()
    
    # Walk the lattice
    current_lattice = q_lattice
    rotation = 0
    
    for step in range(1, max_steps + 1):
        nodes += 1
        
        # Get Fibonacci step
        fib_n = fib_dynamic(step)
        
        # Determine direction: which way moves us closer to G?
        # Try both forward and backward
        forward = (current_lattice + fib_n) % max(1, fib_dynamic(step + 1))
        backward = (current_lattice - fib_n) % max(1, fib_dynamic(step + 1))
        
        dist_forward = abs(forward - g_lattice)
        dist_backward = abs(backward - g_lattice)
        
        if dist_forward < dist_backward:
            direction = 1
            current_lattice = forward
        else:
            direction = -1
            current_lattice = backward
        
        rotation += direction * fib_n
        
        # Mirror back to curve: check what scalar this corresponds to
        candidate_k = mirror_from_lattice(current_lattice, step + 1)
        
        # Verify on curve
        R = scalar_mult(candidate_k, G)
        
        if R == Q_target:
            print(f"\n  🎉 FOUND! k = {candidate_k}")
            print(f"  Steps: {step}")
            print(f"  Nodes: {nodes}")
            print(f"  Collapses: {collapses}")
            return candidate_k, nodes, collapses, "FOUND"
        
        # Track best
        if R is not None:
            dist = abs(R[0] - Q_target[0])
            if dist < best_dist:
                best_dist = dist
                best_k = candidate_k
        
        # COLLAPSE
        v = step / max_steps
        FGG(v, 3)
        collapses += 1
        
        # Progress
        if step <= 10 or step % 25 == 0:
            print(f"  Step {step:3d}: fib={fib_n}, dir={direction:+d}, "
                  f"rot={rotation}, lattice={current_lattice}, best_k={best_k}")
    
    return None, nodes, collapses, f"MAX (best_k={best_k})"


# ═══════════════════════════════════════════
# TEST: Small keys muna
# ═══════════════════════════════════════════

print("═══ TEST 1: k=3 ═══")
secret = 3
Q_test = scalar_mult(secret, G)
print(f"  Secret: k={secret}")
print(f"  Q = ({hex(Q_test[0])[:20]}...)")
print()

start = time.time()
found, nodes, collapses, info = mirror_spiral_solve(Q_test, max_steps=50)
elapsed = time.time() - start

if found is not None:
    print(f"\n  {'✅' if found == secret else '⚠️'} Found k={found} (secret={secret})")
else:
    print(f"\n  ❌ {info}")
print(f"  Time: {elapsed:.4f}s")
print()

print("═══ TEST 2: k=7 ═══")
secret2 = 7
Q_test2 = scalar_mult(secret2, G)
print(f"  Secret: k={secret2}")
print()

start = time.time()
found2, nodes2, collapses2, info2 = mirror_spiral_solve(Q_test2, max_steps=50)
elapsed = time.time() - start

if found2 is not None:
    print(f"\n  {'✅' if found2 == secret2 else '⚠️'} Found k={found2} (secret={secret2})")
else:
    print(f"\n  ❌ {info2}")
print(f"  Time: {elapsed:.4f}s")
print()

print("═══ TEST 3: k=42 ═══")
secret3 = 42
Q_test3 = scalar_mult(secret3, G)
print(f"  Secret: k={secret3}")
print()

start = time.time()
found3, nodes3, collapses3, info3 = mirror_spiral_solve(Q_test3, max_steps=50)
elapsed = time.time() - start

if found3 is not None:
    print(f"\n  {'✅' if found3 == secret3 else '⚠️'} Found k={found3} (secret={secret3})")
else:
    print(f"\n  ❌ {info3}")
print(f"  Time: {elapsed:.4f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  MIRROR SPIRAL — Keep testing! 🪞🌀                       ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

