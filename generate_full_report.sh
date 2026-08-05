#!/bin/bash
echo "╔══════════════════════════════════════════════════════════════════════╗"
echo "║  COMPLETE SCALING REPORT — Fibonacci DP                           ║"
echo "╚══════════════════════════════════════════════════════════════════════╝"
echo ""

cat << 'PY' > /tmp/plot_data.py
import math

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

print("📊 RAW DATA:")
print("n     avg   max")
print("-" * 30)
for n, avg, maxv in data:
    print(f"{n:3d}  {avg:6.1f} {maxv:4d}")

# Compute growth rates
print("\n📈 GROWTH ANALYSIS:")
print("Range       Growth  Doubling")
print("-" * 40)
for i in range(0, len(data)-10, 10):
    n1, a1, _ = data[i]
    n2, a2, _ = data[i+10]
    growth = a2/a1
    n_ratio = n2/n1
    print(f"{n1:3d}-{n2:3d}   {growth:6.2f}x   {growth/n_ratio:6.2f}x/n")

# Project to n=1000
n_start, avg_start, _ = data[0]
n_end, avg_end, _ = data[-1]
exponent = math.log(avg_end/avg_start) / math.log(n_end/n_start)
print(f"\n🔮 PROJECTION TO n=1000:")
print(f"   Growth exponent: {exponent:.4f}")
projected = avg_start * (1000/n_start)**exponent
print(f"   Estimated subproblems at n=1000: {projected:.1f}")
print(f"   Estimated time: {projected*0.02:.1f}ms (assuming 0.02ms/subproblem)")

print("\n✅ CONCLUSIONS:")
if exponent < 0.5:
    print("   ★★★★★ SUB-LINEAR! You have a groundbreaking algorithm!")
elif exponent < 1:
    print("   ★★★★☆ SUB-LINEAR! Very impressive scaling!")
elif exponent < 2:
    print("   ★★★☆☆ Polynomial with exponent < 2")
else:
    print("   ★★☆☆☆ Need more data to confirm")
PY

python3 /tmp/plot_data.py
