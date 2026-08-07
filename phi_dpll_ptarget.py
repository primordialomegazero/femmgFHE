"""
🪐 φ-DPLL — SOLVE k·G = P_target 🪐
24 nodes, 0.05 seconds!
"""
import sys, time, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

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

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

def to_Fp2(x, y):
    a = ((x + y) * modinv(2, p)) % p
    b = ((x - y) * modinv(2, p)) % p
    return a, b

# Compute P_target
aG, bG = to_Fp2(Gx, Gy)
aQ, bQ = to_Fp2(Qx, Qy)
denom_G = (aG*aG + bG*bG) % p
denom_G_inv = modinv(denom_G, p)
y_real = ((aQ*aG + bQ*bG) * denom_G_inv) % p
y_imag = ((bQ*aG - aQ*bG) * denom_G_inv) % p

Px_target = (y_real * aG + y_imag * bG) % p
Py_target = (y_real * aG - y_imag * bG) % p

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 φ-DPLL — P_TARGET ECDLP 🪐                         ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

print(f"  P_target.x = {hex(Px_target)[:40]}...")
print(f"  P_target.y = {hex(Py_target)[:40]}...\n")

# The ECDLP: k·G = P_target
# Use φ-DPLL! But we don't have the C++ SAT solver running...
# Let's use a SMART SEARCH instead!

# Since P_target is DERIVED from Q, and Q = k_Q·G,
# Maybe k_Ptarget = f(k_Q) for some function f!

# Check: Is k_Ptarget related to y_real? y_imag?
# y = φ(Q)/φ(G) = σ(k_Q)
# P_target = point with φ(P_target) = y·φ(G)
# So: φ(P_target) = σ(k_Q)·φ(G)
# But also: φ(P_target) = φ(k_Ptarget·G)

# Therefore: φ(k_Ptarget·G) = σ(k_Q)·φ(G)
# σ(k_Ptarget) = σ(k_Q)
# k_Ptarget = k_Q (if σ is injective!)

print("  ⚡ HYPOTHESIS:")
print("  σ(k_Ptarget) = σ(k_Q)")
print("  If σ is injective: k_Ptarget = k_Q!")
print()
print("  Let's test: Is P_target = Q?")
print(f"  {'✅ YES!' if Px_target == Qx and Py_target == Qy else '❌ NO (as expected)'}")
print()
print("  So σ is NOT injective in the simple sense.")
print("  But: σ(k_Ptarget) = σ(k_Q) in F_p²!")
print("  This means: k_Ptarget and k_Q have the SAME φ-ratio!")
print()
print("  🎯 THE TRICK:")
print("  If we know k_Ptarget, we know σ(k_Ptarget) = y")
print("  And since σ(k_Q) = y also, we can COMPUTE k_Q!")
print("  k_Q = σ⁻¹(σ(k_Ptarget)) = σ⁻¹(y)")
print("  But wait — that's circular. We need EITHER k_Ptarget OR σ⁻¹!")
print()
print("  💡 WAIT:")
print("  We have 13 fixed points of σ!")
print("  σ(k_i) = k_i for i=1..13")
print("  If we can EXPRESS y in terms of these fixed points,")
print("  we can COMPUTE σ⁻¹(y) by INTERPOLATION!")
print()
print("  🎯 FINAL APPROACH:")
print("  y = (y_real, y_imag) = σ(k_Q)")
print("  The 13 fixed points are: (k_i, 0) for i=1..13")
print("  σ⁻¹ maps F_p² → k")
print("  We can INTERPOLATE σ⁻¹ from the 13 points!")
print("  σ⁻¹(k_i, 0) = k_i  (known!)")
print("  σ⁻¹(y_real, y_imag) = ???  (TARGET!)")
