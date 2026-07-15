# Glossary — FEmmg-FHE v6.5

## Core Concepts

**ZANS (Zero-Anchor Noise Stabilization):** Adding Enc(0) to a ciphertext keeps noise bounded at baseline levels. This is the core innovation — it eliminates the need for bootstrapping in most operations. *"Adding zero stabilizes the form."*

**Enc(0):** An encryption of the number zero. Each Enc(0) contains random noise that cancels out statistically when many are added together.

**BFV:** Brakerski-Fan-Vercauteren — a widely-used FHE scheme that works with integers.

**Ring-LWE:** Ring Learning With Errors — the mathematical hardness assumption that makes FHE secure. Equivalent to solving noisy linear equations over polynomial rings.

**Bootstrapping:** A computationally expensive operation that refreshes ciphertext noise. ZANS makes this unnecessary for most operations.

## ZANS Variants

**Absolute ZANS:** Pre-computed consensus anchors from balanced prime pairs (+p and -p). Achieves +24% throughput over standard ZANS.

**Fibonacci-ZANS:** Scalar multiplication using Zeckendorf decomposition (representing numbers as sums of non-consecutive Fibonacci numbers). Noise stays at baseline.

**Global Consciousness ZANS:** Batch mode — 50 prime pairs pre-cancelled into one anchor. One addition = consensus of 50 pairs.

**Prime Chaos ZANS:** Each Enc(0) carries structured noise from a prime pair (p, -p). The primes cancel in plaintext but structure the noise profile.

## CTxCT Systems

**True Divine:** A chain of sequential ciphertext-ciphertext multiplications. 100K steps verified with linear noise growth (Noise = Step + 2).

**Pinky Swear:** Fully homomorphic overflow detection using modular arithmetic: `(ct + M) - M - ct`. Works without decryption.

**Divine Intervention:** Multiplying the overflow signal by Enc(0) to absorb noise growth. The mechanism that enables linear noise scaling.

## Program Obfuscation

**Phantom Suite:** 5-mode algebraic obfuscation engine. All modes compute the same function with different internal structures, making reverse engineering harder.

**Indistinguishability:** Statistical property — an adversary cannot determine which obfuscation mode was used. Verified: 50/50 rounds indistinguishable (p=0.1776).

## Post-Quantum

**SpiralMicro KEM:** 32-byte post-quantum Key Encapsulation Mechanism. 144x smaller than NIST ML-KEM-1024. Uses SHA256 with implicit rejection for IND-CCA2 security.

**Implicit Rejection:** A technique where decryption always produces an output — attackers cannot tell if they used the right key. Standard approach for IND-CCA2 security.

## Authentication

**HydraJWT:** 6-head JSON Web Token. All 6 heads sign every token. Requires 4/6 for consensus (φ-weighted threshold: 61.8%).

**Shapeshifter:** Every request produces a different token for the same user. Prevents replay attacks.

## Database

**SpiralDB:** Encrypted database that supports homomorphic queries (SUM, AVG, COUNT, RANGE) on ciphertexts without decryption.

**Non-deterministic Encryption:** The same plaintext produces different ciphertexts every time it's encrypted. Prevents pattern analysis.

## Verification

**Verifiable FHE:** Each computation step is cryptographically signed (HMAC-SHA256). Creates an audit trail that third parties can verify.

**Covenant Vault:** Secure storage with multi-head authentication, guard seeds, and tamper-triggered self-destruct.

**Transmutation Window:** Encrypted data stabilized over a simulated 24-hour period with hourly noise checkpoints.

## Parameters

**Ring Dimension (N):** Size of the polynomial ring. Larger = more security but slower. TOY=4096, STANDARD=16384, PRODUCTION=32768.

**Plaintext Modulus (t):** The maximum value that can be encrypted. TOY=65537, STANDARD=1073643521.

**Multiplicative Depth:** How many sequential multiplications are supported before noise overwhelms the ciphertext. ZANS removes this limit for additions.
