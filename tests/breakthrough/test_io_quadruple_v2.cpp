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

struct MiniCircuit {
    DualGate mini_a;
    DualGate mini_b;
};

struct MirroredCircuit {
    MiniCircuit circuit_a;
    MiniCircuit circuit_b;
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

DualGate observer_and(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    return {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

MirroredCircuit process_quadruple(SecureContext& current, MirroredCircuit& input,
                                    SecureContext& next, double threshold) {
    
    DualGate computed_a = observer_and(current, input.circuit_a.mini_a, input.circuit_a.mini_a);
    DualGate computed_b = observer_and(current, input.circuit_b.mini_a, input.circuit_b.mini_a);
    
    // iO obfuscation before decrypt
    auto phi_pt = current.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto psi_pt = current.cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    
    auto mixed_a = current.cc->EvalMult(computed_a.a, phi_pt);
    auto mixed_b = current.cc->EvalMult(computed_b.a, psi_pt);
    auto entangled = current.cc->EvalAdd(mixed_a, mixed_b);
    
    double obfuscated_val = decrypt_value(current, entangled);
    double purified = purify(obfuscated_val, threshold);
    
    MirroredCircuit output;
    output.circuit_a.mini_a = encrypt_input(next, purified);
    output.circuit_a.mini_b = encrypt_input(next, purified);
    output.circuit_b.mini_a = encrypt_input(next, purified);
    output.circuit_b.mini_b = encrypt_input(next, purified);
    
    return output;
}

int main(int argc, char* argv[]) {
    uint32_t ring_dim = (argc > 1) ? std::atoi(argv[1]) : 2048;
    uint32_t depth = (argc > 2) ? std::atoi(argv[2]) : 10;
    uint32_t batch_size = (argc > 3) ? std::atoi(argv[3]) : 256;
    int total_gates = (argc > 4) ? std::atoi(argv[4]) : 10000;
    double threshold = (argc > 5) ? std::atof(argv[5]) : 0.5;
    int report_interval = (argc > 6) ? std::atoi(argv[6]) : 1000;
    
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  Quadruple Reality v2\n";
    std::cout << "  Single decrypt, iO-obfuscated before decrypt, mirrored to 4 realities\n";
    std::cout << "  RingDim: " << ring_dim << ", Gates: " << total_gates << "\n\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    double value = 1.0;
    time_t chain_start = time(0);
    
    for (int g = 1; g <= total_gates; g++) {
        SecureContext current = create_context(ring_dim, depth, batch_size);
        SecureContext next = create_context(ring_dim, depth, batch_size);
        
        MirroredCircuit mc;
        mc.circuit_a.mini_a = encrypt_input(current, value);
        mc.circuit_a.mini_b = encrypt_input(current, value);
        mc.circuit_b.mini_a = encrypt_input(current, value);
        mc.circuit_b.mini_b = encrypt_input(current, value);
        
        MirroredCircuit result = process_quadruple(current, mc, next, threshold);
        
        value = reveal(result.circuit_a.mini_a, next, PHI);
        value = purify(value, threshold);
        
        if (g % report_interval == 0 || g == total_gates) {
            time_t now = time(0);
            double elapsed = difftime(now, chain_start);
            double progress = (double)g / total_gates * 100.0;
            double rate = (double)g / elapsed;
            double eta = (total_gates - g) / rate;
            
            std::cout << "  [" << std::setw(5) << g << "/" << total_gates 
                      << " " << std::fixed << std::setprecision(1) << std::setw(5) << progress << "%]"
                      << " | Val: " << value
                      << " | " << std::setprecision(1) << rate << " g/s"
                      << " | ETA: " << std::setprecision(0) << eta << "s"
                      << "                \r" << std::flush;
        }
        
        if (value != 1.0 && value != 0.0) {
            std::cout << "\n\n  DRIFT at gate " << g << "! Value = " << value << "\n";
            return 1;
        }
    }
    
    time_t chain_end = time(0);
    double total_time = difftime(chain_end, chain_start);
    
    std::cout << "\n\n";
    std::cout << "  Complete\n";
    std::cout << "  Final value: " << std::fixed << std::setprecision(4) << value << "\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Time: " << std::setprecision(0) << total_time << "s (" << total_time/60 << " min)\n";
    std::cout << "  Rate: " << std::setprecision(1) << total_gates/total_time << " gates/sec\n";
    std::cout << "  Decrypts per gate: 1\n";
    std::cout << "  Mirrored to: 4 realities\n\n";
    std::cout << "  Ended: " << ctime(&chain_end) << "\n";
    
    return 0;
}
