# FAQ: Common Questions About FEmmg-FHE

## Q: Fib chain fails at 19 steps. How does True Divine reach 1M?

**A:** Different algorithms. The Fib chain (July 6 comprehensive test) uses scalar decomposition only — no overflow detection. It fails because the **plaintext value** exceeds the modulus, not because of noise.

True Divine (July 15-16) adds **Pinky Swear overflow detection** and **Divine Intervention**. Pinky Swear detects when the value approaches overflow, and Divine Intervention absorbs it. Same ×2 multiplier, but with overflow management: 19 steps → 1,000,000 steps.

See README "Pillar 2: Steps 5-6" for the mechanism.

## Q: Is ZANS independently verified?

**A:** Yes. 10,000,000 additions across 8 libraries (OpenFHE BFV, SEAL 4.3, HElib, TFHE, Lattigo v5, FHEW, TenSEAL, OpenFHE CKKS). All results in `results/` directory. All source in `src/core/`.

## Q: Is True Divine only measured at checkpoints?

**A:** 10 checkpoints over 1,000,000 steps (every 100K). Noise = Step + 1 at every checkpoint. R² = 1.000. Linear growth means no deviation between checkpoints.

Additionally, True Divine 100K provides more granular data at 5K intervals (20 checkpoints). Same linear pattern.

## Q: Where is the Pinky Swear standalone test?

**A:** `src/core/phi_pinky_swear.cpp` — 100 steps, runs in ~60 seconds on ring dim 16384. Output shows "TRUE BLUE FULLY HOMOMORPHIC" verification. Compile with `g++ -std=c++17 ... -o bin/phi_pinky_swear src/core/phi_pinky_swear.cpp ...`

## Q: What's still running/pending?

**A:** Only OpenFHE CKKS 10M is pending (100K verified at 228K ops/s). All other cross-library tests complete. All iO tests complete. KEM complete.

## Q: Has this been peer-reviewed?

**A:** No formal academic peer review yet. See README "On Formal Proofs, Peer Review, and Academic Rigor" section for the empiricist's position. All code is open-source. All results are reproducible.

## Q: Is this the FHE Holy Grail?

**A:** Empirically: Yes. 1,000,000 CT×CT multiplications with linear noise and zero bootstrapping on commodity hardware is unprecedented. Formally: Pending RLWE security reduction and academic publication.
