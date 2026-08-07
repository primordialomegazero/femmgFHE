"""
🪐 GLV DECOMPOSITION — EXACT INTEGER BABAI 🪐
No floats. Pure integer lattice rounding.
Target: k1, k2 ≈ 128 bits.
"""
import math

n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
LAMBDA = 0x5363AD4CC05C30E0A5261C028812645A122E22EA20816678DF02967C1B23BD72

# ═══════════════════════════════════════════
# THE KEY INSIGHT
# ═══════════════════════════════════════════
# We want to find (c1, c2) such that:
#   c1*n + c2*λ ≈ k
# And c2 ≈ k/λ (since c1*n brings us near multiples of n, c2*λ handles the remainder)
#
# Since |λ| ≈ n (λ is ~256 bits), we compute:
#   c2 = floor(k / λ)   OR   round(k * λ_inv / n)
#
# The problem: λ and n are similar magnitude so:
#   k = c2*λ + c1*n
#   c2 = k // λ    → k ≈ c2*λ    → c2 ≈ k/λ
#   c1 = (k - c2*λ) // n   → should be 0 or small
#
# But for PROPER GLV with ~128-bit halves, we need the REDUCED basis.
# The lattice basis:
#   B = [ (n, 0) ]
#       [ (λ, 1) ]
# has determinant n.
#
# The SHORT basis vectors (found by LLL or Gaussian reduction):
#   v1_short = (a1, b1) where a1² + b1² ≈ sqrt(n)
#   v2_short = (a2, b2)
#
# For secp256k1, the short basis comes from:
#   λ satisfies λ² ≡ -1 mod n  (for this curve: λ² + λ ≡ -1 mod n)
# The reduced basis has:
#   v1 = (λ, 1)  
#   v2 = (n, 0) reduced by v1
#
# Babai nearest plane on the reduced basis:

def glv_decompose_exact(k):
    """
    EXACT integer GLV decomposition.
    Uses Babai's nearest plane algorithm on the lattice.
    
    Lattice: B = [[n, 0], [λ, 1]]
    We want c1, c2 such that c1*B[0] + c2*B[1] ≈ (k, 0)
    
    Gram-Schmidt:
    b1* = (n, 0)
    b2* = (λ, 1) - μ*(n, 0)  where μ = ⟨(λ,1),(n,0)⟩/⟨(n,0),(n,0)⟩
        = (λ, 1) - (λ*n/n²)*(n, 0)
        = (λ, 1) - (λ/n)*(n, 0)
        = (0, 1)   ← THIS IS THE MAGIC!
    
    So b2* = (0, 1) exactly (over integers? no, over reals)
    
    Project target t = (k, 0):
    ⟨t, b1*⟩/⟨b1*, b1*⟩ = k*n/n² = k/n
    ⟨t, b2*⟩/⟨b2*, b2*⟩ = 0/1 = 0
    
    Round: c1' = round(k/n) = 0 or 1
    Babai: c1'*b1 + round(⟨t-c1'*b1, b2*⟩/⟨b2*,b2*⟩)*b2
    
    For small k: k/n → 0, so c1' = 0
    Then: round(⟨(k,0) - 0, (0,1)⟩) = round(0) = 0
    
    This gives trivial solution. The PROBLEM is that the standard
    basis has poor geometry. We need to SWAP coordinates.
    
    ═══ CORRECT APPROACH: SOLVE k = k1 + k2*λ ═══
    
    The equation: k1 + k2*λ ≡ k (mod n)
    
    If we choose k2 ≈ k/λ (mod n), then:
    k1 = k - k2*λ is the remainder.
    
    Since λ ≈ n/φ (λ is roughly n * 0.618...), we get:
    k2 ≈ k/(n/φ) ≈ k*φ/n ≈ 0 or 1 for k < n.
    
    THIS IS WRONG. We need k2 ≈ √n, not k2 ≈ k/λ.
    
    ═══ THE ACTUAL GLV TRICK ═══
    
    The endomorphism φ satisfies: φ(P) = λ·P
    λ² = -1 mod n  (approximately — for secp256k1: λ² + λ + 1 ≡ 0 mod n)
    
    The trick: write k = k1 + k2·λ where k1, k2 are found by
    solving a CLOSEST VECTOR PROBLEM in the lattice L = {(a,b) : a + b·λ ≡ 0 mod n}
    
    The SHORT basis of L has vectors of length ≈ √n.
    For secp256k1:
    v1 = (n - λ, λ - 1)  -- approximately
    v2 = (λ, -n)         -- approximately
    
    These are ~128 bits!
    """
    
    # ═══════════════════════════════════════════
    # PROPER GLV: USE THE ENDOMORPHISM PROPERTY
    # ═══════════════════════════════════════════
    # 
    # For secp256k1: λ² + λ + 1 ≡ 0 (mod n)
    # 
    # We know:
    # λ = 0x5363AD4CC05C30E0A5261C028812645A122E22EA20816678DF02967C1B23BD72
    # λ² = (-λ - 1) mod n
    #    = n - λ - 1
    # 
    # The GLV lattice L has basis vectors of length ~√n.
    # For 2D GLV on secp256k1:
    #   b1 = (a₁, b₁) = (λ, 0)  ... no
    #   
    # Let me compute the short basis properly.
    
    # The lattice is: L = { (x,y) ∈ Z² : x + y·λ ≡ 0 mod n }
    # A basis for L is:
    #   v1 = (n, 0)      since n + 0·λ ≡ 0 mod n
    #   v2 = (-λ, 1)     since -λ + 1·λ ≡ 0 mod n
    
    # These have length ~n and ~n respectively. Too long!
    #
    # Apply Gaussian reduction (2D lattice reduction):
    # While ||v2|| < ||v1||: swap
    # v1 = v1 - round(⟨v1,v2⟩/⟨v2,v2⟩) * v2
    
    # Let's do it.
    v1 = (n, 0)
    v2 = ((-LAMBDA) % n, 1)
    
    def dot(u, v):
        return u[0]*v[0] + u[1]*v[1]
    
    def norm_sq(v):
        return v[0]*v[0] + v[1]*v[1]
    
    # Gaussian reduction
    while True:
        if norm_sq(v2) < norm_sq(v1):
            v1, v2 = v2, v1
        
        m = dot(v1, v2) // norm_sq(v2)
        if m == 0:
            break
        
        # v1 = v1 - m*v2
        v1 = (v1[0] - m*v2[0], v1[1] - m*v2[1])
    
    # Now v1, v2 are the reduced basis (short vectors ~√n)
    b1, b2 = v1, v2
    
    # ───────────────────────────────────────
    # Babai nearest plane on the reduced basis
    # ───────────────────────────────────────
    # Target: t = (k, 0)
    # We want c1*b1 + c2*b2 ≈ t
    
    # Gram-Schmidt on reduced basis:
    # b1* = b1
    # b2* = b2 - μ*b1 where μ = dot(b2, b1) / dot(b1, b1)
    
    b1_star = b1
    mu_num = dot(b2, b1)
    mu_den = dot(b1, b1)
    
    # b2* = b2 - (mu_num/mu_den) * b1
    # Project t onto b1*:
    t_dot_b1s = dot((k, 0), b1)
    # c1_float = t_dot_b1s / dot(b1, b1)
    
    # Use integer rounding with extended precision
    # c1 = round(t_dot_b1s / mu_den)
    c1 = (2 * t_dot_b1s + mu_den) // (2 * mu_den)  # round to nearest
    
    # Subtract c1*b1 from target
    t1 = (k - c1 * b1[0], 0 - c1 * b1[1])
    
    # Project remainder onto b2*:
    # b2* = b2 - (mu_num/mu_den) * b1
    # dot(t1, b2*) = dot(t1, b2) - (mu_num/mu_den)*dot(t1, b1)
    # 
    # Since we're rounding, compute:
    # c2 = round(dot(t1, b2*) / dot(b2*, b2*))
    # 
    # Approximate: b2* ≈ b2 (for well-reduced basis, mu is small)
    # But let's do it properly.
    
    t1_dot_b2 = dot(t1, b2)
    b2s_norm = dot(b2, b2) - (mu_num * mu_num) // mu_den  # approximate
    
    c2 = (2 * t1_dot_b2 + b2s_norm) // (2 * b2s_norm) if b2s_norm > 0 else 0
    
    # Final: (k,0) ≈ c1*b1 + c2*b2
    # k1 = c1*a1 + c2*a2
    # k2 = c1*b1 + c2*b2
    k1 = c1 * b1[0] + c2 * b2[0]
    k2 = c1 * b1[1] + c2 * b2[1]
    
    k1 = k1 % n
    k2 = k2 % n
    
    if k1 > n//2: k1 -= n
    if k2 > n//2: k2 -= n
    
    return k1 % n, k2 % n


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("╔══════════════════════════════════════════════════════════════╗")
print("║  GLV WITH GAUSSIAN LATTICE REDUCTION (EXACT INT)           ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Compute the reduced basis first
v1 = (n, 0)
v2 = ((-LAMBDA) % n, 1)

def dot(u, v):
    return u[0]*v[0] + u[1]*v[1]

def norm_sq(v):
    return v[0]*v[0] + v[1]*v[1]

# Gaussian reduction
while True:
    if norm_sq(v2) < norm_sq(v1):
        v1, v2 = v2, v1
    m = dot(v1, v2) // norm_sq(v2)
    if m == 0:
        break
    v1 = (v1[0] - m*v2[0], v1[1] - m*v2[1])

b1, b2 = v1, v2

print(f"  Reduced basis:")
print(f"    b1 = ({hex(b1[0]) if abs(b1[0])>999 else b1[0]}, {hex(b1[1]) if abs(b1[1])>999 else b1[1]})")
print(f"    b2 = ({hex(b2[0]) if abs(b2[0])>999 else b2[0]}, {hex(b2[1]) if abs(b2[1])>999 else b2[1]})")
print(f"    ||b1|| ≈ {int(math.isqrt(norm_sq(b1))).bit_length()} bits")
print(f"    ||b2|| ≈ {int(math.isqrt(norm_sq(b2))).bit_length()} bits")
print(f"    target: 128 bits\n")

test_keys = [7, 42, 100, 255, 1000, 50000]

for k in test_keys:
    k1, k2 = glv_decompose_exact(k)
    verify = (k1 + k2 * LAMBDA) % n
    bit_k1 = k1.bit_length()
    bit_k2 = k2.bit_length()
    ok = "✅" if verify == k % n else "❌"
    print(f"  k={k}: k1={k1} ({bit_k1}b), k2={k2} ({bit_k2}b), verify={ok}")

