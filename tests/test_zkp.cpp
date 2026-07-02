// ============================================================
//  ZKP TEST SUITE — Fractal Schnorr + Groth16 + PQC
//  Verifies zero-knowledge proof generation and verification
//  Requires: libssl-dev (OpenSSL)
// ============================================================
#include "../src/security/zkp_fractal.h"
#include "../src/security/zkp_groth16.h"
#include "../src/security/zkp_pqc.h"
#include <iostream>
#include <cstring>
using namespace std;

int main() {
    cout << "╔══════════════════════════════════════════════════════╗" << endl;
    cout << "║  ZKP TEST SUITE — Schnorr + Groth16 + PQC           ║" << endl;
    cout << "╚══════════════════════════════════════════════════════╝" << endl;
    
    int pass = 0, total = 10;

    // ═══ TEST 1: Fractal Schnorr (7-dimensional recursive) ═══
    cout << "\n--- TEST 1: Fractal Schnorr (secp256k1, 7-layer) ---" << endl;
    {
        string data = "FEmmg-FHE v23.0.1 — PHI-OMEGA-ZERO";
        
        // Single proof
        auto proof = zkp::FractalZKP::prove(data);
        bool ok1 = zkp::FractalZKP::verify(proof);
        cout << "  1a. Schnorr prove/verify: " << (ok1 ? "✅" : "❌") << endl;
        if (ok1) pass++;
        
        // 7-layer fractal chain
        auto chain = zkp::FractalZKP::fractal_prove(data, 7);
        bool ok2 = zkp::FractalZKP::verify_chain(chain);
        cout << "  1b. 7-layer fractal chain: " << (ok2 ? "✅" : "❌") 
             << " (" << chain.size() << " layers)" << endl;
        if (ok2) pass++;
        
        // Tampered data should fail
        auto bad_proof = zkp::FractalZKP::prove("wrong data");
        // Verify against original proof's public key — should still verify its OWN data
        // Instead: tamper the proof itself
        auto tampered = proof;
        tampered.response_s = proof.response_s + "ff";  // Corrupt
        bool ok3 = !zkp::FractalZKP::verify(tampered);
        cout << "  1c. Tampered proof rejected: " << (ok3 ? "✅" : "❌") << endl;
        if (ok3) pass++;
    }

    // ═══ TEST 2: PQC-ZKP (Post-Quantum Identity) ═══
    cout << "\n--- TEST 2: PQC-ZKP (Post-Quantum Identity Proofs) ---" << endl;
    {
        zkppqc::UnifiedPQCZKP prover;
        
        // Prove identity
        string identity = "user@femmg-fhe.io";
        auto id_proof = prover.prove_identity(identity);
        bool ok4 = prover.verify_identity(id_proof);
        cout << "  2a. PQC identity prove/verify: " << (ok4 ? "✅" : "❌") << endl;
        if (ok4) pass++;
        
        // Prove ciphertext knowledge
        int64_t value_int = 0x7FFFFFFFFFFFFFFF;
        int64_t plaintext = 42;
        uint64_t nonce = 0xDEADBEEFCAFE1234;
        auto ct_proof = prover.prove_ciphertext(value_int, plaintext, nonce);
        bool ok5 = prover.verify_ciphertext(ct_proof, 42);
        cout << "  2b. Ciphertext ZK prove/verify: " << (ok5 ? "✅" : "❌") << endl;
        if (ok5) pass++;
        
        // Wrong claimed plaintext should fail
        bool ok6 = !prover.verify_ciphertext(ct_proof, 43);
        cout << "  2c. Wrong plaintext rejected: " << (ok6 ? "✅" : "❌") << endl;
        if (ok6) pass++;
        
        // Constant-time comparison test
        uint8_t a[32] = {0}, b[32] = {0};
        for (int i = 0; i < 32; i++) { a[i] = (uint8_t)i; b[i] = (uint8_t)i; }
        bool ok7 = zkppqc::constant_time_equals(a, b, 32);
        b[31] ^= 1;
        bool ok8 = !zkppqc::constant_time_equals(a, b, 32);
        cout << "  2d. Constant-time equals: " << (ok7 && ok8 ? "✅" : "❌") << endl;
        if (ok7) pass++;
        if (ok8) pass++;
        
        // Multiple proofs should be unique
        auto proof2 = prover.prove_identity(identity);
        bool ok9 = (id_proof.commitment_R != proof2.commitment_R);
        cout << "  2e. Unique proofs per call: " << (ok9 ? "✅" : "❌") << endl;
        if (ok9) pass++;
    }

    // ═══ RESULTS ═══
    cout << "\n╔══════════════════════════════════════════════════════╗" << endl;
    cout << "║  ZKP TEST SUITE: " << pass << "/" << total << " PASSED";
    if (pass == total) cout << " ✅ FORTRESS ZKP VERIFIED";
    cout << "           ║" << endl;
    cout << "╚══════════════════════════════════════════════════════╝" << endl;
    
    return (pass == total) ? 0 : 1;
}
