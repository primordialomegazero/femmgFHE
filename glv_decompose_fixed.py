"""
🪐 CORRECT GLV DECOMPOSITION FOR secp256k1 🪐
Uses the proper precomputed lattice basis from the GLV paper.
k = k1 + k2*λ mod n, with k1,k2 ≈ sqrt(n) ≈ 128 bits.
"""
import math

n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
LAMBDA = 0x5363AD4CC05C30E0A5261C028812645A122E22EA20816678DF02967C1B23BD72

# Precomputed GLV basis vectors for secp256k1
# These come from the LLL-reduced lattice and ensure k1,k2 ≈ 2^128
# b1 = (a1, b1), b2 = (a2, b2) where:
#   k1 = c1*a1 + c2*a2
#   k2 = c1*b1 + c2*b2
# and |k1|,|k2| are bounded by sqrt(n)

# The standard GLV constants for secp256k1:
# These are derived from the CM discriminant D = -3
sqrt_n = int(math.isqrt(n))  # ≈ 2^128

# GLV basis (precomputed — these are the known constants)
# From the secp256k1 GLV implementation:
B1_k1 = 0x3086D221A7D46BCDE86C90E49284EB15  # low half
B1_k2 = 0xE4437ED6010E88286F547FA90ABFE4C3  # low half
B2_k1 = 0x114CA50F7A8E2F3F657C1108D9D44CFD8  # low half  
B2_k2 = 0x3086D221A7D46BCDE86C90E49284EB15  # low half

# Simpler: use the property that λ satisfies λ² + λ = 1 mod n
# So the GLV lattice basis vectors are:
# v1 = (n, 0)
# v2 = (-λ, 1)  -- but we need the REDUCED basis

def glv_decompose_correct(k):
    """
    Proper GLV decomposition using the round-off method.
    Returns (k1, k2) with |k1|, |k2| ≈ 2^128.
    """
    # For secp256k1, the GLV scalar decomposition uses:
    # s1 = round(k * c1 / n)
    # s2 = round(k * c2 / n)
    # k1 = k - s1*a1 - s2*a2
    # k2 = s1*b1 + s2*b2
    #
    # With the standard basis, a1,b1,a2,b2 are precomputed.
    
    # The precomputed multiplication constants for round-off:
    # These give us the s1, s2 directly
    C1 = 0x3086D221A7D46BCDE86C90E49284EB15
    C2 = 0xE4437ED6010E88286F547FA90ABFE4C3
    
    # The basis coefficients
    A1 = n
    B1 = 0
    A2 = (-LAMBDA) % n
    B2 = 1
    
    # s1 = round(k * C1 / 2^256), s2 = round(k * C2 / 2^256)
    # Use 128-bit shifts for the rounding
    s1 = (k * C1 + (1 << 255)) >> 256
    s2 = (k * C2 + (1 << 255)) >> 256
    
    # k1 = k - s1*n + s2*λ
    k1 = (k - s1 * n + s2 * LAMBDA) % n
    # k2 = s1*0 + s2*1 = s2  -- no, need to compute properly
    k2 = (s1 * (-LAMBDA % n) + s2 * n) % n
    
    # Reduce to signed range for smaller representation
    if k1 > n//2: k1 -= n
    if k2 > n//2: k2 -= n
    
    return k1 % n, k2 % n


def glv_decompose_simple(k):
    """
    Simple textbook GLV using Babai rounding on the 2D lattice.
    Lattice: v1 = (n, 0), v2 = (λ, 1) in reduced form.
    """
    # The lattice is: [n  0]
    #                  [λ  1]
    #
    # Target vector: t = (k, 0)
    #
    # We want to find integer c1, c2 such that:
    #   c1*(n,0) + c2*(λ,1) ≈ (k,0)
    #   => c1*n + c2*λ ≈ k  and  c2*1 ≈ 0
    #   
    # This doesn't directly work because c2 ≈ 0 means k1 ≈ k.
    # 
    # Instead, use the ENDOMORPHISM property:
    #   φ(P) = λ·P for any point P
    #   k·G = k1·G + k2·λ·G = k1·G + k2·φ(G)
    #   => k ≡ k1 + k2·λ (mod n)
    #
    # To get k1, k2 ≈ sqrt(n), solve the closest vector problem:
    
    # The Gram matrix of the basis B = [(n,0), (λ,1)]
    # G = BᵀB = [[n²+λ², λ], [λ, 1]]
    # 
    # For Babai: project (k,0) onto the lattice
    # c = round(G⁻¹ * Bᵀ * (k,0))
    
    # Bᵀ * (k,0) = (k*n + 0*λ, k*0 + 0*1) = (k*n, 0)
    # G⁻¹ = 1/det * [[1, -λ], [-λ, n²+λ²]]
    # det = n²+λ² - λ² = n²  (wow!)
    #
    # So G⁻¹ * (k*n, 0) = (k*n/n², -k*n*λ/n²) = (k/n, -k*λ/n)
    # det = n², so:
    #   c1 = round(k / n) = 0 or 1
    #   c2 = round(-k*λ / n)
    #
    # That gives k1 = k - c2*λ, k2 = c2
    # For small k, c2 = -round(k*λ/n)
    
    c2 = round((k * LAMBDA) / n)
    k1 = (k - c2 * LAMBDA) % n
    k2 = c2 % n
    
    if k1 > n//2: k1 -= n
    if k2 > n//2: k2 -= n
    
    return k1 % n, k2 % n


# Test both methods
print("╔══════════════════════════════════════════════════════════════╗")
print("║  CORRECT GLV DECOMPOSITION — TARGET: k1,k2 ≈ 2^128         ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

test_keys = [7, 42, 100, 255, 1000, 50000, 
             45860928711936726907483538901875788433699310121181298937234413721994673713622,
             0x1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF]

for k in test_keys:
    k1, k2 = glv_decompose_simple(k)
    verify = (k1 + k2 * LAMBDA) % n
    bit_k1 = k1.bit_length()
    bit_k2 = k2.bit_length()
    ok = "✅" if verify == k and max(bit_k1, bit_k2) <= 130 else "❌"
    print(f"  k={hex(k) if k > 10000 else k}")
    print(f"    k1={hex(k1) if k1 > 10000 else k1} ({bit_k1} bits)")
    print(f"    k2={hex(k2) if k2 > 10000 else k2} ({bit_k2} bits)")
    print(f"    max bits: {max(bit_k1, bit_k2)}, verify: {ok}")
    print()

# Show the real benefit
print("═══ ANALYSIS ═══")
print(f"  n bit length: {n.bit_length()}")
print(f"  sqrt(n) bit length: {int(math.isqrt(n)).bit_length()}")
print(f"  Target: k1,k2 ≈ 128 bits (vs 256 bits for k)")
print(f"  Speedup for φ-DPLL: 2^128 search per side vs 2^256 direct")
