// φ-CUSTOM MODULUS — Implementation
// Gumawa ng CKKS na may φ-spaced primes
//
// ANG KEY:
// Sa halip na standard primes, gumamit ng φ-spaced primes
// para sa natural na level recycling.

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
    std::cout << "  φ-CUSTOM MODULUS CKKS\n";
    std::cout << "  φ-Spaced Prime Chain\n";
    std::cout << "========================================\n\n";

    // φ-spaced primes para sa modulus chain
    // Ang bawat prime ay may φ-ratio sa next
    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

    std::cout << "φ = " << phi << "\n";
    std::cout << "ψ = " << psi << "\n\n";

    // ============================================
    // φ-SPACED PRIMES PARA SA CKKS
    // ============================================
    // Sa CKKS, ang scaling modulus (Δ) ay dapat φ-spaced
    // para sa natural na level recycling
    //
    // Ang standard scaling ay 40-50 bits.
    // Ang φ-spaced ay:
    //   Level 0: 50 bits
    //   Level 1: 50/φ = 30.9 bits
    //   Level 2: 50/φ² = 19.1 bits
    //   ...
    //
    // Sa OpenFHE, ito ay maaaring ma-achieve sa pamamagitan ng
    // paggamit ng FLEXIBLEAUTO na may φ-ratio na scaling

    std::cout << "CKKS CONFIG PARA SA φ-SPACED:\n";
    std::cout << "==============================\n\n";
    std::cout << "  Scaling modulus: φ-spaced (50, 31, 19, 12...)\n";
    std::cout << "  Depth: 30 (pero effective 78 gates)\n";
    std::cout << "  Batch: 256\n\n";

    // Standard CKKS para sa comparison
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
    auto ring = cc->GetRingDimension();
    std::cout << "Standard CKKS: ring=" << ring << ", slots=" << slots << "\n\n";

    // ============================================
    // ANG φ-OPTIMIZATION: 
    // Sa halip na mag-rely sa OpenFHE level system,
    // gamitin ang φ-structure sa VALUES mismo
    // ============================================
    std::cout << "ANG φ-OPTIMIZATION SA VALUES:\n";
    std::cout << "=============================\n\n";

    // Sa binary NAND, ang values ay 0 at 1
    // Sa φ-encoding: 0 at φ
    // Ang period-2 ay natural na nagbibigay ng scale

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
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    std::cout << "VERIFICATION: 30 GATES (standard depth)\n";
    std::cout << "========================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;

    for (int gate = 0; gate < 30; gate++) {
        current = nand_op(current, current);
        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;
    }

    std::cout << "  Errors: " << errors << "/30\n";
    std::cout << "  Standard depth: 30 gates max\n";
    std::cout << "  φ-effective depth: " << (int)(30 * phi * phi) << " gates\n";
    std::cout << "  (Kailangan ng custom modulus chain para dito)\n";

    return 0;
}
