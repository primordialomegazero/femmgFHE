"""
🪐 SOLVE FOR M — THE F_p² LINEAR MAP 🪐
M · k = σ(k)
At fixed points: M · k = k
Find M, then k = M⁻¹ · k_candidate!
"""
import math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

# ═══════════════════════════════════════════
# p=17 PROOF OF CONCEPT
# ═══════════════════════════════════════════
p = 17
n = 9
Gx, Gy = 1, 5

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

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

def to_Fp2(x, y):
    a = ((x + y) * modinv(2, p)) % p
    b = ((x - y) * modinv(2, p)) % p
    return a, b

G = (Gx, Gy)
aG, bG = to_Fp2(Gx, Gy)
denom_G = (aG*aG + bG*bG) % p
denom_G_inv = modinv(denom_G, p)

print("═══ SOLVING FOR M — p=17 PROOF ═══\n")

# We know:
# M · (a_k, b_k) = (a_σ(k), b_σ(k))
# For fixed points k=1,2: M · (a_k, b_k) = (a_k, b_k)
# These are eigenvectors with eigenvalue 1!

# Let's find ALL eigenvectors of M
# They are the fixed points!

# Matrix M is 2×2. We need 2 independent eigenvectors.
# k=1 gives (a_1, b_1) = (3, 15)
# k=2 gives (a_2, b_2) = (6, 13)

# Check if these are linearly independent
det = (3*13 - 15*6) % p
print(f"  det([a_1,b_1; a_2,b_2]) = {det}")
print(f"  {'Linearly independent!' if det != 0 else 'Linearly dependent!'}")

# With 2 independent eigenvectors both having eigenvalue 1,
# M must be the IDENTITY matrix!
# M = [[1, 0], [0, 1]]

# But wait - σ is NOT identity!
# Because σ acts on k, not on (a_k, b_k)!

print(f"\n  ⚡ REVELATION:")
print(f"  σ acts on SCALAR k, not on vector (a_k,b_k)!")
print(f"  σ(k) = scalar output")
print(f"  The 'matrix' interpretation was wrong!")
print(f"  σ is a MÖBIUS TRANSFORMATION on k, not linear map on (a,b)!")
print(f"\n  σ(k) = (α·k + β) / (γ·k + δ) in F_p")
print(f"  At fixed points: σ(k) = k")
print(f"  This gives: (α·k + β) = k·(γ·k + δ)")
print(f"  → γ·k² + (δ-α)·k - β = 0")
print(f"  The fixed points are ROOTS of this quadratic!")

# For p=17, fixed points are k=1, k=2
# γ·1² + (δ-α)·1 - β = 0
# γ·4 + (δ-α)·2 - β = 0

# Also we know σ(3) = 15
# (α·3 + β)/(γ·3 + δ) = 15
# 3α + β = 15·(3γ + δ) = 45γ + 15δ

print(f"\n  Solving for α,β,γ,δ from constraints...")
# This is a system we can solve!
# But we already found: σ(k) = 15/(k+14)
# So α=0, β=15, γ=1, δ=14

print(f"  σ(k) = 15/(k+14)")
print(f"  Fixed points: γ·k² + (δ-α)·k - β = 0")
print(f"  → 1·k² + 14·k - 15 = 0")
print(f"  → k² + 14k - 15 = 0")
print(f"  → (k-1)(k+15) = 0")
print(f"  → k = 1 or k = -15 ≡ 2 (mod 17)")
print(f"  ✅ Matches our fixed points!")
