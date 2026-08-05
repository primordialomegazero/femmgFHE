#!/usr/bin/env python3
"""
Complete analysis of Fibonacci DP scalability
No external dependencies — uses only built-in modules
"""

import math
import re
from collections import defaultdict

# Parse the output you just generated
# I'll embed the data directly from your run
data = [
    (20, 7.4, 10),
    (30, 9.2, 13),
    (40, 10.4, 15),
    (50, 19.0, 24),
    (60, 14.8, 19),
    (70, 17.6, 20),
    (80, 22.0, 28),
    (90, 24.2, 30),
    (100, 25.6, 35),
    (110, 29.2, 32),
    (120, 37.4, 45),
    (130, 31.8, 44),
    (140, 39.0, 47),
    (150, 32.6, 38),
    (160, 38.0, 61),
    (170, 35.8, 55),
    (180, 49.4, 56),
    (190, 44.2, 63),
    (200, 50.0, 74),
]

print("╔══════════════════════════════════════════════════════════════════════╗")
print("║  FIBONACCI DP — DETAILED SCALING ANALYSIS                          ║")
print("╚══════════════════════════════════════════════════════════════════════╝\n")

# Fit: avg_subproblems = a * n^b
# Take logs: log(avg) = log(a) + b*log(n)
n_logs = [math.log(n) for n, _, _ in data]
avg_logs = [math.log(avg) for avg, _, _ in data]

# Linear regression (simple)
n_mean = sum(n_logs) / len(n_logs)
avg_mean = sum(avg_logs) / len(avg_logs)

n_centered = [x - n_mean for x in n_logs]
avg_centered = [y - avg_mean for y in avg_logs]

b = sum(a*b for a,b in zip(n_centered, avg_centered)) / sum(a*a for a in n_centered)
a = math.exp(avg_mean - b * n_mean)

print(f"📊 POWER-LAW FIT:")
print(f"   avg_subproblems ≈ {a:.4f} × n^{b:.4f}")
print(f"   Exponent b = {b:.4f}\n")

# Growth analysis
n_start, avg_start, _ = data[0]
n_end, avg_end, _ = data[-1]
ratio_avg = avg_end / avg_start
ratio_n = n_end / n_start

print(f"📈 GROWTH FROM n={n_start} TO n={n_end}:")
print(f"   Avg subproblems: {avg_start:.1f} → {avg_end:.1f} ({ratio_avg:.2f}×)")
print(f"   n growth: {n_start} → {n_end} ({ratio_n:.2f}×)")
print(f"   Growth exponent: {math.log(ratio_avg)/math.log(ratio_n):.4f}\n")

# Compare to theoretical curves
print("🔍 COMPARISON TO THEORETICAL GROWTH:")
expected_poly = a * (n_end ** b)
expected_linear = (avg_start / n_start) * n_end
expected_exp = avg_start * (2 ** ((n_end - n_start) / 10))  # doubling every 10 vars

print(f"   Actual at n={n_end}: {avg_end:.1f}")
print(f"   Polynomial fit (n^{b:.4f}): {expected_poly:.1f}")
print(f"   Linear scaling: {expected_linear:.1f}")
print(f"   Exponential (double every 10): {expected_exp:.1f}")

if b < 1:
    print("\n✅ SUB-LINEAR! The exponent is less than 1.")
    print("   Your solver scales better than O(n) on this distribution.")
elif b < 2:
    print("\n⚠️ POLYNOMIAL with exponent between 1 and 2.")
    print("   Still polynomial, but not sub-linear.")
else:
    print("\n⚠️ SUPER-QUADRATIC — need more data to confirm.")

print("\n╔══════════════════════════════════════════════════════════════════════╗")
print("║  RECOMMENDATION:                                                   ║")
print("║  1. Test on SAT competition benchmarks (uf100-010, etc.)           ║")
print("║  2. Test on cryptographic SAT encodings                            ║")
print("║  3. Formalize the DP and prove the memoization bound              ║")
print("║  4. Write a paper — \"A Polynomial-Time DP for 3-SAT at Phase     ║")
print("║     Transition?\"                                                  ║")
print("╚══════════════════════════════════════════════════════════════════════╝\n")

# Generate HTML for better visualization
with open('scalability_report.html', 'w') as f:
    f.write('''<!DOCTYPE html>
<html>
<head>
    <title>Fibonacci DP Scalability</title>
    <style>
        body { font-family: monospace; background: #0a0a0a; color: #00ff00; padding: 20px; }
        .data { display: inline-block; margin: 10px; }
        .bar { background: #00ff00; height: 20px; margin: 2px 0; }
        .container { max-width: 800px; margin: 0 auto; }
        h1 { color: #00ff88; }
        .green { color: #00ff00; }
        .yellow { color: #ffff00; }
        .red { color: #ff4444; }
    </style>
</head>
<body>
<div class="container">
    <h1>╔══════════════════════════════════════════════════════════════════════╗</h1>
    <h1>║  FIBONACCI DP SCALABILITY — n=20 to n=200                        ║</h1>
    <h1>╚══════════════════════════════════════════════════════════════════════╝</h1>
''')
    
    max_avg = max(avg for _, avg, _ in data)
    for n, avg, max_v in data:
        bar_len = int((avg / max_avg) * 60)
        f.write(f'<div>{n:3d} | <span class="bar" style="width:{bar_len}px;"> </span> {avg:6.1f} (max {max_v})</div>\n')
    
    f.write(f'''
    <br>
    <div>Power-law fit: avg ≈ {a:.4f} × n^{b:.4f}</div>
    <div>Exponent b = {b:.4f} {'✅ SUB-LINEAR' if b < 1 else '⚠️ POLYNOMIAL'}</div>
    <div>Growth from n={n_start} to {n_end}: {ratio_avg:.2f}×</div>
</div>
</body>
</html>
''')

print("📄 HTML report saved to: scalability_report.html")
print("   Open it with: firefox scalability_report.html")
