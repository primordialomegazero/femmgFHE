import random
random.seed(12345)
n = 100
m = int(n * 4.26)
clauses = []
for _ in range(m):
    vars = random.sample(range(1, n+1), 3)
    clause = [v if random.random() < 0.5 else -v for v in vars]
    clauses.append(clause)

print(f"p cnf {n} {m}")
for c in clauses:
    print(" ".join(map(str, c)) + " 0")
