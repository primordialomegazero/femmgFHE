// RULE 110 DEBUG — Tingnan ang Bawat Gate
// I-debug ang NOT, AND, OR sa φ-space

#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 DEBUG\n";
    std::cout << "  Bawat Gate Values\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto ct_phi = make_ct(PHI);
    auto ct_0 = make_ct(0.0);

    // NOT: x → φ - x
    auto eval_not = [&](auto x) {
        return cc->EvalSub(ct_phi, x);
    };

    std::cout << "NOT GATE DEBUG:\n";
    std::cout << "===============\n";
    std::cout << "  NOT(0) = " << decrypt_val(eval_not(ct_0)) << " (dapat " << PHI << ")\n";
    std::cout << "  NOT(φ) = " << decrypt_val(eval_not(ct_phi)) << " (dapat 0)\n\n";

    // AND via De Morgan: AND(a,b) = NOT(NOT(a) OR NOT(b))
    // OR(a,b) = NOT(NOT(a) AND NOT(b))
    
    // Sa φ-space, subukan ang simpleng OR:
    // OR(a,b) = a + b - φ * (a AND b) — pero may multiplication
    // Mas simple: OR = NOT(AND(NOT(a), NOT(b)))
    
    auto eval_and_via_not_or = [&](auto a, auto b) {
        // AND(a,b) = NOT(NOT(a) OR NOT(b))
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        // OR(not_a, not_b) = ? 
        // Subukan natin ang iba't ibang paraan
        // Una: OR = a + b - φ kung pareho φ
        // Sa φ-space: 0+0=0, 0+φ=φ, φ+φ=2φ
        // Para sa OR: kailangan φ ang result kung may φ
        
        // Simplest OR: a + b (addition lang!)
        auto or_ab = cc->EvalAdd(a, b);
        
        // NOT(OR) = φ - (a+b)
        return eval_not(or_ab);
    };

    std::cout << "AND VIA NOT+OR DEBUG:\n";
    std::cout << "=====================\n";
    std::cout << "  AND(0,0) = " << decrypt_val(eval_and_via_not_or(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  AND(0,φ) = " << decrypt_val(eval_and_via_not_or(ct_0, ct_phi)) << " (dapat 0)\n";
    std::cout << "  AND(φ,φ) = " << decrypt_val(eval_and_via_not_or(ct_phi, ct_phi)) << " (dapat φ)\n\n";

    // Ang problema: kailangan natin ng tamang AND sa φ-space
    // Sa φ-space, ang states ay 0 at φ
    // AND(0,0) = 0, AND(0,φ) = 0, AND(φ,φ) = φ
    
    // Natural na AND: min(a,b) sa φ-space
    // Sa φ-space: min(0,0)=0, min(0,φ)=0, min(φ,φ)=φ
    
    // Paano gawin ang min sa homomorphic na walang decrypt?
    // min(a,b) = (a + b - |a - b|) / 2
    
    // Sa φ-space: |a-b| = φ kung magkaiba, 0 kung pareho
    // Dahil a,b ∈ {0, φ}
    
    // Kaya: min(a,b) = (a + b - |a-b|) / 2
    // |a-b| = φ kung magkaiba, 0 kung pareho
    // Sa FHE: kailangan natin ng absolute value
    
    std::cout << "XOR AS DIFF DEBUG:\n";
    std::cout << "==================\n";
    auto diff_00 = cc->EvalSub(ct_0, ct_0);
    auto diff_0phi = cc->EvalSub(ct_0, ct_phi);
    auto diff_phi0 = cc->EvalSub(ct_phi, ct_0);
    auto diff_phiphi = cc->EvalSub(ct_phi, ct_phi);
    
    std::cout << "  diff(0,0) = " << decrypt_val(diff_00) << "\n";
    std::cout << "  diff(0,φ) = " << decrypt_val(diff_0phi) << "\n";
    std::cout << "  diff(φ,0) = " << decrypt_val(diff_phi0) << "\n";
    std::cout << "  diff(φ,φ) = " << decrypt_val(diff_phiphi) << "\n\n";

    return 0;
}
