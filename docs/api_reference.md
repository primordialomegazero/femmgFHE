# Spiral Fractal iO — API Reference

## Core Encryption Modules

### GFNEncryption (Golden Fibonacci N-Layer Encryption)

**Header:** `src/config/gf_n_encryption.h`

**Description:** N-layer stacked Golden Fibonacci encryption. Each layer uses an independent seed from the Hierarchical Seed Tree with unique Cassini invariant > 0.1.

**Initialization:**
```cpp
GFNEncryption fne;
fne.init(master_seed, N_layers);           // N = 1 (DEV) to 100 (UNIVERSE)
fne.init_enterprise(master_seed, N_layers); // With unique branch per layer
fne.init_from_config(system_config);        // From SystemConfig
```

**Encryption:**
```cpp
auto ct = fne.encrypt(plaintext);           // Returns CipherText {y1, y2_trail}
auto [y1, y2] = fne.encrypt_pair(plaintext); // Returns pair {y1, avg(y2)}
```

**Decryption:**
```cpp
double plaintext = fne.decrypt(ct);         // Requires original y2_trail
```

**Security Levels:**
| Level | N | Use Case |
|-------|---|----------|
| STANDARD | 1 | Development |
| ELEVATED | 3 | Testing |
| ENTERPRISE | 5 | Production |
| MILITARY | 10 | High Security |
| UNIVERSE | 100 | Maximum |

---

### GoldenFibonacci (Single Layer)

**Header:** `src/crypto/golden_fibonacci.h`

**Description:** Single-layer Golden Fibonacci encryption using Fibonacci-like sequence and matrix multiplication over mod 1.

**Initialization:**
```cpp
GoldenFibonacci gf;
gf.init(master_seed, n = 50);              // n >= 50
gf.init_with_params(seed, n, min_cassini, max_retries);
```

**Key Properties:**
- `gf.cassini`: Cassini invariant (must be > 0.1)
- `gf.secret_seed`: Derived encryption seed
- `gf.power_n`: Sequence length used

**Encrypt/Decrypt:**
```cpp
auto [y1, y2] = gf.encrypt(plaintext);     // Matrix encryption
double x = gf.decrypt(y1, y2);             // Matrix inversion via Cassini
```

---

### SpiralBootstrap (Encrypted Noise Reset)

**Header:** `src/refresh/spiral_bootstrap.h`

**Description:** Complete encrypted bootstrapping mechanism. Performs CKKS Decrypt → GF Ciphertext (never plaintext) → GF ReEncrypt → CKKS ReEncrypt with fresh noise budget. Protected by 3-phase Spiral Obfuscation.

**Initialization:**
```cpp
SpiralBootstrap sb;
sb.init(master_seed, gf_layers = 5, obfuscate = false);
sb.init_from_config(master_seed, system_config);
```

**Bootstrap Operations:**
```cpp
// Full bootstrap with spiral obfuscation
auto fresh_ct = sb.bootstrap(encrypted_ct, secure_context);

// Quick bootstrap (no obfuscation, faster)
auto fresh_ct = sb.quick_bootstrap(encrypted_ct, secure_context);
```

**Configurable Parameters (all N-configurable, Fibonacci-scaled):**
| Parameter | DEV | PROD | ENTERPRISE |
|-----------|-----|------|------------|
| N_gf_layers | 1 | 5 | 13 |
| N_spiral_rounds | 5 | 13 | 21 |
| N_spiral_depth | 15 | 39 | 63 |
| N_timing_iterations | 3 | 8 | 13 |
| N_obfuscation_layers | 5 | 13 | 21 |

**State Management:**
```cpp
sb.store_gf_state(ct);                     // Store y2_trail for decryption
bool ok = sb.verify_cassini();             // Check all layers invertible
std::string status = sb.status();          // Human-readable status
```

---

### FractalRefresh (iO Obfuscation Engine)

**Header:** `src/refresh/emergent_refresh.h`

**Description:** 7-step obfuscation engine: Emergent Timing → Fractal Transform → Random Permutation → Commutative Reconstruction → GF ReEncrypt → FHE ReEncrypt. Used by the iO layer for cross-circuit obfuscation.

**Initialization:**
```cpp
FractalRefresh fr;
fr.init(master_seed, N = 23, depth = 7, input_weight = 0.618);
fr.init_from_config(master_seed, system_config);
```

**Refresh:**
```cpp
auto [np, npsi] = fr.refresh(phi, psi);    // Obfuscate {phi, psi} pair
```

**Controller:**
```cpp
EmergentRefreshController ctrl;
ctrl.init_from_config(system_config);
bool should = ctrl.should_refresh(phi, psi, gate_count);
```

---

## FHE Core

### SecureContext (CKKS FHE Wrapper)

**Header:** `src/fhe/fhe_core.h`

**Description:** Wrapper around OpenFHE CKKS homomorphic encryption scheme.

**Creation:**
```cpp
auto sc = create_fhe_context(ring_dim, depth);
// ring_dim: 2048, 4096, 8192, 16384, 32768
// depth: 60, 120, 200, 300
```

**Encrypt/Decrypt:**
```cpp
DualGate ct = enc(sc, value);              // Returns {a, b} pair
double pt = dec(sc, ciphertext);           // Decrypt single ciphertext
```

**DualGate Projections:**
```cpp
double phi = phi_val(gate, sc);            // φ(a,b) = a + b·φ
double psi = psi_val(gate, sc);            // ψ(a,b) = a + b·ψ
```

**Homomorphic Gates:**
```cpp
DualGate nand_result = nand_op(sc, X, Y);  // NAND gate
DualGate and_result = and_op(sc, X, Y);    // AND = NAND(NAND(X,Y))
DualGate or_result = or_op(sc, X, Y);      // OR = NAND(NOT(X), NOT(Y))
DualGate not_result = not_op(sc, X);       // NOT = NAND(X, X)
DualGate xor_result = xor_op(sc, X, Y);    // XOR from AND/OR/NOT
```

---

## iO Compiler

### iOCompiler (Circuit Evaluation)

**Header:** `src/io/universal_compiler.h`

**Description:** Compiles and evaluates Boolean circuits in the encrypted domain using DualGate NAND operations.

**Circuit Definition:**
```cpp
Circuit cA;
cA.name = "A";
cA.num_inputs = 3;
cA.gates = {
    {Gate::AND, 0, 1},  // Gate 1: AND(input 0, input 1)
    {Gate::OR, 3, 2}    // Gate 2: OR(wire 3, input 2)
};
```

**Evaluation:**
```cpp
iOCompiler compiler(secure_context);
DualGate result = compiler.evaluate(circuit, encrypted_inputs);
```

**iO Encoding:**
```cpp
DualGate obfuscated = compiler.iO_encode(result_A, result_B);
bool equivalent = compiler.verify_equivalence(circuit_A, circuit_B);
```

---

## Database Modules

### SpiralFractalDB (Core Database)

**Header:** `src/database/spiral_fractal_db.h`

**Description:** SQLite3-based encrypted database with AES-256-GCM, 7-Layer Fractal Index, N-Mirror mode, and audit logging.

**Initialization:**
```cpp
SpiralFractalDB db;
db.init("database.db", "passphrase", enable_fhe = false, mirror_count = 3, mirror_threshold = 2);
```

**CRUD Operations:**
```cpp
db.put(key, value);                        // Encrypted write
std::string val = db.get(key, default);    // Decrypted read
bool exists = db.exists(key);
db.remove(key);
```

**Batch Operations:**
```cpp
db.put_batch({{k1, v1}, {k2, v2}, ...});  // Batch write
auto results = db.get_prefix("user:");     // Prefix query
```

**Snapshots:**
```cpp
db.create_snapshot("backup_001");          // Create backup
```

---

### HarmonizedObfuscation (Multi-DB Pipeline)

**Header:** `src/database/harmonized_obfuscation.h`

**Description:** Unified obfuscation layer for PostgreSQL, Redis, and VectorDB — all passing through AES-256-GCM encryption.

**Initialization:**
```cpp
HarmonizedObfuscation harm;
harm.init("harmonized.db", "passphrase", vector_dim = 128);
```

**PostgreSQL-style:**
```cpp
harm.pg_put("table", "key", "value");
std::string val = harm.pg_get("table", "key");
```

**Redis-style:**
```cpp
harm.redis_set("key", "value", ttl_seconds);
std::string val = harm.redis_get("key");
```

**VectorDB-style:**
```cpp
harm.vec_insert("id", {1.0, 0.0, 0.0, 0.0});
auto results = harm.vec_search({1.0, 0.0, 0.0, 0.0}, top_k = 10);
```

**Key Management:**
```cpp
std::string key_hex = harm.export_key();
harm.import_key(key_hex);
harm.save_key_to_file("key.hex");
harm.load_key_from_file("key.hex");
```

---

## Authentication & Security

### SpiralFractalAuth (HydraJWT)

**Header:** `src/database/spiral_fractal_auth.h`

**Description:** 6-head post-quantum JWT authentication using HydraJWT (Schnorr, Falcon-1024, ML-DSA-87, SLH-DSA, ECDSA, φ-HMAC).

**Initialization:**
```cpp
SpiralFractalAuth auth;
auth.init(&db, config);
```

**Token Operations:**
```cpp
auth.create_token("user_id", token_out);
bool ok = auth.verify_token(token, user_id_out);
```

**Authenticated DB Access:**
```cpp
auth.authenticated_put(token, "key", "value");
std::string val = auth.authenticated_get(token, "key");
```

---

### SpiralFractalDefense (Blackhole Active)

**Header:** `src/database/spiral_fractal_defense.h`

**Description:** Active intrusion countermeasures: honeypots, recursive trapdoors, memory poisoning, attacker fingerprinting.

**Initialization:**
```cpp
SpiralFractalDefense defense;
defense.init(&db, config);
```

**Defense Operations:**
```cpp
std::string hp = defense.deploy_honeypot();
bool detected = defense.detect_intrusion("honeypot:" + hp);
defense.respond_to_intrusion();
std::string fp = defense.fingerprint_attacker(pattern);
```

---

## Configuration

### SystemConfig (45 N-Configurable Parameters)

**Header:** `src/config/system_config.h`

**Preset Modes:**
```cpp
SystemConfig cfg;
cfg.set_dev_mode();        // RingDim=2048, 1 GF layer, 3 variants
cfg.set_test_mode();       // RingDim=4096, 3 GF layers, 5 variants
cfg.set_prod_mode();       // RingDim=32768, 5 GF layers, 12 variants
cfg.set_enterprise_mode(); // RingDim=65536, 13 GF layers, 23 variants
cfg.derive_all();          // Auto-compute master_seed, pairs, gates
```

**Manual Override:**
```cpp
cfg.N_ring_dim = 32768;
cfg.N_fractal_layers = 23;
cfg.N_variants = 12;
cfg.derive_all();          // Recompute dependent values
```

---

## Metaprogramming

### SpiralMetaConfig (Compile-Time Optimizer)

**Header:** `src/metaprogramming/spiral_meta_optimizer.h`

**Description:** Compile-time auto-tuning of all Spiral Bootstrap parameters from target RingDim and SecurityLevel. Zero runtime overhead.

**Usage:**
```cpp
// All parameters computed at COMPILE TIME
using Config = SpiralMetaConfig<32768, 2>;  // RingDim=32768, PROD
// Config::N_gf_layers = 5
// Config::N_spiral_rounds = 13
// Config::N_spiral_depth = 39
// Config::N_timing_iterations = 8
```

**Pre-made Configs:**
```cpp
SpiralDevConfig   // RingDim=2048, DEV
SpiralTestConfig  // RingDim=4096, TEST
SpiralProdConfig  // RingDim=32768, PROD
SpiralEntConfig   // RingDim=65536, ENTERPRISE
```

---

## Statistical Validation

### AdvancedStatisticalValidator (KS Test)

**Header:** `src/production/advanced_stats.h`

**Description:** Kolmogorov-Smirnov test for iO indistinguishability validation with real-time progress tracking.

**Usage:**
```cpp
AdvancedStatisticalValidator adv;
adv.init(samples = 100, ks_threshold = 0.05, report_interval = 10);
adv.add_sample(value_A, value_B);
std::string verdict = adv.final_verdict();
double ks = adv.compute_ks(dist_A, dist_B);
bool passed = adv.passed();
```

**Verdict Thresholds:**
| KS Range | Verdict |
|----------|---------|
| < 0.01 | iO-SECURE (Excellent) |
| < 0.05 | iO-SECURE (Pass) |
| < 0.10 | NEEDS MORE SAMPLES |
| >= 0.10 | DISTINGUISHABLE (Fail) |

---

## CLI Tool

**Binary:** `./bin/femmgfhe`

```bash
# Encryption
./bin/femmgfhe encrypt "hello" "passphrase"

# Decryption
./bin/femmgfhe decrypt "ciphertext_hex" "passphrase"

# Benchmark
./bin/femmgfhe benchmark

# Key management
./bin/femmgfhe export-key "key.hex"
./bin/femmgfhe import-key "key.hex"

# Status
./bin/femmgfhe status
```

---

*Complete API reference for Spiral Fractal iO v28.0*
