import math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

p = 17
Gx, Gy = 1, 5
n = 9

def modinv(a, m): return pow(a, -1, m)

def pt_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3*x1*x1) * modinv(2*y1, p) % p
    else:
        lam = ((y2-y1) * modinv(x2-x1, p)) % p
    x3 = (lam*lam - x1 - x2) % p
    y3 = (lam*(x1 - x3) - y1) % p
    return (x3, y3)

def pt_double(P):
    return pt_add(P, P)

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

def phi_map(P):
    if P is None: return 0.0
    return (float(P[0]) * PHI + float(P[1]) * PSI) % float(p)

# Generate ALL curve points
def all_points():
    points = []
    for k in range(1, n):
        P = scalar_mult(k, (Gx, Gy))
        if P not in points:
            points.append(P)
    return points

points = all_points()
print(f"═══ ALL HOMOMORPHIC POINTS (p=17) ═══\n")
print(f"  Curve order: {n}")
print(f"  Total points: {len(points)}\n")
print(f"  Point      φ(P)           φ(2P)         2·φ(P)        Match?")
print(f"  " + "-"*70)

homomorphic = []
for P in points:
    if P is None: continue
    P2 = pt_double(P)
    phi_P = phi_map(P)
    phi_2P = phi_map(P2)
    double_phi = (2 * phi_P) % p
    match = abs(phi_2P - double_phi) < 0.001
    if match: homomorphic.append(P)
    print(f"  {P}  {phi_P:.4f}  {phi_2P:.4f}  {double_phi:.4f}  {'✅' if match else '❌'}")

print(f"\n  Homomorphic points: {len(homomorphic)}")
for P in homomorphic:
    print(f"    {P} (order: {n})")
print(f"\n  Permutation fixed points: {len(homomorphic)}")
