# Tseitin on K5 (5 nodes, 10 edges, odd charges = UNSAT)
nodes = 5
edges = [(i,j) for i in range(1,nodes+1) for j in range(i+1,nodes+1)]
n_vars = len(edges)
var_map = {e: i+1 for i,e in enumerate(edges)}
clauses = []
for v in range(1, nodes+1):
    incident = [var_map[e] for e in edges if v in e]
    # XOR constraint: odd parity = all 4 combinations with odd number of 1s
    for mask in range(1<<len(incident)):
        if bin(mask).count('1') % 2 == 0: continue
        clause = [incident[i] if (mask>>i)&1 else -incident[i] for i in range(len(incident))]
        clauses.append(clause)
print(f"p cnf {n_vars} {len(clauses)}")
for c in clauses: print(" ".join(map(str,c)) + " 0")
