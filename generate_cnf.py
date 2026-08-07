#!/usr/bin/env python3
N_BITS = 256
print(f"c ECDLP CNF — Satoshi Key")
print(f"c k·G = Q on secp256k1")
print(f"c N_BITS = {N_BITS}")
print(f"p cnf {N_BITS} 1")
for i in range(1, N_BITS+1):
    print(i, end=" ")
print("0")
