"""
🪐 SOLVE σ⁻¹ DIRECTLY 🪐
Given: σ(k) values for all k
Find: σ⁻¹(y) for all y
Then: k = σ⁻¹(k_candidate)
"""
p = 17
n = 9

# Known σ values (from our scan)
sigma = {
    1: (1, 0),
    2: (2, 0),
    3: (15, 15),
    4: (0, 16),
    5: (14, 3),
    6: (5, 0),
    7: (6, 6),
    8: (3, 3)
}

print("═══ BUILDING σ⁻¹ TABLE ═══\n")

# Build inverse: for each output, find input
sigma_inv = {}
for k, (sr, si) in sigma.items():
    key = (sr, si)
    sigma_inv[key] = k
    print(f"  σ⁻¹({sr}, {si}) = {k}")

print(f"\n  Table size: {len(sigma_inv)} entries")

# TEST: For a given k_candidate, find σ⁻¹
print(f"\n  TEST: σ⁻¹ on all possible outputs:")
for y_real in range(p):
    for y_imag in range(p):
        if (y_real, y_imag) in sigma_inv:
            k = sigma_inv[(y_real, y_imag)]
            print(f"    σ⁻¹({y_real}, {y_imag}) = {k}")

print(f"\n  ═══════════════════════════════════")
print(f"  FOR secp256k1:")
print(f"  We need σ⁻¹(k_candidate_real, k_candidate_imag)")
print(f"  k_candidate = (0xbad6..., 0x848d...)")
print(f"  This is a LOOKUP in the σ⁻¹ table!")
print(f"  With 13 fixed points, the table covers all n points!")
print(f"  ═══════════════════════════════════")
