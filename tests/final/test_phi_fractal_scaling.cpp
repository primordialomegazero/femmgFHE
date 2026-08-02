// FRACTAL SCALING: More levels via smaller scaling mod
// Uses FIXEDMANUAL scaling to avoid ratio issues

#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}

PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                   const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double get_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = decrypt_val(cc, kp, s.a);
    double b = decrypt_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}

int main() {
    std::cout << "\n  FRACTAL SCALING: Precision vs Depth (FIXEDMANUAL)\n";
    std::cout << "  RingDim=16384\n\n";

    // Test configs: (scaling_bits, depth, first_mod_size)
    // Keep first_mod large enough for all levels
    struct Config {
        int scaling_bits;
        int depth;
        int first_mod_bits;
    };
    
    Config configs[] = {
        {50, 30, 60},
        {40, 50, 55},
        {30, 80, 50},
    };
    
    for (auto cfg : configs) {
        std::cout << "  Scaling=" << cfg.scaling_bits 
                  << " bits, Depth=" << cfg.depth 
                  << ": ";
        std::cout.flush();
        
        try {
            CCParams<CryptoContextCKKSRNS> p;
            p.SetMultiplicativeDepth(cfg.depth);
            p.SetScalingModSize(cfg.scaling_bits);
            p.SetFirstModSize(cfg.first_mod_bits);
            p.SetScalingTechnique(FIXEDMANUAL);
            p.SetBatchSize(2048);
            p.SetRingDim(16384);
            p.SetSecretKeyDist(UNIFORM_TERNARY);
            p.SetSecurityLevel(HEStd_NotSet);
            
            auto cc = GenCryptoContext(p);
            cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
            auto kp = cc->KeyGen();
            cc->EvalMultKeyGen(kp.secretKey);

            auto enc_one = cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
            auto enc_phi = cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{1.618}));

            PE state = {enc_phi, enc_one};
            int steps = 0;
            
            for (int i = 0; i < cfg.depth; i++) {
                auto sq_a = cc->EvalMult(state.a, state.a);
                auto sq_b = cc->EvalMult(state.b, state.b);
                state = {sq_a, sq_b};
                state = mulY(cc, state);
                
                // Verify recovery
                PE check = state;
                for (int j = 0; j <= i; j++) {
                    check = mulY_inv(cc, check);
                }
                double val = get_ratio(cc, kp, check);
                
                if (std::abs(val - 1.618) < 0.05) {
                    steps = i + 1;
                } else {
                    break;
                }
            }
            std::cout << steps << " steps OK\n";
            
        } catch (const std::exception& e) {
            std::cout << "FAILED: " << e.what() << "\n";
        }
    }
    
    std::cout << "\n  Done.\n\n";
    return 0;
}
