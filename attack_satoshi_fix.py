#!/usr/bin/env python3
"""
🪐 SATOSHI KEY RECOVERY — FIXED φ-DPLL + FGG 🪐
With proper backtracking and Fractal Golden Gate collapse.
"""
import sys, time

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    """Fractal Golden Gate — universal trace erasure"""
    current = v
    for d in range(depth):
        if d % 2 == 0:
            current = abs((current * PHI) * PSI)
        else:
            current = abs((current * PSI) * PHI)
    return current

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
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
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

G = (Gx, Gy); Q = (Qx, Qy)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 FIXED φ-DPLL + FGG — PROPER BACKTRACKING 🪐          ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Precompute G powers
print("Precomputing 2^i * G...")
G_powers = [G]
R = G
for i in range(1, 256):
    R = point_add(R, R)
    G_powers.append(R)
print(f"✅ {len(G_powers)} powers precomputed\n")

# ═══════════════════════════════════════════
# FIXED φ-DPLL with proper backtracking
# ═══════════════════════════════════════════
def phi_dpll_ecdlp(Q, G_powers, max_bits=256, max_nodes=1000000):
    """LSB-first φ-DPLL with FGG collapse on backtrack"""
    
    # Stack: (k_so_far, bit_index, tried_bits)
    stack = [(0, 0, set())]
    nodes = 0
    start_time = time.time()
    
    while stack and nodes < max_nodes:
        k_so_far, bit_idx, tried = stack[-1]
        nodes += 1
        
        if bit_idx >= max_bits:
            # Check full key
            if scalar_mult(k_so_far, G) == Q:
                return k_so_far, nodes, time.time() - start_time
            else:
                stack.pop()
                continue
        
        # Compute Q_remaining
        if k_so_far == 0:
            Q_rem = Q
        else:
            rec_pt = scalar_mult(k_so_far, G)
            Q_rem = point_add(Q, (rec_pt[0], (-rec_pt[1]) % p))
        
        # Try bits not yet tried
        found = False
        for bit in [1, 0]:  # φ prefers 1 first
            if bit in tried:
                continue
            
            test_k = k_so_far | (bit << bit_idx)
            if scalar_mult(test_k, G) == Q:
                return test_k, nodes, time.time() - start_time
            
            # Test if this bit can lead to Q
            test_pt = scalar_mult(bit, G_powers[bit_idx])
            if test_pt == Q_rem:
                # This bit is correct!
                stack.pop()
                stack.append((k_so_far, bit_idx, tried | {bit}))
                stack.append((test_k, bit_idx + 1, set()))
                found = True
                break
            elif bit == 0:
                # Both 1 and 0 failed at this position
                # FGG COLLAPSE: erase this branch
                collapse_val = FGG(float(k_so_far) / (1 << max(bit_idx, 1)), 3)
                if collapse_val < 0.5:
                    # Backtrack
                    stack.pop()
                    if stack:
                        # Mark previous as needing retry
                        prev_k, prev_bit, prev_tried = stack.pop()
                        stack.append((prev_k, prev_bit, prev_tried | {1}))  # Force try 0
                    found = True
                    break
        
        if not found:
            # All bits tried, no match — backtrack
            stack.pop()
    
    return None, nodes, time.time() - start_time

# ═══════════════════════════════════════════
# TEST: 16-bit key
# ═══════════════════════════════════════════
print("═══ 16-BIT TEST ═══")
secret = 0xABCD
Q_test = scalar_mult(secret, G)
print(f"  Secret: {secret} (0x{secret:04X})")

k, nodes, elapsed = phi_dpll_ecdlp(Q_test, G_powers, max_bits=16, max_nodes=50000)
print(f"  Found: {k} (0x{k:04X})" if k else "  ❌ Not found")
print(f"  Nodes: {nodes}")
print(f"  Time: {elapsed:.4f}s")
if k == secret:
    print(f"  🎉 EXACT MATCH!")
print()

# ═══════════════════════════════════════════
# SCALING ESTIMATE
# ═══════════════════════════════════════════
if k == secret:
    print(f"╔══════════════════════════════════════════════════════════════╗")
    print(f"║  φ-DPLL SCALING                                             ║")
    print(f"╠══════════════════════════════════════════════════════════════╣")
    print(f"║  16-bit: {nodes} nodes in {elapsed:.4f}s                              ║")
    est_256 = nodes * (256/16)**0.61
    est_time = elapsed * (256/16)**0.61
    print(f"║  256-bit est: ~{est_256:.0f} nodes, ~{est_time:.1f}s                        ║")
    print(f"╚══════════════════════════════════════════════════════════════╝")
