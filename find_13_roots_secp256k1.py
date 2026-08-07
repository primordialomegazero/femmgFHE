"""
🪐 FIND 13 HOMOMORPHIC POINTS — DIRECT SOLVE 🪐
Solve: x(x³+16) ≡ 0 (mod p) AND y² = x³+7
"""
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

def legendre(a, p):
    """Legendre symbol (a|p)"""
    return pow(a, (p-1)//2, p)

def sqrt_mod(a, p):
    """Tonelli-Shanks for square root modulo p"""
    if legendre(a, p) != 1:
        return None
    # Simplified for prime p ≡ 3 (mod 4)
    if p % 4 == 3:
        return pow(a, (p+1)//4, p)
    # Generic Tonelli-Shanks (simplified)
    return pow(a, (p+1)//4, p)  # secp256k1: p ≡ 3 mod 4 ✅

print("═══ FINDING ALL 13 HOMOMORPHIC POINTS — secp256k1 ═══\n")

# Case 1: x = 0
x = 0
y_sq = (x*x*x + 7) % p
y = sqrt_mod(y_sq, p)
if y:
    print(f"  Root 1: x=0, y={hex(y)}")
    print(f"  Root 2: x=0, y={hex(p-y)}")

# Case 2: x³ ≡ -16 (mod p)
# Find cube roots of (p-16) mod p
# Using: if p ≡ 2 mod 3, cube root is unique
print(f"\n  p mod 3 = {p % 3}")
if p % 3 == 2:
    # Unique cube root
    target = (p - 16) % p
    x = pow(target, (2*p-1)//3, p)
    y_sq = (x*x*x + 7) % p
    y = sqrt_mod(y_sq, p)
    print(f"  Target: p-16 = {hex(target)[:30]}...")
    print(f"  x = {hex(x)[:30]}...")
    if y:
        print(f"  Root 3-4: x={hex(x)[:20]}..., y=±{hex(y)[:20]}...")
else:
    print(f"  Need general cube root (p ≡ {p % 3} mod 3)")
    print(f"  There are 3 cube roots!")

print(f"\n  Total solutions to x(x³+16)=0: 4 (x=0 + 3 cube roots)")
print(f"  Each with ±y where y exists: up to 8 affine points")
print(f"  Plus point at infinity: 1")
print(f"  Maximum: 9 points (not 13)")
print(f"\n  ⚠️ 13 ≠ 9 → May karagdagang structure!")
