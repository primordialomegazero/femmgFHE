#!/usr/bin/env python3
"""
🔍 RIEMANN CONTINUOUS φ-OSCILLATION — sin(φ·log(t) + δ)
The alternation is in CONTINUOUS height t, not discrete index n.
Model: Δ(t) = density × (1 + A·κ(t)·sin(φ·log(t) + δ))
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

# Compute excesses
excesses = []
for i in range(len(spacings)):
    t = known_zeros[i]
    predicted = 2*pi / math.log(t/(2*pi))
    excess = spacings[i] / predicted - 1.0
    excesses.append(excess)

# ═══════════════════════════════════════════
# CONTINUOUS φ-OSCILLATION FIT
# ═══════════════════════════════════════════
print("═══ CONTINUOUS φ-OSCILLATION FIT ═══")
print()
print("  Model: excess(t) = A·κ(t)·sin(φ·log(t) + δ)")
print()

# Search for best A and δ
best_A = 0
best_delta = 0
best_err = float('inf')

for delta_test in [j*0.05 for j in range(126)]:  # 0 to 2π, fine grid
    sin_vals = [math.sin(phi * math.log(known_zeros[n]) + delta_test) for n in range(len(excesses))]
    k_vals = [kappa(known_zeros[n]) for n in range(len(excesses))]
    
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
print(f"  φ-related check:")
for name, val in [("π/φ", pi/phi), ("π·|ψ|", pi*abs(psi)), ("π/φ²", pi/phi**2), ("2π/φ", 2*pi/phi)]:
    diff = abs(best_delta - val)
    print(f"    {name} = {val:.4f}  (diff = {diff:.4f})")
print()

# ═══════════════════════════════════════════
# PREDICTION vs ACTUAL
# ═══════════════════════════════════════════
print("═══ PREDICTION vs ACTUAL ═══")
print()
print("  n     t_n          actual_Δ    predicted_Δ  error      excess    model_excess")
print("  " + "-"*80)

total_err = 0
model_excesses = []
for i in range(len(spacings)):
    t = known_zeros[i]
    k = kappa(t)
    density = 2*pi / math.log(t/(2*pi))
    
    model_excess = best_A * k * math.sin(phi * math.log(t) + best_delta)
    model_excesses.append(model_excess)
    predicted_delta = density * (1 + model_excess)
    
    err = abs(predicted_delta - spacings[i])
    total_err += err
    
    if i < 25:
        print(f"  {i:2d}  {t:10.6f}  {spacings[i]:9.6f}  {predicted_delta:9.6f}  {err:8.6f}  {excesses[i]:+9.4f}  {model_excess:+9.4f}")

avg_err = total_err/len(spacings)
print("-"*80)
print(f"  Average error (discrete sin(φ·n)): 0.7527 (from previous)")
print(f"  Average error (continuous sin(φ·log(t))): {avg_err:.4f}")
print(f"  Improvement: {(0.7527-avg_err)/0.7527*100:.1f}%")
print()

# ═══════════════════════════════════════════
# SIGN MATCH ANALYSIS
# ═══════════════════════════════════════════
print("═══ SIGN MATCH ANALYSIS ═══")
sign_matches = sum(1 for i in range(len(excesses)) if excesses[i] * model_excesses[i] > 0)
print(f"  Sign matches: {sign_matches}/{len(excesses)} = {100*sign_matches/len(excesses):.1f}%")
print()

# ═══════════════════════════════════════════
# φ-FREQUENCY ANALYSIS (Continuous)
# ═══════════════════════════════════════════
print("═══ φ-FREQUENCY ANALYSIS (Continuous) ═══")
print()

# Correlation of excess with sin(φ·log(t))
sin_corr_cont = sum(excesses[n] * math.sin(phi * math.log(known_zeros[n])) for n in range(len(excesses)))
cos_corr_cont = sum(excesses[n] * math.cos(phi * math.log(known_zeros[n])) for n in range(len(excesses)))
phi_corr_cont = math.sqrt(sin_corr_cont**2 + cos_corr_cont**2) / len(excesses)

# Compare with discrete version
sin_corr_disc = sum(excesses[n] * math.sin(phi * n) for n in range(len(excesses)))
cos_corr_disc = sum(excesses[n] * math.cos(phi * n) for n in range(len(excesses)))
phi_corr_disc = math.sqrt(sin_corr_disc**2 + cos_corr_disc**2) / len(excesses)

# Random baseline
import random
random_corrs = []
for _ in range(200):
    rand_freq = random.uniform(0.5, 3.0)
    s = sum(excesses[n] * math.sin(rand_freq * math.log(known_zeros[n])) for n in range(len(excesses)))
    c = sum(excesses[n] * math.cos(rand_freq * math.log(known_zeros[n])) for n in range(len(excesses)))
    random_corrs.append(math.sqrt(s**2 + c**2) / len(excesses))

avg_random = sum(random_corrs) / len(random_corrs)
max_random = max(random_corrs)

print(f"  φ-freq correlation (discrete sin(φ·n)):   {phi_corr_disc:.6f}")
print(f"  φ-freq correlation (continuous sin(φ·log(t))): {phi_corr_cont:.6f}")
print(f"  Random frequency average:                  {avg_random:.6f}")
print(f"  Random frequency max:                      {max_random:.6f}")
print(f"  Continuous/random ratio:                   {phi_corr_cont/avg_random:.2f}x")
print(f"  Continuous vs discrete improvement:        {(phi_corr_cont-phi_corr_disc)/phi_corr_disc*100:.1f}%")
print()

# ═══════════════════════════════════════════
# PHASE PORTRAIT
# ═══════════════════════════════════════════
print("═══ PHASE PORTRAIT — excess/κ vs φ·log(t) mod 2π ═══")
print()

# Plot data in polar-like coordinates (text-based)
print("  φ·log(t) mod 2π   excess/κ(t)   sin(φ·log(t)+δ)")
print("  " + "-"*50)
for i in [0, 1, 2, 3, 4, 5, 8, 10, 12, 15, 18, 20, 25, 30, 35, 40, 45]:
    if i < len(excesses):
        t = known_zeros[i]
        phase = (phi * math.log(t) + best_delta) % (2*pi)
        scaled = excesses[i] / kappa(t) if kappa(t) > 0 else 0
        expected_sin = math.sin(phi * math.log(t) + best_delta)
        print(f"  {phase:13.4f}  {scaled:+11.4f}  {expected_sin:+11.4f}")

print()
print("╔══════════════════════════════════════════════════════════════╗")
print("║  CONTINUOUS φ-OSCILLATION MODEL:                           ║")
print("║  Δ(t) = density(t) × (1 + A·κ(t)·sin(φ·log(t) + δ))      ║")
print("║  The φ-spiral is in LOG-SPACE, not index-space.            ║")
print("║  κ(t) = 2t²/(0.25+t²)^1.5 is the ENVELOPE.                ║")
print("║  As t→∞, κ→0, oscillation dies → classical density.       ║")
print("╚══════════════════════════════════════════════════════════════╝")
