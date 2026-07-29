// MIRROR DEBUG — Isolate the crash
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

DualGate observe_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    return {a_out, cc->EvalMult(cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}

DualGate make_input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

DualGate mirror_recycle(CryptoContext<DCRTPoly>& cc, const DualGate& gate) {
    std::cout << "    [DEBUG] mirror_recycle called\n" << std::flush;
    
    // Step 1: Decode φ-value
    std::cout << "    [DEBUG] Step 1: compute v0 = a + b*φ\n" << std::flush;
    auto phi_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto b_phi = cc->EvalMult(gate.b, phi_pt);
    auto v0 = cc->EvalAdd(gate.a, b_phi);
    std::cout << "    [DEBUG] v0 done\n" << std::flush;
    
    // Step 2: Decode ψ-value
    std::cout << "    [DEBUG] Step 2: compute v1 = a + b*ψ\n" << std::flush;
    auto psi_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    auto b_psi = cc->EvalMult(gate.b, psi_pt);
    auto v1 = cc->EvalAdd(gate.a, b_psi);
    std::cout << "    [DEBUG] v1 done\n" << std::flush;
    
    // Step 3: diff = v0 - v1
    std::cout << "    [DEBUG] Step 3: diff = v0 - v1\n" << std::flush;
    auto diff = cc->EvalSub(v0, v1);
    std::cout << "    [DEBUG] diff done\n" << std::flush;
    
    // Step 4: b_new = diff / (φ-ψ)
    std::cout << "    [DEBUG] Step 4: b_new = diff * inv_denom\n" << std::flush;
    double inv_denom = 1.0 / (PHI - PSI);
    std::cout << "    [DEBUG] inv_denom = " << inv_denom << "\n" << std::flush;
    auto inv_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{inv_denom});
    auto b_new = cc->EvalMult(diff, inv_pt);
    std::cout << "    [DEBUG] b_new done\n" << std::flush;
    
    // Step 5: a_new = v0 - b_new*φ
    std::cout << "    [DEBUG] Step 5: a_new = v0 - b_new*φ\n" << std::flush;
    auto b_new_phi = cc->EvalMult(b_new, phi_pt);
    auto a_new = cc->EvalSub(v0, b_new_phi);
    std::cout << "    [DEBUG] a_new done\n" << std::flush;
    
    std::cout << "    [DEBUG] mirror_recycle COMPLETE\n" << std::flush;
    return {a_new, b_new};
}

int main() {
    std::cout << "\n  MIRROR DEBUG — Step by step\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "  [DEBUG] Context created\n" << std::flush;
    
    // Simple AND gate
    std::cout << "  [DEBUG] Creating inputs...\n" << std::flush;
    DualGate x = make_input(cc, kp, 1.0);
    DualGate y = make_input(cc, kp, 1.0);
    std::cout << "  [DEBUG] Inputs created\n" << std::flush;
    
    std::cout << "  [DEBUG] Computing observe_and...\n" << std::flush;
    DualGate g = observe_and(cc, x, y);
    std::cout << "  [DEBUG] observe_and done\n" << std::flush;
    
    std::cout << "  [DEBUG] Starting mirror_recycle...\n" << std::flush;
    try {
        DualGate mirrored = mirror_recycle(cc, g);
        std::cout << "  [DEBUG] Mirror SUCCESS!\n" << std::flush;
        
        double before = decrypt_val(cc, kp, g.a);
        double after = decrypt_val(cc, kp, mirrored.a);
        std::cout << "  Before: " << before << ", After: " << after << "\n";
    } catch (const std::exception& e) {
        std::cout << "  [ERROR] " << e.what() << "\n";
    } catch (...) {
        std::cout << "  [ERROR] Unknown exception\n";
    }
    
    std::cout << "\n  DONE\n";
    return 0;
}
