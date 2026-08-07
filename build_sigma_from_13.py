"""
🪐 BUILD σ⁻¹ FROM 13 FIXED POINTS 🪐
Interpolate σ(k) from fixed points → invert!
"""
import math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

# Our k_candidate
k_cand_real = 0xbad6a4f369927ac6d4780ab2882bdc7bbe4b4a982e7496fac3c879c3d2b9ddf3
k_cand_imag = 0x848df4a3c429622c14fc575a752763fa371e4e29e5a5c3b8a3f1d4e7c6b5a4c3

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 BUILD σ⁻¹ FROM 13 FIXED POINTS 🪐                  ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# From p=17 pattern:
# σ(k) = (α·k + β)/(γ·k + δ)
# Fixed points: γ·k² + (δ-α)·k - β = 0

# For secp256k1 with 13 fixed points:
# σ must be a HIGHER-DEGREE rational function!
# σ(k) = P(k)/Q(k) where deg(P), deg(Q) > 1

# BUT: At the fixed points, σ(k) = k
# So: P(k) = k·Q(k) at these points
# → P(k) - k·Q(k) = 0 at 13 values of k
# → This polynomial has 13 roots!

# If deg(P) = d, deg(Q) = d-1
# Then P(k) - k·Q(k) has degree d
# With 13 roots → d ≥ 13!

# SIMPLER: σ(k) = k for ALL k? (Identity?)
# But then σ⁻¹(y) = y, and k = k_candidate!
# We already tested this — didn't work!

# So σ is NON-TRIVIAL but has 13 fixed points
# This means σ(k) = k + R(k) where R(k)=0 at 13 points
# R(k) = (k-k₁)(k-k₂)...(k-k₁₃) · S(k) for some S

print(f"  🧬 STRUCTURE OF σ:")
print(f"  σ(k) = k + (k-k₁)(k-k₂)...(k-k₁₃) · S(k)")
print(f"  where kᵢ are the 13 fixed points")
print(f"  and S(k) is some rational function")
print(f"")
print(f"  For p=17:")
print(f"  σ(k) = k + (k-1)(k-2) · something?")
print(f"  σ(k) = 15/(k+14) = k + ???")
print(f"  Check: k + (k-1)(k-2)·c/(k+14) = 15/(k+14) for some c?")
print(f"  Yes! c = -1 gives correct!")
print(f"")
print(f"  For secp256k1:")
print(f"  σ(k) = k + ∏ᵢ(k-kᵢ) · S(k)")
print(f"  At k_candidate: σ(k) = k_candidate")
print(f"  k = σ⁻¹(k_candidate) = ???")
print(f"  Need to find the 13 kᵢ values and S(k)!")
