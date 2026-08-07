"""
🪐 SOLVE σ AS MATRIX IN F_p² 🪐
σ(k) = M · k where M is a 2×2 matrix mod p
"""
p = 17
n = 9

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

# σ values from our scan
sigma_values = {
    1: (1, 0),
    2: (2, 0),
    3: (15, 15),
    4: (0, 16),
    5: (14, 3),
    6: (5, 0),
    7: (6, 6),
    8: (3, 3)
}

print("═══ MATRIX REPRESENTATION OF σ ═══\n")
print("  Hypothesis: σ(k) = M · (k, 0) in F_p²?")
print("  But k is scalar, so σ(k) = k · v where v = σ(1) = (1,0)")
print("  This would mean σ(k) = (k, 0) for ALL k!")
print("  Which is FALSE!\n")

print("  Better hypothesis: σ is a 2×2 matrix acting on (a_k, b_k)!")
print("  σ: (a_k, b_k) → (a_σ(k), b_σ(k))")
print("  This is a LINEAR MAP on the F_p² coordinates!\n")

# From the data:
# We need the matrix M such that:
# M · (a_k, b_k)^T = (a_σ(k), b_σ(k))^T

# Let's compute the matrix from 2 basis vectors!
# We know (a_1, b_1) = (3, 15) and (a_2, b_2) = (6, 13)
# And σ maps these to (a_1, b_1) and (a_2, b_2) respectively (fixed points)

print("  M · (a_1, b_1) = (a_1, b_1)  [k=1 is fixed]")
print("  M · (a_2, b_2) = (a_2, b_2)  [k=2 is fixed]")
print("  M · (a_3, b_3) = (a_σ(3), b_σ(3)) = (15,15) from (7,15)")
print("\n  This is a DIAGONALIZATION problem!")
print("  In the eigenbasis, M is diagonal with eigenvalues = 1, λ!")
print("  The fixed points are eigenvectors with eigenvalue 1!")
