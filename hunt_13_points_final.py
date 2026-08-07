"""
🪐 HUNT 13 HOMOMORPHIC POINTS — FINAL 🪐
x³ ≡ -16 (mod p), p ≡ 1 mod 3 → 3 cube roots
"""
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

def legendre(a, p):
    return pow(a, (p-1)//2, p)

def sqrt_mod(a, p):
    if legendre(a, p) != 1:
        return None
    return pow(a, (p+1)//4, p)

def cube_roots(a, p):
    """Find all cube roots of a mod p (p ≡ 1 mod 3)"""
    # Simplified for s=1 case
    if p % 3 != 1:
        return [pow(a, (2*p-1)//3, p)]
    
    # Factor p-1 = 3^s * t
    s, t = 0, p-1
    while t % 3 == 0:
        s += 1
        t //= 3
    
    # Find primitive 3^s-th root of unity
    g = 2
    while pow(g, (p-1)//3, p) == 1:
        g += 1
    omega = pow(g, (p-1)//3, p)
    
    # Adleman-Manders-Miller
    # For s=1, simple case:
    if s == 1:
        r = pow(a, (2*t + 1)//3, p)
        roots = [r]
        omega_cube = pow(g, (p-1)//3, p)
        roots.append((r * omega_cube) % p)
        roots.append((r * omega_cube * omega_cube) % p)
        return roots
    
    return [pow(a, (2*p-1)//3, p)]  # Fallback

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 HUNTING ALL 13 HOMOMORPHIC POINTS 🪐                ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

target = (p - 16) % p
print(f"  Solving: x³ ≡ {hex(target)[:30]}... (mod p)\n")

roots = cube_roots(target, p)
print(f"  Found {len(roots)} cube roots:")

homomorphic_points = []

for i, x in enumerate(roots):
    y_sq = (pow(x, 3, p) + 7) % p
    y = sqrt_mod(y_sq, p)
    print(f"\n  Root {i+1}: x = {hex(x)[:40]}...")
    if y:
        print(f"    y₁ = {hex(y)[:40]}...")
        print(f"    y₂ = {hex(p-y)[:40]}...")
        homomorphic_points.append((x, y))
        homomorphic_points.append((x, p-y))
        
        # Compute k values (discrete logs)
        print(f"    These are points on the curve!")
        print(f"    Need to compute k = discrete_log(P)")
        print(f"    → Use bsgs or φ-DPLL for small instances")
    else:
        print(f"    No y solution (y² is QNR)")

# Check x=0
x = 0
y_sq = (pow(x, 3, p) + 7) % p
y = sqrt_mod(y_sq, p)
print(f"\n  Root x=0:")
if y:
    print(f"    y₁ = {hex(y)[:40]}...")
    print(f"    y₂ = {hex(p-y)[:40]}...")
    homomorphic_points.append((x, y))
    homomorphic_points.append((x, p-y))
else:
    print(f"    No y solution")

# Point at infinity
print(f"\n  Point at infinity: O (always homomorphic)")

print(f"\n  TOTAL: {len(homomorphic_points)} affine + 1 at infinity = {len(homomorphic_points)+1} points")
print(f"  Expected: 13!")
