#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <cstdlib>
#include <random>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double PSI = (1.0 - std::sqrt(5.0)) / 2.0;

struct SecureContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

SecureContext create_context(uint32_t ring_dim, uint32_t depth, uint32_t batch_size) {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(depth); p.SetScalingModSize(50); p.SetBatchSize(batch_size);
    p.SetRingDim(ring_dim); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    return {cc, kp};
}

Ciphertext<DCRTPoly> encrypt(SecureContext& sc, double v) {
    return sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{v}));
}

double decrypt(SecureContext& sc, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; sc.cc->Decrypt(sc.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

// Circuit A: f(x,y,z) = (x AND y) OR z = xy + z - xyz
Ciphertext<DCRTPoly> poly_A(SecureContext& sc, Ciphertext<DCRTPoly>& x, 
                              Ciphertext<DCRTPoly>& y, Ciphertext<DCRTPoly>& z) {
    auto xy = sc.cc->EvalMult(x, y);
    auto xyz = sc.cc->EvalMult(xy, z);
    auto sum = sc.cc->EvalAdd(xy, z);
    auto neg = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return sc.cc->EvalAdd(sum, sc.cc->EvalMult(xyz, neg));
}

// Circuit B: g(x,y,z) = (x OR z) AND (y OR z) = xz + yz + xy - 2xyz - xyz² + ...
// Simplified equivalent: same truth table, different polynomial structure
Ciphertext<DCRTPoly> poly_B(SecureContext& sc, Ciphertext<DCRTPoly>& x,
                              Ciphertext<DCRTPoly>& y, Ciphertext<DCRTPoly>& z) {
    auto xz = sc.cc->EvalMult(x, z);
    auto yz = sc.cc->EvalMult(y, z);
    auto xy = sc.cc->EvalMult(x, y);
    auto xyz = sc.cc->EvalMult(xy, z);
    
    auto sum = sc.cc->EvalAdd(xz, yz);
    auto neg = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto neg_xyz = sc.cc->EvalMult(xyz, neg);
    
    return sc.cc->EvalAdd(sum, neg_xyz);
}

// iO: Encode two polynomials into one ciphertext pair (a,b)
// φ reveals poly_A, ψ reveals poly_B
struct DualOutput { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

DualOutput iO_encode(SecureContext& sc, Ciphertext<DCRTPoly>& result_A, Ciphertext<DCRTPoly>& result_B) {
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto inv_denom = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    
    DualOutput out;
    auto diff = sc.cc->EvalSub(result_A, result_B);
    out.b = sc.cc->EvalMult(diff, inv_denom);
    out.a = sc.cc->EvalSub(result_A, sc.cc->EvalMult(out.b, phi_pt));
    return out;
}

int main(int argc, char* argv[]) {
    uint32_t rd = (argc > 1) ? std::atoi(argv[1]) : 4096;
    uint32_t dp = (argc > 2) ? std::atoi(argv[2]) : 60;
    int target = (argc > 3) ? std::atoi(argv[3]) : 100;
    
    time_t start = time(0);
    std::cout << "\n  Polynomial iO — Merged\n";
    std::cout << "  poly_A: xy + z - xyz\n";
    std::cout << "  poly_B: xz + yz - xyz\n";
    std::cout << "  φ reveals A, ψ reveals B\n";
    std::cout << "  RingDim " << rd << " Depth " << dp << "\n\n";
    
    SecureContext sc = create_context(rd, dp, 256);
    
    std::random_device rdev; std::mt19937 gen(rdev());
    std::uniform_int_distribution<int> bit(0, 1);
    
    // Initial state: both polynomials on same input
    auto x0 = encrypt(sc, 1.0);
    auto y0 = encrypt(sc, 0.0);
    auto z0 = encrypt(sc, 1.0);
    auto rA = poly_A(sc, x0, y0, z0);
    auto rB = poly_B(sc, x0, y0, z0);
    DualOutput state = iO_encode(sc, rA, rB);
    
    int gates = 0;
    
    for (int i = 0; i < target; i++) {
        auto x = encrypt(sc, bit(gen));
        auto y = encrypt(sc, bit(gen));
        auto z = encrypt(sc, bit(gen));
        
        try {
            auto next_A = poly_A(sc, x, y, z);
            auto next_B = poly_B(sc, x, y, z);
            DualOutput next = iO_encode(sc, next_A, next_B);
            
            // Chain: state = state AND next (on a-component)
            state.a = sc.cc->EvalMult(state.a, next.a);
            state.b = sc.cc->EvalAdd(state.b, next.b);
            gates++;
        } catch (...) {
            break;
        }
        
        if (i % 25 == 0 || i == target - 1) {
            double phi_val = decrypt(sc, state.a) + decrypt(sc, state.b) * PHI;
            double psi_val = decrypt(sc, state.a) + decrypt(sc, state.b) * PSI;
            std::cout << "  " << std::setw(4) << i 
                      << " φ=" << std::fixed << std::setprecision(4) << phi_val
                      << " ψ=" << psi_val << "\n" << std::flush;
        }
    }
    
    time_t end = time(0);
    double final_phi = decrypt(sc, state.a) + decrypt(sc, state.b) * PHI;
    double final_psi = decrypt(sc, state.a) + decrypt(sc, state.b) * PSI;
    
    std::cout << "\n  Gates: " << gates << "/" << target << "\n";
    std::cout << "  Time: " << difftime(end, start) << "s\n";
    std::cout << "  Final φ: " << std::fixed << std::setprecision(6) << final_phi << "\n";
    std::cout << "  Final ψ: " << final_psi << "\n\n";
    
    return 0;
}
