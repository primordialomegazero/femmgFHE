// φ²-DOMAIN 0-LEVEL UNIVERSAL
// I-combine ang NOT chain sa ibang operations
// para makabuo ng universal gate na 0-level

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN 0-LEVEL UNIVERSAL\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);

    // 0-LEVEL NOT: alternating φ² ↔ 0
    auto not_0level = [&](auto a) {
        // If a=0: return φ²; if a=φ²: return 0
        // Sa φ²-domain: NOT(a) = φ² - a (subtraction only!)
        return cc->EvalSub(ct_phi_sq, a);
    };

    std::cout << "0-LEVEL NOT TEST:\n";
    std::cout << "=================\n";
    auto not_zero = not_0level(ct_zero);
    auto not_phi = not_0level(ct_phi_sq);
    std::cout << "  NOT(0) = " << decrypt_val(not_zero) << " (expected " << phi_sq << ")\n";
    std::cout << "  NOT(φ²) = " << decrypt_val(not_phi) << " (expected 0)\n\n";

    // 0-LEVEL NAND attempt: NAND(a,b) = φ² - (a AND b)
    // Kung AND ay 0-level din, NAND ay 0-level!
    // AND(a,b) = max(0, a+b-φ²) — pero ito ay comparison
    
    // Alternative: NAND via De Morgan: NAND(a,b) = NOT(a AND b) = NOT(a) OR NOT(b)
    // Sa φ²-domain, baka ang OR ay addition?
    // OR(a,b) = min(φ², a+b) — comparison ulit
    
    // PINAKA-SIMPLE: NAND(a,b) = φ² - a - b + (a AND b)
    // Kailangan pa rin ng AND (multiplication)

    std::cout << "OBSERVATION:\n";
    std::cout << "============\n";
    std::cout << "  NOT ay 0-level (subtraction only).\n";
    std::cout << "  NAND kailangan ng AND (multiplication).\n";
    std::cout << "  PERO: De Morgan's Law ay nagbibigay ng alternatibo.\n";
    std::cout << "  NAND = NOT(a) OR NOT(b)\n\n";

    // TEST: NAND via NOT + OR (kung OR ay addition)
    std::cout << "NAND VIA DE MORGAN:\n";
    std::cout << "===================\n";
    
    auto de_morgan_nand = [&](auto a, auto b) {
        // NOT(a) OR NOT(b)
        auto not_a = cc->EvalSub(ct_phi_sq, a);
        auto not_b = cc->EvalSub(ct_phi_sq, b);
        // OR via addition (0-level!)
        return cc->EvalAdd(not_a, not_b);
    };
    
    auto dm_00 = de_morgan_nand(ct_zero, ct_zero);
    auto dm_0phi = de_morgan_nand(ct_zero, ct_phi_sq);
    auto dm_phiphi = de_morgan_nand(ct_phi_sq, ct_phi_sq);
    
    std::cout << "  NAND(0,0) = " << decrypt_val(dm_00) << "\n";
    std::cout << "  NAND(0,φ²) = " << decrypt_val(dm_0phi) << "\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(dm_phiphi) << "\n";
    std::cout << "  (Need normalization to 0 or φ²)\n";

    return 0;
}
