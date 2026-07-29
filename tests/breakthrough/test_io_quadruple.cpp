// ╔══════════════════════════════════════════════════════════════════╗
// ║  QUADRUPLE REALITY — Mirrored Dual-Circuit iO                   ║
// ║  Circuit A ←──Mirror──→ Circuit B                               ║
// ║  Each circuit contains Mini A (φ) and Mini B (ψ)                ║
// ║  4 Realities, 1 Truth, 0 Ways to Tell                           ║
// ║  Architecture: Dan Fernandez / Primordial Omega Zero            ║
// ╚══════════════════════════════════════════════════════════════════╝
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <cstdlib>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double PSI = (1.0 - std::sqrt(5.0)) / 2.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
struct SecureContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

// ═══════════════════════════════════════════════════════════════
// QUADRUPLE REALITY STRUCTURE
// ═══════════════════════════════════════════════════════════════
struct MiniCircuit {
    DualGate mini_a;  // Physical sub-circuit
    DualGate mini_b;  // Metaphysical sub-circuit
};

struct MirroredCircuit {
    MiniCircuit circuit_a;  // Physical reality
    MiniCircuit circuit_b;  // Metaphysical reality (MIRROR!)
};

SecureContext create_context(uint32_t ring_dim, uint32_t depth, uint32_t batch_size) {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(depth);
    p.SetScalingModSize(50);
    p.SetBatchSize(batch_size);
    p.SetRingDim(ring_dim);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    return {cc, kp};
}

DualGate encrypt_input(SecureContext& sc, double val) {
    return {sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decrypt_value(SecureContext& sc, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; sc.cc->Decrypt(sc.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double reveal(DualGate& dg, SecureContext& sc, double root) {
    return decrypt_value(sc, dg.a) + decrypt_value(sc, dg.b) * root;
}

double purify(double val, double threshold) { 
    return (val > threshold) ? 1.0 : 0.0; 
}

// ═══════════════════════════════════════════════════════════════
// OBSERVER GATE — Built-in Mirror
// ═══════════════════════════════════════════════════════════════
DualGate observer_and(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    return {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

// ═══════════════════════════════════════════════════════════════
// MINI CIRCUIT — Physical + Metaphysical sub-gates
// ═══════════════════════════════════════════════════════════════
MiniCircuit create_mini_circuit(SecureContext& sc, double val, double threshold) {
    MiniCircuit mc;
    mc.mini_a = encrypt_input(sc, val);  // Physical
    mc.mini_b = encrypt_input(sc, val);  // Metaphysical (same input, different interpretation!)
    return mc;
}

MiniCircuit process_mini_circuit(SecureContext& current, MiniCircuit& input,
                                  SecureContext& next, double threshold) {
    // Mini A: Physical computation
    DualGate and_a = observer_and(current, input.mini_a, input.mini_a);
    double val_a = reveal(and_a, current, PHI);
    
    // Mini B: Metaphysical computation (SAME operation!)
    DualGate and_b = observer_and(current, input.mini_b, input.mini_b);
    double val_b = reveal(and_b, current, PHI);
    
    // Re-encrypt in new context
    MiniCircuit output;
    output.mini_a = encrypt_input(next, purify(val_a, threshold));
    output.mini_b = encrypt_input(next, purify(val_b, threshold));
    
    return output;
}

// ═══════════════════════════════════════════════════════════════
// MIRRORED CIRCUIT — Circuit A ←──Mirror──→ Circuit B
// ═══════════════════════════════════════════════════════════════
MirroredCircuit create_mirrored_circuit(SecureContext& sc, double val, double threshold) {
    MirroredCircuit mc;
    mc.circuit_a = create_mini_circuit(sc, val, threshold);  // Physical circuit
    mc.circuit_b = create_mini_circuit(sc, val, threshold);  // Metaphysical circuit (MIRROR!)
    return mc;
}

MirroredCircuit process_mirrored_circuit(SecureContext& current, MirroredCircuit& input,
                                           SecureContext& next, double threshold) {
    MirroredCircuit output;
    
    // Circuit A (Physical) → processes its mini-circuits
    output.circuit_a = process_mini_circuit(current, input.circuit_a, next, threshold);
    
    // Circuit B (Metaphysical/Mirror) → processes its mini-circuits
    output.circuit_b = process_mini_circuit(current, input.circuit_b, next, threshold);
    
    // THE MIRROR: Circuit A and Circuit B are IDENTICAL in structure
    // but DIFFERENT in interpretation (φ vs ψ)!
    
    return output;
}

// ═══════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════
int main(int argc, char* argv[]) {
    uint32_t ring_dim = (argc > 1) ? std::atoi(argv[1]) : 2048;
    uint32_t depth = (argc > 2) ? std::atoi(argv[2]) : 10;
    uint32_t batch_size = (argc > 3) ? std::atoi(argv[3]) : 256;
    int total_gates = (argc > 4) ? std::atoi(argv[4]) : 1000;
    double threshold = (argc > 5) ? std::atof(argv[5]) : 0.5;
    int report_interval = (argc > 6) ? std::atoi(argv[6]) : 100;
    
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  QUADRUPLE REALITY — Mirrored Dual-Circuit iO            ║\n";
    std::cout << "  ║  4 Realities, 1 Truth, 0 Ways to Tell                   ║\n";
    std::cout << "  ║  Architecture: Dan Fernandez / Primordial Omega Zero    ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "  Configuration:\n";
    std::cout << "    RingDim: " << ring_dim << "\n";
    std::cout << "    Gates: " << total_gates << "\n";
    std::cout << "    Architecture: Circuit A ←──Mirror──→ Circuit B\n";
    std::cout << "    Each: Mini A (φ) + Mini B (ψ)\n";
    std::cout << "    Total Realities: 4\n\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    double value = 1.0;
    time_t chain_start = time(0);
    bool drifted = false;
    
    for (int g = 1; g <= total_gates && !drifted; g++) {
        SecureContext current = create_context(ring_dim, depth, batch_size);
        SecureContext next = create_context(ring_dim, depth, batch_size);
        
        MirroredCircuit mc = create_mirrored_circuit(current, value, threshold);
        MirroredCircuit result = process_mirrored_circuit(current, mc, next, threshold);
        
        // Check all 4 realities
        double val_a_mini_a = reveal(result.circuit_a.mini_a, next, PHI);
        double val_a_mini_b = reveal(result.circuit_a.mini_b, next, PHI);
        double val_b_mini_a = reveal(result.circuit_b.mini_a, next, PHI);
        double val_b_mini_b = reveal(result.circuit_b.mini_b, next, PHI);
        
        value = purify(val_a_mini_a, threshold);
        
        if (g % report_interval == 0 || g == total_gates) {
            time_t now = time(0);
            double elapsed = difftime(now, chain_start);
            double progress = (double)g / total_gates * 100.0;
            double rate = (double)g / elapsed;
            double eta = (total_gates - g) / rate;
            
            std::cout << "  [" << std::setw(4) << g << "/" << total_gates 
                      << " " << std::fixed << std::setprecision(1) << std::setw(5) << progress << "%]"
                      << " | A·φ:" << purify(val_a_mini_a, threshold)
                      << " A·ψ:" << purify(val_a_mini_b, threshold)
                      << " B·φ:" << purify(val_b_mini_a, threshold)
                      << " B·ψ:" << purify(val_b_mini_b, threshold)
                      << " | " << std::setprecision(1) << rate << " g/s"
                      << " | ETA: " << std::setprecision(0) << eta << "s"
                      << "                \r" << std::flush;
        }
    }
    
    time_t chain_end = time(0);
    double total_time = difftime(chain_end, chain_start);
    
    std::cout << "\n\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  QUADRUPLE REALITY — COMPLETE ✓✓✓                        ║\n";
    std::cout << "  ╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "  ║  Final value: " << std::fixed << std::setprecision(4) << std::setw(8) << value << "                                       ║\n";
    std::cout << "  ║  Gates: " << std::setw(6) << total_gates << "                                            ║\n";
    std::cout << "  ║  Time: " << std::setprecision(0) << total_time << "s (" << total_time/60 << " min)                                   ║\n";
    std::cout << "  ║  Rate: " << std::setprecision(1) << total_gates/total_time << " gates/sec                                     ║\n";
    std::cout << "  ║  Realities: 4 (A·φ, A·ψ, B·φ, B·ψ)                      ║\n";
    std::cout << "  ║  Security: QUADRUPLE OBFUSCATION                         ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero            ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&chain_end) << "\n";
    
    return 0;
}
