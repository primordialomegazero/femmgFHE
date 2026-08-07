#!/usr/bin/env python3
"""
🔍 RIEMANN ZERO COMPUTATION — DIRECT NUMERICAL SOLVER
Refined Bridge: κ(t) → corrected theta → precise zero locations
Pure computation, minimal output.
"""
import math

phi = (1 + math.sqrt(5)) / 2
psi = (1 - math.sqrt(5)) / 2

def kappa(t):
    """Field strength at height t"""
    return 2 * t**2 / (0.25 + t**2)**1.5

def theta_standard(t):
    """Riemann-Siegel theta (simplified asymptotic)"""
    if t < 10: return 0
    return (t/2)*math.log(t/(2*math.pi)) - t/2 - math.pi/8 + 1/(48*t) + 7/(5760*t**3)

def theta_phi(t):
    """φ-enhanced theta with κ(t) correction"""
    ts = theta_standard(t)
    k = kappa(t)
    correction = math.log(k) / (4*math.pi) if k > 0 else 0
    return ts + correction

def find_zero_n(n, use_phi=True):
    """Solve θ(t) = (n+1)π for the (n+1)-th zero (n starting from 0)"""
    theta_func = theta_phi if use_phi else theta_standard
    target = (n + 1) * math.pi
    
    # Initial guess using asymptotic formula
    t = 2*math.pi*(n+1) / math.log(n+2) + 14
    
    # Newton-Raphson
    for _ in range(30):
        f = theta_func(t) - target
        # Derivative: dθ/dt ≈ (1/2)log(t/2π) for standard
        if use_phi:
            k = kappa(t)
            dk = -2*t*(0.75*t**2 - 0.25) / (0.25 + t**2)**2.5
            df = 0.5*math.log(t/(2*math.pi)) + (dk/k)/(4*math.pi) if k > 0 else 0.5*math.log(t/(2*math.pi))
        else:
            df = 0.5*math.log(t/(2*math.pi))
        
        t_new = t - f/df
        if abs(t_new - t) < 1e-12:
            break
        t = t_new
    
    return t

# ═══════════════════════════════════════════
# COMPUTE
# ═══════════════════════════════════════════
known_zeros = [14.134725, 21.022040, 25.010858, 30.424876, 32.935062,
               37.586178, 40.918719, 43.327073, 48.005151, 49.773832,
               52.970, 56.446, 59.347, 60.831, 65.112]

print("n   known_t      standard_t    err_std    phi_t         err_phi   better")
print("-" * 77)

total_err_std = 0
total_err_phi = 0
phi_better = 0
std_better = 0

for n in range(15):
    known = known_zeros[n]
    t_std = find_zero_n(n, use_phi=False)
    t_phi = find_zero_n(n, use_phi=True)
    
    err_std = abs(t_std - known)
    err_phi = abs(t_phi - known)
    
    total_err_std += err_std
    total_err_phi += err_phi
    
    better = "φ" if err_phi < err_std else "std" if err_std < err_phi else "tie"
    if better == "φ": phi_better += 1
    elif better == "std": std_better += 1
    
    print(f"{n:2d}  {known:12.6f}  {t_std:12.6f}  {err_std:9.6f}  {t_phi:12.6f}  {err_phi:9.6f}  {better}")

print("-" * 77)
print(f"TOTAL ERR:  std={total_err_std:.4f}  phi={total_err_phi:.4f}")
print(f"φ better: {phi_better}  std better: {std_better}  tie: {15-phi_better-std_better}")
print(f"Avg improvement: {(total_err_std-total_err_phi)/15:+.6f} per zero")
print()

# ═══════════════════════════════════════════
# Compute N(T) directly via φ-method
# ═══════════════════════════════════════════
print("T        N_std(T)    N_phi(T)    actual    err_std   err_phi")
print("-" * 62)

def N_std(T):
    if T < 14.13: return 0
    return (T/(2*math.pi))*math.log(T/(2*math.pi)) - T/(2*math.pi) + 7/8

def N_phi(T):
    if T < 14.13: return 0
    n_std = N_std(T)
    k_T = kappa(T)
    k_0 = kappa(14.134725)
    S_phi = (1/math.pi) * math.log(k_T/k_0) if k_T > 0 and k_0 > 0 else 0
    return n_std + S_phi

actual_counts = {10:0, 20:1, 30:3, 40:5, 50:8, 80:19, 100:29, 200:85, 500:334, 1000:857}

for T in [20, 30, 40, 50, 80, 100, 200, 500, 1000]:
    ns = N_std(T)
    nphi = N_phi(T)
    actual = actual_counts[T]
    print(f"{T:5d}  {ns:10.4f}  {nphi:10.4f}  {actual:6d}  {abs(ns-actual):8.4f}  {abs(nphi-actual):8.4f}")

print()
print(f"κ(14.13) = {kappa(14.134725):.6f}")
print(f"κ(100)   = {kappa(100):.6f}")
print(f"κ(1000)  = {kappa(1000):.6f}")
print(f"S_phi(100)  = {(1/math.pi)*math.log(kappa(100)/kappa(14.134725)):.6f}")
print(f"S_phi(1000) = {(1/math.pi)*math.log(kappa(1000)/kappa(14.134725)):.6f}")
