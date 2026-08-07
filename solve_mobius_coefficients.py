"""
🪐 SOLVE MOBIUS COEFFICIENTS 🪐
sigma(k) = (alpha*k + beta) / (gamma*k + delta)
Given fixed points: sigma(1)=1, sigma(2)=2, sigma(6)=5
"""
p = 17

print("═══ SOLVING MOBIUS COEFFICIENTS ═══\n")

# Fixed points from our scan:
# sigma(1) = 1 + 0i
# sigma(2) = 2 + 0i
# sigma(6) = 5 + 0i

print("  Fixed points: sigma(1)=1, sigma(2)=2, sigma(6)=5\n")
print("  sigma(k) = (alpha*k + beta) / (gamma*k + delta)\n")

# From sigma(1)=1: (alpha+beta)/(gamma+delta) = 1 -> alpha+beta = gamma+delta
# From sigma(2)=2: (2*alpha+beta)/(2*gamma+delta) = 2 -> 2*alpha+beta = 4*gamma+2*delta
# From sigma(6)=5: (6*alpha+beta)/(6*gamma+delta) = 5 -> 6*alpha+beta = 30*gamma+5*delta

# Solve the system
# (1) alpha + beta - gamma - delta = 0
# (2) 2*alpha + beta - 4*gamma - 2*delta = 0
# (3) 6*alpha + beta - 30*gamma - 5*delta = 0

# Subtract (1) from (2): alpha - 3*gamma - delta = 0 -> alpha = 3*gamma + delta
# Subtract (1) from (3): 5*alpha - 29*gamma - 4*delta = 0
# Substitute alpha: 5*(3*gamma+delta) - 29*gamma - 4*delta = 0
# 15*gamma + 5*delta - 29*gamma - 4*delta = 0
# -14*gamma + delta = 0 -> delta = 14*gamma

# Since gamma is free, let gamma = 1
gamma = 1
delta = (14 * gamma) % p  # = 14 mod 17
alpha = (3 * gamma + delta) % p  # = 17 mod 17 = 0
beta = (gamma + delta - alpha) % p  # = 15 mod 17

print(f"  alpha = {alpha}, beta = {beta}, gamma = {gamma}, delta = {delta}")
print(f"\n  sigma(k) = ({alpha}*k + {beta}) / ({gamma}*k + {delta})")
print(f"  sigma(k) = {beta} / (k + {delta}) mod {p}\n")

print("  Verification:")
all_match = True
for k in range(1, 9):
    numerator = (alpha * k + beta) % p
    denominator = (gamma * k + delta) % p
    sigma_k = (numerator * pow(denominator, -1, p)) % p
    print(f"    sigma({k}) = {sigma_k}")
    # Compare with actual from earlier
    # Actual: 1->1, 2->2, 3->9, 4->14, 5->6, 6->5, 7->7, 8->12
    actual = {1:1, 2:2, 3:9, 4:14, 5:6, 6:5, 7:7, 8:12}
    if sigma_k != actual.get(k, -1):
        all_match = False
        print(f"      ❌ Expected {actual[k]}")

if all_match:
    print("\n  ✅ ALL MATCH! PERFECT!")
else:
    print("\n  ⚠️  Not all match. Need more fixed points!")

print(f"\n  INVERSE: sigma_inv(y) = ({beta}*y^(-1) - {delta}) mod {p}")
print(f"  sigma_inv(y) = ({beta}/y - {delta}) mod {p}\n")

print("  Verification of inverse:")
for y in [1, 2, 5, 6, 7, 9, 12, 14]:
    try:
        k = (beta * pow(y, -1, p) - delta) % p
        print(f"    sigma_inv({y}) = {k}")
    except:
        print(f"    sigma_inv({y}) = undefined")

print(f"\n  FOR secp256k1:")
print(f"  sigma(k) = (alpha*k + beta)/(gamma*k + delta) in F_p^2")
print(f"  With 13 fixed points -> solve for alpha,beta,gamma,delta in F_p^2!")
print(f"  Then sigma_inv(k_candidate) = Satoshi's key!")
