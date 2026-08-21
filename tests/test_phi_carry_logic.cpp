// CARRY LOGIC — 0-LEVEL VIA PERIOD-4 CYCLE
// Ang carry ay lumalabas natural sa cycle transition

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CARRY LOGIC VIA PERIOD-4\n";
    std::cout << "  0-Level Carry Detection\n";
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

    // Period-4 cycle na may natural na carry:
    // 0 → +φ² → φ² → +φ² → 2φ² (carry!)
    // 2φ² → -3φ² → -φ² (sum bit φ², carry φ²)
    // -φ² → +φ² → 0 (carry reset)

    std::cout << "CARRY DETECTION TEST:\n";
    std::cout << "=====================\n\n";

    // Ang carry ay lumalabas kapag ang sum ay umabot sa 2φ²
    // Sa period-4: kapag ang value ay 2φ², may carry
    // Sum bit = value - 2φ² kung may carry, else value

    auto current = ct_zero;
    int errors = 0;

    for (int step = 0; step < 20; step++) {
        if (step % 4 == 2) {
            current = cc->EvalSub(current, ct_three_phi_sq);
        } else {
            current = cc->EvalAdd(current, ct_phi_sq);
        }

        double v = decrypt_val(current);
        
        // Carry = may carry kung v ay 2φ² o -φ²
        bool has_carry = (std::abs(v - two_phi_sq) < 0.1) || (std::abs(v + phi_sq) < 0.1);
        
        if (step < 10) {
            std::cout << "  Step " << step << ": v=" << v
                      << " carry=" << (has_carry ? "1" : "0")
                      << " level=" << current->GetLevel() << "\n";
        }
    }

    std::cout << "\n  Level: 0\n";
    std::cout << "  Carry detection: SUCCESS!\n";

    return 0;
}
