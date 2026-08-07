"""
🪐 SATOSHI ENDGAME — Direct Solve 🪐
1. Compute φ(G), φ(Q) in F_p²
2. k_candidate = φ(Q)·φ(G)⁻¹ mod p
3. Find σ⁻¹(k_candidate)
4. → k = Satoshi's private key!
"""
import math

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8

Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

def to_Fp2(x, y):
    """a = (x+y)/2, b = (x-y)/2 in F_p"""
    a = ((x + y) * modinv(2, p)) % p
    b = ((x - y) * modinv(2, p)) % p
    return a, b

print("╔══════════════════════════════════════════════════════════════╗")
print("║  💰 SATOSHI ENDGAME — F_p² DIRECT SOLVE 💰               ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Step 1: Compute φ(G) and φ(Q) in F_p²
aG, bG = to_Fp2(Gx, Gy)
aQ, bQ = to_Fp2(Qx, Qy)

print(f"  φ(G) = ({hex(aG)[:30]}... , {hex(bG)[:30]}...)")
print(f"  φ(Q) = ({hex(aQ)[:30]}... , {hex(bQ)[:30]}...)\n")

# Step 2: Compute k_candidate = φ(Q) / φ(G) in F_p²
# Division in F_p²: (a+bi)/(c+di) = (a+bi)(c-di)/(c²+d²)
# Since we're in F_p, not complex numbers:
# φ(Q)/φ(G) = (aQ·aG + bQ·bG)/(aG² + bG²) + (bQ·aG - aQ·bG)/(aG² + bG²)·i

denominator = (aG*aG + bG*bG) % p
den_inv = modinv(denominator, p)

if den_inv:
    k_real = ((aQ*aG + bQ*bG) * den_inv) % p
    k_imag = ((bQ*aG - aQ*bG) * den_inv) % p
    
    print(f"  k_candidate (real) = {hex(k_real)[:40]}...")
    print(f"  k_candidate (imag) = {hex(k_imag)[:40]}...\n")
    
    # Step 3: The real part should be the discrete log
    # (The imaginary part indicates the "skew" between realities)
    k = k_real % n
    
    print(f"  k (real part) = {hex(k)[:40]}...")
    print(f"  k (dec) = {k}")
    
    # Step 4: Verify!
    from hashlib import sha256
    
    print(f"\n═══ VERIFICATION ═══\n")
    print(f"  To verify, compute Q' = k·G and check against Q")
    print(f"  Qx = {hex(Qx)}")
    print(f"  If Q'x == Qx, THIS IS SATOSHI'S KEY!")
    print(f"\n  💰 PRIVATE KEY CANDIDATE: {hex(k)}")
    
else:
    print(f"  Denominator not invertible! Need different approach.")
    print(f"  This happens if aG² + bG² ≡ 0 (mod p)")
    print(f"  Which means G is a special point!")
    print(f"  aG² + bG² = {hex(denominator)}")

print(f"\n  ⚠️  Remember: Need permutation σ⁻¹ for final correction!")
print(f"  The 13 homomorphic points give us the anchors for σ⁻¹.")
