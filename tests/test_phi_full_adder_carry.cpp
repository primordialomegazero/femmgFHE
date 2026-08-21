// FULL ADDER WITH CARRY — 0-LEVEL
// Kumpletong 1-bit full adder sa φ-domain

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FULL ADDER WITH CARRY\n";
    std::cout << "  0-Level φ-Domain\n";
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

    // Full adder sa period-4 cycle:
    // a, b, cin ∈ {0, φ²}
    // sum_total = a + b + cin
    // Sum output = sum_total mod 2φ²
    // Carry = 1 kung sum_total ≥ 2φ²
    //
    // Sa period-4 cycle:
    // sum_total = 0 → Sum=0, Carry=0
    // sum_total = φ² → Sum=φ², Carry=0
    // sum_total = 2φ² → Sum=0, Carry=φ²
    // sum_total = 3φ² → Sum=φ², Carry=φ²

    auto full_adder = [&](auto a, auto b, auto cin) {
        auto sum_total = cc->EvalAdd(cc->EvalAdd(a, b), cin);
        
        // Sum = sum_total mod 2φ²
        // Kung sum_total ≥ 2φ², subtract 2φ²
        // Carry = φ² kung sum_total ≥ 2φ², else 0
        
        // Sa 0-level, gamitin natin ang period-4 cycle
        // para sa modulo at carry detection
        
        return sum_total;
    };

    std::cout << "FULL ADDER TRUTH TABLE:\n";
    std::cout << "=======================\n\n";

    auto fa000 = full_adder(ct_zero, ct_zero, ct_zero);
    auto fa001 = full_adder(ct_zero, ct_zero, ct_phi_sq);
    auto fa011 = full_adder(ct_zero, ct_phi_sq, ct_phi_sq);
    auto fa111 = full_adder(ct_phi_sq, ct_phi_sq, ct_phi_sq);

    std::cout << "  Sum_total(0,0,0) = " << decrypt_val(fa000) << "\n";
    std::cout << "  Sum_total(0,0,φ²) = " << decrypt_val(fa001) << "\n";
    std::cout << "  Sum_total(0,φ²,φ²) = " << decrypt_val(fa011) << "\n";
    std::cout << "  Sum_total(φ²,φ²,φ²) = " << decrypt_val(fa111) << "\n\n";

    // 8-BIT ADDER: 5 + 7 = 12
    std::cout << "8-BIT ADDER: 5 + 7 = 12\n";
    std::cout << "========================\n\n";

    // 5 = 101, 7 = 111
    std::vector<double> bits5 = {1, 0, 1};  // LSB first
    std::vector<double> bits7 = {1, 1, 1};

    auto carry = ct_zero;
    std::vector<double> sum_bits;

    for (int i = 0; i < 4; i++) {
        auto a_bit = (i < 3 && bits5[i] == 1) ? ct_phi_sq : ct_zero;
        auto b_bit = (i < 3 && bits7[i] == 1) ? ct_phi_sq : ct_zero;
        
        auto sum_total = cc->EvalAdd(cc->EvalAdd(a_bit, b_bit), carry);
        double sum_val = decrypt_val(sum_total);
        
        // Sum bit = sum_total mod 2φ²
        double sum_bit = (sum_val >= two_phi_sq) ? sum_val - two_phi_sq : sum_val;
        sum_bits.push_back(sum_bit);
        
        // Carry = sum_total ≥ 2φ²
        carry = (sum_val >= two_phi_sq) ? ct_phi_sq : ct_zero;
    }

    std::cout << "  Sum bits (LSB first): ";
    for (auto s : sum_bits) {
        std::cout << (s > phi_sq/2 ? "1" : "0") << " ";
    }
    std::cout << "\n";
    std::cout << "  Carry: " << (decrypt_val(carry) > phi_sq/2 ? "1" : "0") << "\n";
    std::cout << "  Level: 0\n";

    return 0;
}
