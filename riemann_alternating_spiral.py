#!/usr/bin/env python3
"""
🔍 RIEMANN ALTERNATING SPIRAL — φ-DRIVEN OSCILLATION IN ZERO SPACING
The spacing Δ_n oscillates with a φ-based period.
Pattern: Δ_n = density × (1 + A·κ(t) · sin(φ·n + δ))
"""
import math

phi = (1 + math.sqrt(5)) / 2
psi = (1 - math.sqrt(5)) / 2
pi = math.pi

def kappa(t):
    return 2 * t**2 / (0.25 + t**2)**1.5

known_zeros = [
    14.134725142, 21.022039639, 25.010857580, 30.424876126, 32.935061588,
    37.586178159, 40.918719012, 43.327073281, 48.005150881, 49.773832478,
    52.970321478, 56.446247697, 59.347044003, 60.831778525, 65.112544048,
    67.079810529, 69.546401711, 72.067157674, 75.704690699, 77.144840069,
    79.337375020, 82.910380854, 84.735492981, 87.425274613, 88.809111208,
    92.491899271, 94.651344041, 95.870634228, 98.831194218, 101.317851006,
    103.725538040, 105.446623052, 107.168611184, 111.029535994, 111.874659179,
    114.320220915, 116.226680321, 118.790782866, 121.370125002, 122.946829294,
    124.256818554, 127.516683880, 129.578704200, 131.087688531, 133.497737203,
    134.756509754, 138.116042055, 139.736208952, 141.123707404, 143.111845808
]

spacings = [known_zeros[i+1] - known_zeros[i] for i in range(len(known_zeros)-1)]

# ═══════════════════════════════════════════
# ALTERNATING PATTERN ANALYSIS
# ═══════════════════════════════════════════
print("═══ ALTERNATING PATTERN IN SPACING ═══")
print()

# Compute the "excess" over predicted density
excesses = []
for i in range(len(spacings)):
    t = known_zeros[i]
    predicted = 2*pi / math.log(t/(2*pi))
    excess = spacings[i] / predicted - 1.0
    excesses.append(excess)

# Check alternating sign
print("  Alternation check (sign of excess):")
alternations = 0
for i in range(1, len(excesses)):
    if excesses[i] * excesses[i-1] < 0:
        alternations += 1

print(f"  Sign changes: {alternations}/{len(excesses)-1} = {100*alternations/(len(excesses)-1):.1f}%")
print()

# ═══════════════════════════════════════════
# φ-PERIOD ANALYSIS
# ═══════════════════════════════════════════
print("═══ φ-PERIOD IN ALTERNATION ═══")
print()

# If the alternation follows φ, then the distance between sign changes
# should cluster around φ-related intervals

sign_change_positions = []
for i in range(1, len(excesses)):
    if excesses[i] * excesses[i-1] < 0:
        sign_change_positions.append(i)

print("  Positions of sign changes:")
print(f"  {sign_change_positions[:20]}")
print()

# Intervals between sign changes
intervals = [sign_change_positions[i+1] - sign_change_positions[i] 
             for i in range(len(sign_change_positions)-1)]
print("  Intervals between sign changes:")
print(f"  {intervals[:15]}")
print()

# Check if intervals cluster around φ-related numbers
avg_interval = sum(intervals) / len(intervals)
print(f"  Average interval: {avg_interval:.3f}")
print(f"  φ = {phi:.3f}")
print(f"  φ+1 = {phi+1:.3f}")
print(f"  φ² = {phi**2:.3f}")
print(f"  Closest φ-related number: ", end="")
diffs = [abs(avg_interval - phi), abs(avg_interval - (phi+1)), abs(avg_interval - phi**2)]
if min(diffs) == diffs[0]:
    print(f"φ (diff={diffs[0]:.3f})")
elif min(diffs) == diffs[1]:
    print(f"φ+1 (diff={diffs[1]:.3f})")
else:
    print(f"φ² (diff={diffs[2]:.3f})")
print()

# ═══════════════════════════════════════════
# ALTERNATING SPIRAL MODEL
# ═══════════════════════════════════════════
print("═══ ALTERNATING SPIRAL MODEL ═══")
print()

# Model: excess_n = A·κ(t_n) · sin(φ·n + δ)
# The κ(t) provides the ENVELOPE (decaying amplitude)
# The sin(φ·n + δ) provides the ALTERNATION

import math as m

# Fit the phase δ and amplitude A
# We want: excess_n ≈ A · κ(t_n) · sin(φ·n + δ)

# First, find best δ by minimizing error
best_A = 0
best_delta = 0
best_err = float('inf')

for delta_test in [j*0.1 for j in range(63)]:  # 0 to 2π
    # For each δ, find A by least squares
    sin_vals = [m.sin(phi * n + delta_test) for n in range(len(excesses))]
    k_vals = [kappa(known_zeros[n]) for n in range(len(excesses))]
    
    # A = Σ(excess × κ×sin) / Σ(κ×sin)²
    num = sum(excesses[n] * k_vals[n] * sin_vals[n] for n in range(len(excesses)))
    den = sum((k_vals[n] * sin_vals[n])**2 for n in range(len(excesses)))
    
    if abs(den) < 1e-10:
        continue
    
    A_test = num / den
    err = sum((excesses[n] - A_test * k_vals[n] * sin_vals[n])**2 for n in range(len(excesses)))
    
    if err < best_err:
        best_err = err
        best_A = A_test
        best_delta = delta_test

print(f"  Best fit: A = {best_A:.4f}, δ = {best_delta:.4f} rad")
print(f"  δ/π = {best_delta/pi:.4f}")
print(f"  φ-related phase? ", end="")
if abs(best_delta - pi/phi) < 0.2:
    print("δ ≈ π/φ ✅")
elif abs(best_delta - pi*psi) < 0.2:
    print("δ ≈ π·ψ ✅")
else:
    print(f"δ = {best_delta:.4f} (custom)")
print()

# ═══════════════════════════════════════════
# PREDICTION WITH ALTERNATING MODEL
# ═══════════════════════════════════════════
print("═══ PREDICTION vs ACTUAL ═══")
print()
print("  n     actual_Δ    predicted_Δ  error      excess    model_excess")
print("  " + "-"*68)

total_err = 0
for i in range(min(30, len(spacings))):
    t = known_zeros[i]
    k = kappa(t)
    density = 2*pi / m.log(t/(2*pi))
    
    # Model: Δ = density × (1 + A·κ·sin(φ·n + δ))
    model_excess = best_A * k * m.sin(phi * i + best_delta)
    predicted_delta = density * (1 + model_excess)
    
    err = abs(predicted_delta - spacings[i])
    total_err += err
    
    print(f"  {i:2d}  {spacings[i]:9.6f}  {predicted_delta:9.6f}  {err:8.6f}  {excesses[i]:+9.4f}  {model_excess:+9.4f}")

print("-"*68)
print(f"  Average error: {total_err/min(30,len(spacings)):.6f}")
print()

# Check if model captures the alternation
print("═══ ALTERNATION CAPTURE ═══")
model_excesses = [best_A * kappa(known_zeros[n]) * m.sin(phi * n + best_delta) 
                  for n in range(len(excesses))]

sign_matches = 0
for i in range(len(excesses)):
    if excesses[i] * model_excesses[i] > 0:  # same sign
        sign_matches += 1

print(f"  Sign matches: {sign_matches}/{len(excesses)} = {100*sign_matches/len(excesses):.1f}%")
print()

# FFT-like analysis for φ-frequency
print("═══ φ-FREQUENCY ANALYSIS ═══")
print()

# Check correlation of excess with sin(φ·n) and cos(φ·n)
sin_corr = sum(excesses[n] * m.sin(phi * n) for n in range(len(excesses)))
cos_corr = sum(excesses[n] * m.cos(phi * n) for n in range(len(excesses)))
phi_corr = m.sqrt(sin_corr**2 + cos_corr**2) / len(excesses)

# Compare with random frequencies
import random
random_corrs = []
for _ in range(100):
    rand_freq = random.uniform(0.5, 3.0)
    s = sum(excesses[n] * m.sin(rand_freq * n) for n in range(len(excesses)))
    c = sum(excesses[n] * m.cos(rand_freq * n) for n in range(len(excesses)))
    random_corrs.append(m.sqrt(s**2 + c**2) / len(excesses))

avg_random = sum(random_corrs) / len(random_corrs)
print(f"  φ-frequency correlation: {phi_corr:.6f}")
print(f"  Random frequency avg:    {avg_random:.6f}")
print(f"  φ/random ratio:          {phi_corr/avg_random:.2f}x")
print()

print("╔══════════════════════════════════════════════════════════════╗")
print("║  ALTERNATING SPIRAL MODEL:                                 ║")
print("║  Δ_n = density × (1 + A·κ(t_n)·sin(φ·n + δ))             ║")
print("║  The φ-frequency sin(φ·n) drives the alternation.         ║")
print("║  κ(t) provides the DECAYING ENVELOPE.                     ║")
print("║  As t→∞, κ→0, alternation dies out → classical density.  ║")
print("╚══════════════════════════════════════════════════════════════╝")
