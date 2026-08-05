import random, sys
random.seed(42)
n = 100
m = 426
print(f"p cnf {n} {m}")
for _ in range(m):
    vs = random.sample(range(1, n+1), 3)
    print(" ".join(str(v if random.random()<0.5 else -v) for v in vs) + " 0")
