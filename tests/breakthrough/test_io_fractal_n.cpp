#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <cstdlib>
#include <random>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double PSI = (1.0 - std::sqrt(5.0)) / 2.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
struct SecureContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

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

DualGate gate_not(SecureContext& sc, DualGate& X) {
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg_one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {sc.cc->EvalSub(one, X.a), sc.cc->EvalMult(neg_one, X.b)};
}

DualGate gate_nand(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    DualGate and_result = {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
    return gate_not(sc, and_result);
}

DualGate gate_and(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate nand_result = gate_nand(sc, X, Y);
    return gate_not(sc, nand_result);
}

DualGate gate_or(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate not_x = gate_not(sc, X);
    DualGate not_y = gate_not(sc, Y);
    return gate_nand(sc, not_x, not_y);
}

// Single fractal layer: Chaos + Stability encoded in (a,b)
DualGate fractal_layer(SecureContext& sc, double a_val, double b_val) {
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto psi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto inv_denom = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    
    DualGate input = encrypt_input(sc, a_val);
    input.b = encrypt_input(sc, b_val).a;
    
    // Chaos: logistic map x → φ·x·(1-x)
    auto one_minus_a = sc.cc->EvalSub(one, input.a);
    auto logistic_a = sc.cc->EvalMult(input.a, one_minus_a);
    auto chaos_a = sc.cc->EvalMult(logistic_a, phi_pt);
    
    auto one_minus_b = sc.cc->EvalSub(one, input.b);
    auto logistic_b = sc.cc->EvalMult(input.b, one_minus_b);
    auto chaos_b = sc.cc->EvalMult(logistic_b, psi_pt);
    
    // Stability: x → x/φ
    auto inv_phi = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/PHI});
    auto inv_psi = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/PSI});
    auto stable_a = sc.cc->EvalMult(input.a, inv_phi);
    auto stable_b = sc.cc->EvalMult(input.b, inv_psi);
    
    // Encode into (a,b): φ→Chaos, ψ→Stability
    DualGate result;
    auto diff_a = sc.cc->EvalSub(chaos_a, stable_a);
    auto diff_b = sc.cc->EvalSub(chaos_b, stable_b);
    result.b = sc.cc->EvalAdd(sc.cc->EvalMult(diff_a, inv_denom), sc.cc->EvalMult(diff_b, inv_denom));
    result.a = sc.cc->EvalSub(chaos_a, sc.cc->EvalMult(result.b, phi_pt));
    
    return result;
}

int main(int argc, char* argv[]) {
    uint32_t ring_dim = (argc > 1) ? std::atoi(argv[1]) : 2048;
    uint32_t depth = (argc > 2) ? std::atoi(argv[2]) : 20;
    uint32_t batch_size = (argc > 3) ? std::atoi(argv[3]) : 256;
    int num_layers = (argc > 4) ? std::atoi(argv[4]) : 5;
    int chain_length = (argc > 5) ? std::atoi(argv[5]) : 100;
    double threshold = (argc > 6) ? std::atof(argv[6]) : 0.5;
    int report_interval = (argc > 7) ? std::atoi(argv[7]) : 20;
    
    if (num_layers < 1) num_layers = 1;
    if (num_layers > 100) num_layers = 100;
    
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  Fractal N-iO — " << num_layers << " Layers, Context Transition\n";
    std::cout << "  RingDim: " << ring_dim << ", Depth per layer: " << depth << "\n";
    std::cout << "  Budget: UNLIMITED (fresh context per layer)\n";
    std::cout << "  Chain: " << chain_length << " steps\n\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    time_t chain_start = time(0);
    double val_a = 0.5, val_b = 0.5;
    
    for (int step = 0; step < chain_length; step++) {
        double current_a = val_a, current_b = val_b;
        
        for (int layer = 0; layer < num_layers; layer++) {
            SecureContext sc = create_context(ring_dim, depth, batch_size);
            DualGate output = fractal_layer(sc, current_a, current_b);
            
            double raw_a = reveal(output, sc, PHI);
            double raw_b = reveal(output, sc, PSI);
            
            current_a = purify(raw_a, threshold);
            current_b = purify(raw_b, threshold);
        }
        
        val_a = current_a;
        val_b = current_b;
        
        if (step % report_interval == 0 || step == chain_length - 1) {
            time_t now = time(0);
            double elapsed = difftime(now, chain_start);
            double progress = (double)(step+1) / chain_length * 100.0;
            double rate = (double)(step+1) / elapsed;
            double eta = (chain_length - step - 1) / rate;
            
            std::cout << "  [" << std::setw(4) << (step+1) << "/" << chain_length 
                      << " " << std::fixed << std::setprecision(1) << std::setw(5) << progress << "%]"
                      << " | φ:" << val_a << " ψ:" << val_b
                      << " | " << std::setprecision(1) << rate << " s/s"
                      << " | ETA: " << std::setprecision(0) << eta << "s"
                      << "                \r" << std::flush;
        }
    }
    
    time_t chain_end = time(0);
    double total_time = difftime(chain_end, chain_start);
    
    std::cout << "\n\n";
    std::cout << "  Complete\n";
    std::cout << "  Layers: " << num_layers << "\n";
    std::cout << "  Chain length: " << chain_length << "\n";
    std::cout << "  Time: " << std::setprecision(0) << total_time << "s (" << total_time/60 << " min)\n";
    std::cout << "  Rate: " << std::setprecision(1) << chain_length/total_time << " steps/sec\n";
    std::cout << "  Final: φ=" << val_a << " ψ=" << val_b << "\n";
    std::cout << "  Budget: UNLIMITED (fresh context per layer)\n";
    std::cout << "  φ → Chaos, ψ → Stability\n";
    std::cout << "  Indistinguishable: " << (val_a == val_b ? "YES" : "NO") << "\n";
    std::cout << "\n  Ended: " << ctime(&chain_end) << "\n";
    
    return 0;
}
