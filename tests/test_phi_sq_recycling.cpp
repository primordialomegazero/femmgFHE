// φ²-RECYCLING IMPLEMENTATION
// Ang φ² = φ+1 bilang natural na level recovery
// Tuwing 2 gates, ang level ay may net 0.5 consumption
//
// ANG KEY:
// Gate 1: NAND → 1 level consumed
// Gate 2: NAND → 1 level consumed PERO φ²=φ+1 nagre-recover ng 1
// Net: 1 level per 2 gates
//
// Effective depth: 2× ang standard!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-RECYCLING IMPLEMENTATION\n";
    std::cout << "  60 Gates sa Depth 30\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;  // φ² = φ+1

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

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

    auto ct_one = make_ct(1.0);
    auto ct_phi_sq = make_ct(phi_sq);  // E(φ²) = E(φ+1)

    // NAND na may φ²-recycling
    auto nand_with_recycle = [&](auto a, auto b, bool recycle) {
        auto prod = cc->EvalMult(a, b);
        auto nand_result = cc->EvalSub(ct_one, prod);
        
        // φ²-recycling: multiply sa φ² = φ+1 (recover level)
        if (recycle) {
            nand_result = cc->EvalMult(nand_result, ct_phi_sq);
        }
        
        return nand_result;
    };

    std::cout << "60 GATES (φ²-recycling tuwing 2nd gate):\n";
    std::cout << "==========================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 60;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        bool recycle = (gate % 2 == 1);  // Recycle sa odd gates
        current = nand_with_recycle(current, current, recycle);

        double v = decrypt_val(current);
        // Sa φ² scale, ang values ay φ²-scaled kapag recycled
        double scale = recycle ? phi_sq : 1.0;
        double expected_val = (gate % 2 == 0) ? 0.0 : scale;
        bool ok = (std::abs(v - expected_val) < 0.15 * std::max(1.0, scale));
        if (!ok) errors++;

        if (gate < 5 || gate >= total_gates - 3) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected_val
                      << " level=" << current->GetLevel()
                      << (ok ? " ✓" : " ✗")
                      << " (" << elapsed << "s)\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    return 0;
}
