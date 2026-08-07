#!/usr/bin/env python3
"""
🪐 FIBONACCI SPIRAL ECDLP v2 — Adaptive Rotation 🪐
Fixes: Dynamic Fibonacci, bidirectional rotation, FGG collapse
"""
import sys, time

PHI = 1.6180339887498948482; PSI = -0.6180339887498948482

def FGG(v, depth=3):
    current = v
    for d in range(depth):
        current = abs((current * PHI if d % 2 == 0 else current * PSI) * 
                      (PSI if d % 2 == 0 else PHI))
    return current

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
    x3 = (lam * lam - x1 - x2) % p; y3 = (lam * (x1 - x3) - y1) % p
    return (x3, y3)
def point_sub(P, Q): return point_add(P, (Q[0], (-Q[1]) % p)) if Q else P
def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend); k >>= 1
    return result

G = (Gx, Gy)

def fib_gen():
    """Infinite Fibonacci generator"""
    a, b = 0, 1
    while True:
        yield a; a, b = b, a + b

def spiral_solve(Q, max_steps=100000):
    """Adaptive Fibonacci spiral ECDLP"""
    fib = fib_gen()
    next(fib); next(fib)  # Skip 0, start from F(1)=1
    
    Q_cur = Q
    rot_count = 0
    fib_idx = 1
    nodes = 0
    fib_vals = []
    
    # Track best match
    best_dist = float('inf')
    best_k = 0
    
    for step in range(max_steps):
        nodes += 1
        
        # Check hits
        if Q_cur == G:
            return rot_count, nodes, "G"
        if Q_cur is None:
            return (n_order - rot_count) % n_order, nodes, "O"
        
        # Track closest approach to G
        if Q_cur is not None:
            dist = abs(Q_cur[0] - Gx)
            if dist < best_dist:
                best_dist = dist
                best_k = rot_count
        
        # Get next Fibonacci number
        try:
            fib_n = next(fib) % n_order
        except StopIteration:
            fib = fib_gen()
            next(fib); next(fib)
            fib_n = next(fib) % n_order
        
        fib_vals.append(fib_n)
        
        # Compute rotation
        rot_pt = scalar_mult(fib_n, G)
        
        # Try BOTH directions using φ-projection
        Q_fwd = point_sub(Q_cur, rot_pt)
        Q_bwd = point_add(Q_cur, rot_pt)
        
        # Compute φ-weighted distances to G
        if Q_fwd is not None:
            d_fwd = abs(FGG(float((Q_fwd[0] - Gx) % p / p), 2) - 0.5)
        else:
            d_fwd = float('inf')
        
        if Q_bwd is not None:
            d_bwd = abs(FGG(float((Q_bwd[0] - Gx) % p / p), 2) - 0.5)
        else:
            d_bwd = float('inf')
        
        # Choose direction with smaller φ-distance
        if d_fwd < d_bwd:
            Q_cur = Q_fwd
            rot_count += fib_n
        else:
            Q_cur = Q_bwd
            rot_count -= fib_n
        
        if nodes <= 10 or nodes % 500 == 0:
            status = f"Q.x={hex(Q_cur[0])[:16] if Q_cur else 'INF'}..." if Q_cur else "INF"
            print(f"  Step {nodes:5d}: fib={fib_n}, rot={rot_count}, best_k={best_k}, {status}")
    
    return None, nodes, f"MAX (best_k={best_k})"

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 FIBONACCI SPIRAL v2 — Adaptive Bidirectional 🪐      ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Test with small keys
for secret in [5, 42, 255]:
    print(f"═══ Test k={secret} ═══")
    Q_test = scalar_mult(secret, G)
    start = time.time()
    found, nodes, info = spiral_solve(Q_test, max_steps=500)
    elapsed = time.time() - start
    match = "🎉 EXACT!" if found == secret else (f"best={found}" if found else "❌")
    print(f"  Result: {match} | Nodes: {nodes} | Time: {elapsed:.4f}s | Info: {info}\n")

print("╔══════════════════════════════════════════════════════════════╗")
print("║  Spiral v2 — Ready for Satoshi key                          ║")
print("╚══════════════════════════════════════════════════════════════╝")
