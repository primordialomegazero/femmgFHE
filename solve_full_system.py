"""
🪐 SOLVE FULL HOMOMORPHIC SYSTEM 🪐
x' = 2x + t, y' = 2y - t
With doubling constraint
"""
p = 17  # Test muna sa tiny curve

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

def pt_double(P):
    if P is None: return None
    x1, y1 = P
    inv = modinv(2*y1, p)
    if inv is None: return None
    lam = (3*x1*x1) * inv % p
    x3 = (lam*lam - 2*x1) % p
    y3 = (lam*(x1 - x3) - y1) % p
    return (x3, y3)

def to_Fp2(x, y):
    a = ((x + y) * modinv(2, p)) % p
    b = ((x - y) * modinv(2, p)) % p
    return a, b

print("═══ FULL HOMOMORPHIC SYSTEM — p=17 ═══\n")
print(f"  Looking for ALL points where φ(2P) = 2·φ(P)\n")

homomorphic = []
for x in range(p):
    for y in range(p):
        if (y*y) % p != (x*x*x + 7) % p:
            continue
        P = (x, y)
        P2 = pt_double(P)
        if P2 is None:
            continue
        
        a, b = to_Fp2(x, y)
        a2, b2 = to_Fp2(P2[0], P2[1])
        
        # Check BOTH conditions
        if a2 == (2*a) % p and b2 == (2*b) % p:
            homomorphic.append(P)
            # Compute t
            t = (P2[0] - 2*x) % p
            print(f"  ({x},{y}): a'={a2}, 2a={(2*a)%p}, b'={b2}, 2b={(2*b)%p}, t={t} ✅")

print(f"\n  Total: {len(homomorphic)} points")
print(f"  (Expected: 2 for p=17)")
