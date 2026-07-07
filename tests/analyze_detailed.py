#!/usr/bin/env python3
"""
Detailed analysis ng tensor data
Hanapin ang exact formula para sa scaling factor
"""

import csv
import math
from collections import defaultdict

# Read data
data = []
with open('tensor_data.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        data.append({
            'a': int(row['a']),
            'b': int(row['b']),
            'expected': int(row['expected']),
            'actual': int(row['actual']),
            'ratio': float(row['ratio']),
            'noise': int(row['noise'])
        })

print("=" * 80)
print("DETAILED TENSOR DATA ANALYSIS")
print("=" * 80)
print(f"Total entries: {len(data)}\n")

# ========================================
# 1. Hanapin ang pattern ng ratio
# ========================================
print("1. PATTERN NG RATIO")
print("-" * 40)

# I-group by a at b
ratios_by_a = defaultdict(list)
ratios_by_b = defaultdict(list)

for d in data:
    ratios_by_a[d['a']].append(d['ratio'])
    ratios_by_b[d['b']].append(d['ratio'])

# Check kung ang ratio ay depende lang sa a
print("\nRatios by a (unique values):")
for a in sorted(ratios_by_a.keys()):
    unique = set(ratios_by_a[a])
    if len(unique) > 1:
        print(f"  a={a:2d}: {len(unique)} different ratios")
    else:
        print(f"  a={a:2d}: ratio = {unique.pop():.4f}")

# Check kung ang ratio ay depende lang sa b
print("\nRatios by b (unique values):")
for b in sorted(ratios_by_b.keys()):
    unique = set(ratios_by_b[b])
    if len(unique) > 1:
        print(f"  b={b:2d}: {len(unique)} different ratios")
    else:
        print(f"  b={b:2d}: ratio = {unique.pop():.4f}")

# ========================================
# 2. Hanapin ang formula
# ========================================
print("\n" + "=" * 80)
print("2. FORMULA ANALYSIS")
print("=" * 80)

# I-test ang iba't ibang formula
formulas = [
    ("ratio = (a + b) / (a + b - 10)", lambda d: (d['a'] + d['b']) / (d['a'] + d['b'] - 10)),
    ("ratio = (a + b) / (b - a + 10)", lambda d: (d['a'] + d['b']) / (d['b'] - d['a'] + 10)),
    ("ratio = (a + b) / (a + b - 5)", lambda d: (d['a'] + d['b']) / (d['a'] + d['b'] - 5)),
    ("ratio = (a + b) / (a + b - 15)", lambda d: (d['a'] + d['b']) / (d['a'] + d['b'] - 15)),
    ("ratio = a / (a - 5) * b / (b - 5)", lambda d: (d['a'] / (d['a'] - 5)) * (d['b'] / (d['b'] - 5)) if d['a'] > 5 and d['b'] > 5 else None),
    ("ratio = (a + 5) / 10 * (b + 5) / 10", lambda d: ((d['a'] + 5) / 10) * ((d['b'] + 5) / 10)),
    ("ratio = a / 5 * b / 5", lambda d: (d['a'] / 5) * (d['b'] / 5)),
]

for name, func in formulas:
    matches = 0
    total = 0
    for d in data:
        try:
            predicted = func(d)
            if predicted is not None and abs(predicted - d['ratio']) < 0.01:
                matches += 1
            total += 1
        except:
            pass
    print(f"\n{name}")
    print(f"  Matches: {matches}/{total} ({matches/total*100:.1f}%)")

# ========================================
# 3. Sample data para makita ang pattern
# ========================================
print("\n" + "=" * 80)
print("3. SAMPLE DATA (a=1-10, b=1-10)")
print("=" * 80)

print("\n     ", end="")
for b in range(1, 11):
    print(f"{b:6d}", end="")
print()

for a in range(1, 11):
    print(f"{a:3d} ", end="")
    for b in range(1, 11):
        for d in data:
            if d['a'] == a and d['b'] == b:
                print(f"{d['ratio']:6.2f}", end="")
                break
    print()

# ========================================
# 4. Hanapin ang exact formula
# ========================================
print("\n" + "=" * 80)
print("4. EXACT FORMULA SEARCH")
print("=" * 80)

# Hanapin ang formula na ratio = f(a) * f(b)
print("\nChecking if ratio = f(a) * f(b)")

# Compute f(a) from data where b=10
f_values = {}
for a in range(1, 21):
    for d in data:
        if d['a'] == a and d['b'] == 10:
            f_values[a] = d['ratio']
            break

print(f"\nf(a) values (from a×10):")
for a in sorted(f_values.keys()):
    print(f"  f({a:2d}) = {f_values[a]:.4f}")

# I-test kung gumagana ang f(a) * f(b)
matches = 0
for d in data:
    if d['a'] in f_values and d['b'] in f_values:
        predicted = f_values[d['a']] * f_values[d['b']]
        if abs(predicted - d['ratio']) < 0.01:
            matches += 1

print(f"\nMatches using f(a)*f(b): {matches}/{len(data)} ({matches/len(data)*100:.1f}%)")

# ========================================
# 5. Conclusion
# ========================================
print("\n" + "=" * 80)
print("5. CONCLUSION")
print("=" * 80)
print("""
Ang scaling factor ay:
- Hindi linear
- Depende sa a at b
- Mukhang product ng individual factors: f(a) × f(b)

Ang f(a) ay kailangan pang i-determine.
""")

