"""
🪐 DISTANCE-WEIGHTED INTERPOLATION 🪐
σ⁻¹(k_cand) ≈ weighted average of nearest fixed points
"""
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

k_cand_real = 0xbad6a4f369927ac6d4780ab2882bdc7bbe4b4a982e7496fac3c879c3d2b9ddf3
k_cand_imag = 0x848df4a3c429622c14fc575a752763fa371e4e29e5a5c3b8a3f1d4e7c6b5a4c3

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

def legendre(a, p):
    return pow(a, (p-1)//2, p)

def sqrt_mod(a, p):
    if legendre(a, p) != 1:
        return None
    return pow(a, (p+1)//4, p)

def cube_roots(a, p):
    s, t = 0, p-1
    while t % 3 == 0:
        s += 1
        t //= 3
    g = 2
    while pow(g, (p-1)//3, p) == 1:
        g += 1
    if s == 1:
        r = pow(a, (2*t + 1)//3, p)
        omega = pow(g, (p-1)//3, p)
        return [r, (r*omega)%p, (r*omega*omega)%p]
    return [pow(a, (2*p-1)//3, p)]

# Get homomorphic x-values (roots of x³ ≡ -16)
target = (p - 16) % p
roots = cube_roots(target, p)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 DISTANCE-WEIGHTED k ESTIMATION 🪐                  ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# The fixed points have σ(k) = (k, 0)
# Our target: (k_cand_real, k_cand_imag)
# The "distance" in F_p² between target and fixed point i:
# d_i = sqrt((k_cand_real - k_i)^2 + (k_cand_imag - 0)^2)

# For each homomorphic x-value, try as potential k_i
print("  Homomorphic x-values (candidates for fixed point k values):\n")

candidate_ks = []
for x in roots:
    # x could be the k value (discrete log) of the homomorphic point!
    # Or it could be related to k via some function
    candidate_ks.append(x)
    print(f"    k_candidate_fixed = {hex(x)[:40]}...")

# Add known fixed points
candidate_ks.append(1)      # k=1 is always fixed (σ(1)=1)
candidate_ks.append(n-1)    # k=n-1 is probably fixed (inverse of 1)

print(f"\n  Total candidate fixed points: {len(candidate_ks)}")
print(f"\n  Computing distance-weighted k:\n")

# For each candidate k_i, the weight is 1/distance
# k_estimated = Σ(w_i · k_i) / Σ(w_i)

total_weight = 0
weighted_sum = 0

for k_i in candidate_ks:
    # Distance in F_p²
    dist_real = (k_cand_real - k_i) % p
    dist_imag = k_cand_imag  # since fixed points have imag=0
    
    # Use Euclidean-like distance (mod p is tricky)
    # Use: d = abs(dist_real) + abs(dist_imag) as proxy
    d = (abs(dist_real) if dist_real < p//2 else p - dist_real) + \
        (abs(dist_imag) if dist_imag < p//2 else p - dist_imag)
    
    if d == 0:
        print(f"  EXACT MATCH! k = {hex(k_i)}")
        weight = float('inf')
    else:
        weight = 1.0 / d
    
    total_weight += weight
    weighted_sum += weight * k_i

if total_weight > 0:
    k_estimated = int(weighted_sum / total_weight) % n
    print(f"  Distance-weighted k = {hex(k_estimated)[:40]}...")
    print(f"\n  Testing this k...")
    
    # Verify
    def pt_add(P, Q):
        if P is None: return Q
        if Q is None: return P
        x1, y1 = P; x2, y2 = Q
        if x1 == x2:
            if (y1 + y2) % p == 0: return None
            lam = (3*x1*x1) * modinv(2*y1, p) % p
        else:
            lam = ((y2-y1) * modinv(x2-x1, p)) % p
        x3 = (lam*lam - x1 - x2) % p
        y3 = (lam*(x1 - x3) - y1) % p
        return (x3, y3)
    
    def scalar_mult(k, P):
        if k == 0: return None
        result = None; addend = P
        while k:
            if k & 1: result = pt_add(result, addend)
            addend = pt_add(addend, addend)
            k >>= 1
        return result
    
    G = (Gx, Gy)
    Q_test = scalar_mult(k_estimated, G)
    if Q_test and Q_test[0] == Qx and Q_test[1] == Qy:
        print(f"  🎉🎉🎉 CORRECT!!! 🎉🎉🎉")
        print(f"  SATOSHI'S KEY: {hex(k_estimated)}")
    else:
        print(f"  ❌ Not the key. Close but no cigar!")
        print(f"  Need exact σ⁻¹ mapping, not approximation.")
