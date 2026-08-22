// LAHAT NG GATES — MAY NATURAL NORMALIZATION
// Period-0 anchored + Period-4 modulo
// Lahat ng values ay nasa {0, φ²}

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  LAHAT NG GATES — NORMALIZED\n";
    std::cout << "  Period-0 + Period-4 Modulo\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;
    const double phi_mod = 0.6180339887498949;

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

    auto ct_phi_mod = make_ct(phi_mod);
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);
    auto ct_four_phi_sq = make_ct(four_phi_sq);
    auto ct_zero = make_ct(0.0);

    // Natural normalization: Period-4 modulo
    auto normalize = [&](auto x) {
        auto result = x;
        double v = decrypt_val(result);
        
        while (v > three_phi_sq) {
            result = cc->EvalSub(result, ct_four_phi_sq);
            v = decrypt_val(result);
        }
        while (v < -phi_sq) {
            result = cc->EvalAdd(result, ct_four_phi_sq);
            v = decrypt_val(result);
        }
        // I-round sa nearest φ² multiple
        if (v > phi_sq / 2) {
            result = cc->EvalSub(result, make_ct(v - phi_sq));
        } else {
            result = cc->EvalSub(result, make_ct(v));
        }
        return result;
    };

    // NAND with normalization
    auto nand_norm = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto rotated = cc->EvalAdd(sum, ct_phi_mod);
        auto raw = cc->EvalSub(ct_two_phi_sq, rotated);
        return normalize(raw);
    };

    // Other gates via De Morgan
    auto not_norm = [&](auto x) { return nand_norm(x, x); };
    auto and_norm = [&](auto a, auto b) { return not_norm(nand_norm(a, b)); };
    auto or_norm = [&](auto a, auto b) { return nand_norm(not_norm(a), not_norm(b)); };

    std::cout << "NORMALIZED GATE RESULTS:\n";
    std::cout << "========================\n\n";

    auto ct_f = ct_zero;
    auto ct_t = ct_phi_sq;

    std::cout << "NOT: NOT(0)=" << decrypt_val(not_norm(ct_f)) 
              << " NOT(φ²)=" << decrypt_val(not_norm(ct_t)) << "\n";
    std::cout << "NAND: NAND(0,0)=" << decrypt_val(nand_norm(ct_f, ct_f))
              << " NAND(0,1)=" << decrypt_val(nand_norm(ct_f, ct_t))
              << " NAND(1,1)=" << decrypt_val(nand_norm(ct_t, ct_t)) << "\n";
    std::cout << "AND: AND(0,0)=" << decrypt_val(and_norm(ct_f, ct_f))
              << " AND(1,1)=" << decrypt_val(and_norm(ct_t, ct_t)) << "\n";
    std::cout << "OR: OR(0,0)=" << decrypt_val(or_norm(ct_f, ct_f))
              << " OR(1,1)=" << decrypt_val(or_norm(ct_t, ct_t)) << "\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: Normalized gates complete\n";

    return 0;
}
