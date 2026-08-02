// TEST: EvalSub with plaintext - does it preserve level?
#include <iostream>
#include "openfhe.h"
using namespace lbcrypto;

int main() {
    std::cout << "\n  PLAINTEXT SUBTRACTION LEVEL TEST\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(10);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    
    auto pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.5});
    auto ct = cc->Encrypt(kp.publicKey, pt);
    
    std::cout << "  Initial level: " << ct->GetLevel() << "\n\n";
    
    // Test 1: EvalSub(ciphertext, ciphertext)
    auto ct_sub_ct = cc->EvalSub(ct, ct);
    std::cout << "  EvalSub(CT, CT): level=" << ct_sub_ct->GetLevel();
    std::cout << (ct_sub_ct->GetLevel() == ct->GetLevel() ? " (preserved)" : " (changed)") << "\n";
    
    // Test 2: EvalSub(plaintext, ciphertext) via negation
    // CKKS doesn't have direct EvalSub(pt, ct)
    // But we can do: -(ct) + pt
    auto neg_ct = cc->EvalNegate(ct);
    std::cout << "  EvalNegate(CT):  level=" << neg_ct->GetLevel();
    std::cout << (neg_ct->GetLevel() == ct->GetLevel() ? " (preserved)" : " (changed)") << "\n";
    
    // Then add plaintext
    auto pt_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto result = cc->EvalAdd(neg_ct, pt_one);
    std::cout << "  EvalAdd(negCT, PT): level=" << result->GetLevel();
    std::cout << (result->GetLevel() == ct->GetLevel() ? " (preserved)" : " (changed)") << "\n";
    
    // Test 3: Direct EvalSub with plaintext
    // result = 1 - ct = EvalSub(pt_one, ct) — does this work?
    try {
        auto direct = cc->EvalSub(pt_one, ct);
        std::cout << "  EvalSub(PT, CT):   level=" << direct->GetLevel();
        std::cout << (direct->GetLevel() == ct->GetLevel() ? " (preserved)" : " (changed)") << "\n";
        std::cout << "  Direct EvalSub(PT, CT) works!\n";
    } catch (...) {
        std::cout << "  EvalSub(PT, CT):   NOT SUPPORTED\n";
        std::cout << "  But EvalNegate + EvalAdd(PT) achieves same: level preserved!\n";
    }
    
    // Verify value
    Plaintext result_pt;
    cc->Decrypt(kp.secretKey, result, &result_pt);
    double val = result_pt->GetCKKSPackedValue()[0].real();
    std::cout << "\n  Computed 1 - 0.5 = " << val << " (expected 0.5)\n\n";
    
    return 0;
}
