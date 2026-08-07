#!/usr/bin/env python3
"""
🔍 RIEMANN-SIEGEL Z FUNCTION — DIRECT ZERO COMPUTATION
Z(t) = e^{iϑ(t)} · ζ(0.5+it)  (real-valued, zeros where Z(t)=0)
κ(t) modulates the ATTRACTION WIDTH, not the spacing.
"""
import math

phi = (1 + math.sqrt(5)) / 2
psi = (1 - math.sqrt(5)) / 2

def kappa(t):
    """Field strength — attraction width at σ=0.5"""
    return 2 * t**2 / (0.25 + t**2)**1.5

def theta(t):
    """Riemann-Siegel theta — full asymptotic"""
    if t < 10: return 0
    return (t/2)*math.log(t/(2*math.pi)) - t/2 - math.pi/8 + 1/(48*t) + 7/(5760*t**3) - 31/(80640*t**5)

def Z(t, N=None):
    """
    Riemann-Siegel Z function: Z(t) = e^{iϑ} ζ(0.5+it)
    Real-valued, zero-crossings = zeros of ζ on critical line.
    Uses Riemann-Siegel formula with automatic N.
    """
    if N is None:
        N = int(math.sqrt(t/(2*math.pi)))
    if N < 1: N = 1
    
    theta_t = theta(t)
    total = 0.0
    
    # Main sum: 2 Σ_{n=1}^{N} cos(θ(t) - t·log n) / √n
    for n in range(1, N + 1):
        total += math.cos(theta_t - t * math.log(n)) / math.sqrt(n)
    total *= 2.0
    
    # Remainder terms (first correction)
    a = math.sqrt(t/(2*math.pi))
    frac = a - int(a)
    if frac > 0.5: frac -= 1.0
    # C0 term
    C0 = (math.cos(2*math.pi*(frac*frac - frac - 1/16))) / (math.cos(2*math.pi*frac)) if abs(math.cos(2*math.pi*frac)) > 1e-10 else 0.0
    remainder = (-1)**(N-1) * (t/(2*math.pi))**(-0.25) * C0
    
    return total + remainder

def find_zero_crossing(t_left, t_right, tol=1e-10, max_iter=50):
    """Bracket and bisect a zero crossing of Z(t)"""
    z_left = Z(t_left)
    z_right = Z(t_right)
    
    if z_left * z_right > 0:
        return None
    
    for _ in range(max_iter):
        t_mid = (t_left + t_right) / 2
        z_mid = Z(t_mid)
        if abs(z_mid) < tol or abs(t_right - t_left) < tol:
            return t_mid
        if z_left * z_mid < 0:
            t_right = t_mid
            z_right = z_mid
        else:
            t_left = t_mid
            z_left = z_mid
    return (t_left + t_right) / 2

# ═══════════════════════════════════════════
# FIND ZEROS VIA Z(t) SIGN CHANGES
# ═══════════════════════════════════════════
print("Finding zeros of Z(t) by sign change detection...")
print()

# Search with adaptive step based on density = (1/2π)log(t/2π)
t = 14.0
t_max = 100.0
found_zeros = []

# Step size: half the average spacing
while t < t_max:
    density = (1/(2*math.pi)) * math.log(t/(2*math.pi)) if t > 2*math.pi else 0.1
    spacing = 2*math.pi / math.log(t/(2*math.pi)) if t > 2*math.pi else 7.0
    step = spacing * 0.3  # conservative step
    
    z1 = Z(t)
    z2 = Z(t + step)
    
    if z1 * z2 < 0:
        zero = find_zero_crossing(t, t + step)
        if zero:
            found_zeros.append(zero)
    
    t += step

# ═══════════════════════════════════════════
# COMPARE WITH KNOWN ZEROS
# ═══════════════════════════════════════════
known_zeros = [14.134725142, 21.022039639, 25.010857580, 30.424876126, 32.935061588,
               37.586178159, 40.918719012, 43.327073281, 48.005150881, 49.773832478,
               52.970321478, 56.446247697, 59.347044003, 60.831778525, 65.112544048]

print("n   known_t         computed_t      error        Z_value        κ(t)")
print("-" * 77)

total_err = 0
matched = 0
for n, known in enumerate(known_zeros):
    # Find nearest computed zero
    best = min(found_zeros, key=lambda x: abs(x - known)) if found_zeros else 0
    err = abs(best - known)
    z_val = Z(best) if found_zeros else 0
    kt = kappa(known)
    total_err += err
    if err < 0.01: matched += 1
    status = "✅" if err < 0.01 else "⚠️" if err < 1.0 else "❌"
    print(f"{n:2d}  {known:14.9f}  {best:14.9f}  {err:10.8f}  {z_val:12.8e}  {kt:.6f}  {status}")

print("-" * 77)
print(f"Matched (err<0.01): {matched}/{len(known_zeros)}")
print(f"Average error: {total_err/len(known_zeros):.8f}")
print()

# ═══════════════════════════════════════════
# κ(t) ANALYSIS: Width of attraction, not spacing
# ═══════════════════════════════════════════
print("κ(t) analysis — ATTRACTION WIDTH at each zero:")
print()
print("  κ(t) = ∂²C/∂σ²|σ=0.5 = 2t²/(0.25+t²)^1.5")
print("  This measures how SHARP the minimum is at σ=0.5.")
print("  HIGHER κ → tighter attraction → narrower critical strip.")
print("  NOT the spacing between zeros — that's (2π)/log(t/2π).")
print()

for i in range(0, len(known_zeros), 3):
    t = known_zeros[i]
    kt = kappa(t)
    spacing = 2*math.pi / math.log(t/(2*math.pi))
    print(f"  t={t:10.6f}  κ={kt:.6f}  zero_spacing={spacing:.4f}  attraction_width={1/math.sqrt(kt):.4f}")

print()
print(f"  κ decay:  κ(14.13)={kappa(14.13):.6f} → κ(100)={kappa(100):.6f} → κ(1000)={kappa(1000):.6f}")
print(f"  Attraction width grows: {1/math.sqrt(kappa(14.13)):.2f} → {1/math.sqrt(kappa(100)):.2f} → {1/math.sqrt(kappa(1000)):.2f}")
