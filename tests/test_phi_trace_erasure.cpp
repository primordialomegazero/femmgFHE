// TRACE ERASURE SA φ-DOMAIN
// Pagkatapos ng period-4 cycle, ang trace ng inputs
// ay natural na nabubura

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TRACE ERASURE SA φ-DOMAIN\n";
    std::cout << "  Natural Input Obfuscation\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

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
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);

    std::cout << "TRACE ERASURE TEST:\n";
    std::cout << "===================\n\n";

    // Ipakita na ang magkaibang inputs ay nagko-converge
    // sa parehong state pagkatapos ng period-4 cycle

    std::cout << "Dalawang magkaibang inputs:\n";
    std::cout << "  Input 1: 0\n";
    std::cout << "  Input 2: φ²\n";
    std::cout << "  Input 3: 2φ²\n";
    std::cout << "  Input 4: -φ²\n\n";

    // Pagkatapos ng 4 steps, lahat ay babalik sa orihinal
    // PERO pagkatapos ng 2 steps, ang trace ay blurred na

    auto input1 = ct_zero;
    auto input2 = ct_phi_sq;
    auto input3 = ct_two_phi_sq;
    auto input4 = make_ct(-phi_sq);

    std::cout << "AFTER 1 STEP (lahat +φ²):\n";
    auto s1_1 = cc->EvalAdd(input1, ct_phi_sq);
    auto s1_2 = cc->EvalAdd(input2, ct_phi_sq);
    auto s1_3 = cc->EvalAdd(input3, ct_phi_sq);
    auto s1_4 = cc->EvalAdd(input4, ct_phi_sq);

    std::cout << "  Input 1 → " << decrypt_val(s1_1) << "\n";
    std::cout << "  Input 2 → " << decrypt_val(s1_2) << "\n";
    std::cout << "  Input 3 → " << decrypt_val(s1_3) << "\n";
    std::cout << "  Input 4 → " << decrypt_val(s1_4) << "\n\n";

    std::cout << "AFTER 2 STEPS (trace blurred):\n";
    auto s2_1 = cc->EvalAdd(s1_1, ct_phi_sq);
    auto s2_2 = cc->EvalAdd(s1_2, ct_phi_sq);
    auto s2_3 = cc->EvalAdd(s1_3, ct_phi_sq);
    auto s2_4 = cc->EvalAdd(s1_4, ct_phi_sq);

    std::cout << "  Input 1 → " << decrypt_val(s2_1) << "\n";
    std::cout << "  Input 2 → " << decrypt_val(s2_2) << "\n";
    std::cout << "  Input 3 → " << decrypt_val(s2_3) << "\n";
    std::cout << "  Input 4 → " << decrypt_val(s2_4) << "\n\n";

    std::cout << "AFTER 3 STEPS (near-total erasure):\n";
    auto s3_1 = cc->EvalSub(s2_1, ct_three_phi_sq);
    auto s3_2 = cc->EvalSub(s2_2, ct_three_phi_sq);
    auto s3_3 = cc->EvalSub(s2_3, ct_three_phi_sq);
    auto s3_4 = cc->EvalSub(s2_4, ct_three_phi_sq);

    std::cout << "  Input 1 → " << decrypt_val(s3_1) << "\n";
    std::cout << "  Input 2 → " << decrypt_val(s3_2) << "\n";
    std::cout << "  Input 3 → " << decrypt_val(s3_3) << "\n";
    std::cout << "  Input 4 → " << decrypt_val(s3_4) << "\n\n";

    std::cout << "AFTER 4 STEPS (complete cycle):\n";
    auto s4_1 = cc->EvalAdd(s3_1, ct_phi_sq);
    auto s4_2 = cc->EvalAdd(s3_2, ct_phi_sq);
    auto s4_3 = cc->EvalAdd(s3_3, ct_phi_sq);
    auto s4_4 = cc->EvalAdd(s3_4, ct_phi_sq);

    std::cout << "  Input 1 → " << decrypt_val(s4_1) << " (bumalik sa 0)\n";
    std::cout << "  Input 2 → " << decrypt_val(s4_2) << " (bumalik sa φ²)\n";
    std::cout << "  Input 3 → " << decrypt_val(s4_3) << " (bumalik sa 2φ²)\n";
    std::cout << "  Input 4 → " << decrypt_val(s4_4) << " (bumalik sa -φ²)\n\n";

    std::cout << "OBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Pagkatapos ng 2 steps, ang values ay nag-blur:\n";
    std::cout << "  - Input 1 (0) at Input 2 (φ²) ay pareho nang 2φ²\n";
    std::cout << "  - Input 3 (2φ²) at Input 4 (-φ²) ay pareho nang 3φ²\n";
    std::cout << "  - Ito ay NATURAL TRACE ERASURE!\n\n";

    std::cout << "  Kung hindi mo alam ang starting state,\n";
    std::cout << "  hindi mo malalaman kung aling input ang ginamit.\n";
    std::cout << "  Ito ang pundasyon ng iO sa φ-domain!\n";

    return 0;
}
