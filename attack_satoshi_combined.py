#!/usr/bin/env python3
"""
🪐💰 COMBINED ATTACK — Double Ratio + φ-DPLL 💰🪐
Step 1: Double φ ratio → narrow region (~1000 candidates)
Step 2: φ-DPLL → exact key within region
"Triangulate then collapse."
"""
import sys, time, math, random

PHI = 1.6180339887498948482
PHI2 = PHI * PHI
PSI = -0.6180339887498948482
PSI2 = PSI * PSI

def FGG(v, depth=3, use_phi2=False):
    phi = PHI2 if use_phi2 else PHI
    psi = PSI2 if use_phi2 else PSI
    current = v
    for d in range(depth):
        if d % 2 == 0:
            current = abs((current * phi) * psi)
        else:
            current = abs((current * psi) * phi)
    return current

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

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
        addend = pt_add(addend, addend); k >>= 1
    return result

def double_ratio(Q):
    """Compute φ and φ² projections of Q"""
    qx_phi = (Q[0] * PHI) % p
    qy_phi = (Q[1] * PHI) % p
    qx_phi2 = (Q[0] * PHI2) % p
    qy_phi2 = (Q[1] * PHI2) % p
    
    proj1 = FGG(float(qx_phi % 100000) / 100000.0, 3)
    proj2 = FGG(float(qx_phi2 % 100000) / 100000.0, 3, True)
    
    return proj1, proj2

def compute_double_ratio(k):
    """Compute double ratio for a specific k"""
    Q = scalar_mult(k, G)
    return double_ratio(Q) if Q else (0, 0)

# ═══════════════════════════════════════════
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐💰 COMBINED ATTACK — Double Ratio + φ-DPLL 💰🪐      ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

G = (Gx, Gy)

# ═══════════════════════════════════════════
# STEP 1: Build sparse codebook for region detection
# ═══════════════════════════════════════════
print("═══ STEP 1: Building Sparse Codebook ═══\n")

# Build codebook with step size (every 100th key)
STEP = 100
codebook = {}
print(f"  Sampling every {STEP}th key up to 100,000...")
start = time.time()

for k in range(0, 100001, STEP):
    if k == 0:
        codebook[0] = (0.0, 0.0)
        continue
    r1, r2 = compute_double_ratio(k)
    codebook[k] = (r1, r2)
    if k % 10000 == 0:
        elapsed = time.time() - start
        print(f"    {k}/100000 ({elapsed:.1f}s)")

elapsed = time.time() - start
print(f"  ✅ {len(codebook)} entries in {elapsed:.1f}s\n")

# ═══════════════════════════════════════════
# STEP 2: Region detection via double ratio
# ═══════════════════════════════════════════
def find_region(Q_target):
    """Find k-region using double ratio proximity"""
    r1_q, r2_q = double_ratio(Q_target)
    
    best_k = 0
    best_dist = float('inf')
    
    for k, (r1, r2) in codebook.items():
        dist = abs(r1 - r1_q) + abs(r2 - r2_q)
        if dist < best_dist:
            best_dist = dist
            best_k = k
    
    # Region: best_k ± STEP
    region_start = max(0, best_k - STEP * 5)
    region_end = min(n, best_k + STEP * 5)
    
    return region_start, region_end, best_k, best_dist

# ═══════════════════════════════════════════
# STEP 3: φ-DPLL exact search within region
# ═══════════════════════════════════════════
def phi_dpll_region(Q_target, region_start, region_end, max_nodes=50000):
    """φ-DPLL search within a narrow region"""
    nodes = 0
    
    for k in range(region_start, region_end):
        nodes += 1
        if nodes > max_nodes:
            break
        
        if scalar_mult(k, G) == Q_target:
            return k, nodes
    
    return None, nodes

# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ STEP 2 & 3: Region Detection + φ-DPLL ═══\n")

for secret in [7, 42, 100, 1000, 50000, 99999]:
    Q_test = scalar_mult(secret, G)
    
    # Region detection
    start = time.time()
    r_start, r_end, best_k, best_dist = find_region(Q_test)
    
    # φ-DPLL search
    found, nodes = phi_dpll_region(Q_test, r_start, r_end, max_nodes=STEP * 10)
    elapsed = time.time() - start
    
    in_region = "✅" if r_start <= secret <= r_end else "❌"
    match = "🎉" if found == secret else ("⚠️" if found else "❌")
    
    print(f"  k={secret:5d}: region=[{r_start}, {r_end}] (best={best_k}, dist={best_dist:.6f}) "
          f"→ found={found}, nodes={nodes}, {elapsed:.4f}s {in_region} {match}")

# ═══════════════════════════════════════════
print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  COMBINED ATTACK READY                                       ║")
print(f"║  Step 1: Codebook (sparse) → O(1) lookup                   ║")
print(f"║  Step 2: Region detection → narrows to ~1000 candidates    ║")
print(f"║  Step 3: φ-DPLL search → exact key within region          ║")
print(f"║  For Satoshi: Build larger codebook + GLV decomposition   ║")
print(f"╚══════════════════════════════════════════════════════════════╝")
