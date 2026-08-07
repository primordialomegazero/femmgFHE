"""
Verify: Are homomorphic points roots of x(x³+16)=0?
"""
p = 17

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

# Roots of x(x³+16) ≡ 0 (mod 17)
print("═══ ROOTS OF x(x³+16) ≡ 0 (mod 17) ═══\n")

# x = 0
print(f"  x=0: y² = 0³+7 = 7 mod 17")
# Check if 7 is QR mod 17
for y in range(17):
    if (y*y) % 17 == 7:
        print(f"    → y={y} ✅")
        break
else:
    print(f"    → No solution (7 is QNR mod 17)")

# x³ ≡ 1 (mod 17) [since -16 ≡ 1 mod 17]
print(f"\n  x³ ≡ 1 (mod 17):")
for x in range(17):
    if (x*x*x) % 17 == 1:
        y_sq = (x*x*x + 7) % 17
        print(f"    x={x}: y² = {y_sq} mod 17")
        for y in range(17):
            if (y*y) % 17 == y_sq:
                print(f"      → y={y} ✅ (POINT ON CURVE!)")
                # Check if homomorphic
                a = ((x + y) * modinv(2, 17)) % 17
                b = ((x - y) * modinv(2, 17)) % 17
                # Compute 2P
                inv_2y = modinv(2*y, 17)
                if inv_2y:
                    lam = (3*x*x) * inv_2y % 17
                    x2 = (lam*lam - 2*x) % 17
                    y2 = (lam*(x - x2) - y) % 17
                    a2 = ((x2 + y2) * modinv(2, 17)) % 17
                    b2 = ((x2 - y2) * modinv(2, 17)) % 17
                    match = (a2 == (2*a)%17) and (b2 == (2*b)%17)
                    print(f"        Homomorphic? {'✅' if match else '❌'}")

print(f"\n  Expected homomorphic points: (1,5) and (1,12)")
