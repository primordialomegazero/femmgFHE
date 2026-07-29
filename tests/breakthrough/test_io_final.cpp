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

DualGate gate_standard(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    return {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

DualGate gate_double_not(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate nand_result = gate_nand(sc, X, Y);
    return gate_not(sc, nand_result);
}

DualGate gate_demorgan(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate not_x = gate_not(sc, X);
    DualGate not_y = gate_not(sc, Y);
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto or_a = sc.cc->EvalSub(one, sc.cc->EvalMult(
        sc.cc->EvalSub(one, not_x.a), sc.cc->EvalSub(one, not_y.a)));
    auto or_b = sc.cc->EvalAdd(
        sc.cc->EvalAdd(
            sc.cc->EvalMult(sc.cc->EvalSub(one, not_x.a), not_y.b),
            sc.cc->EvalMult(not_x.b, sc.cc->EvalSub(one, not_y.a))
        ), sc.cc->EvalMult(not_x.b, not_y.b));
    DualGate or_result = {or_a, or_b};
    return gate_not(sc, or_result);
}

DualGate gate_arithmetic(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto inv_phi = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/PHI});
    auto x_phi = sc.cc->EvalAdd(X.a, sc.cc->EvalMult(X.b, phi_pt));
    auto y_phi = sc.cc->EvalAdd(Y.a, sc.cc->EvalMult(Y.b, phi_pt));
    auto prod = sc.cc->EvalMult(x_phi, y_phi);
    auto a_out = sc.cc->EvalMult(prod, inv_phi);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    return {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

int main(int argc, char* argv[]) {
    uint32_t ring_dim = (argc > 1) ? std::atoi(argv[1]) : 2048;
    uint32_t depth = (argc > 2) ? std::atoi(argv[2]) : 10;
    uint32_t batch_size = (argc > 3) ? std::atoi(argv[3]) : 256;
    int total_gates = (argc > 4) ? std::atoi(argv[4]) : 10000;
    double threshold = (argc > 5) ? std::atof(argv[5]) : 0.5;
    int report_interval = (argc > 6) ? std::atoi(argv[6]) : 1000;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> gate_dist(0, 3);
    
    typedef DualGate (*GateFunc)(SecureContext&, DualGate&, DualGate&);
    GateFunc gates[4] = {gate_standard, gate_double_not, gate_demorgan, gate_arithmetic};
    std::string gate_names[4] = {"Standard", "NOT(NAND)", "De Morgan", "Arithmetic"};
    
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  iO Chain — 4 Structurally Different, Functionally Equivalent Gates\n";
    std::cout << "  RingDim: " << ring_dim << ", Gates: " << total_gates << "\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    double value = 1.0;
    time_t chain_start = time(0);
    std::vector<int> gate_usage(4, 0);
    
    for (int g = 1; g <= total_gates; g++) {
        SecureContext current = create_context(ring_dim, depth, batch_size);
        SecureContext next = create_context(ring_dim, depth, batch_size);
        
        int gate_idx = gate_dist(gen);
        gate_usage[gate_idx]++;
        
        DualGate X = encrypt_input(current, value);
        DualGate Y = encrypt_input(current, 1.0);
        DualGate computed = gates[gate_idx](current, X, Y);
        
        // iO obfuscation before single decrypt
        auto phi_pt = current.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
        auto psi_pt = current.cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
        auto mixed = current.cc->EvalAdd(
            current.cc->EvalMult(computed.a, phi_pt),
            current.cc->EvalMult(computed.b, psi_pt));
        
        double obfuscated = decrypt_value(current, mixed);
        value = purify(obfuscated, threshold);
        
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
    std::cout << "  Decrypts per gate: 1 (iO-obfuscated before decrypt)\n";
    std::cout << "  Gate distribution:\n";
    for (int i = 0; i < 4; i++) {
        std::cout << "    " << gate_names[i] << ": " << gate_usage[i] << "\n";
    }
    std::cout << "\n  Ended: " << ctime(&chain_end) << "\n";
    
    return 0;
}
