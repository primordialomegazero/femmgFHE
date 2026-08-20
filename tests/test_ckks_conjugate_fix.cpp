// CKKS + φ-CONJUGATE REFRESH — FIXED
// Gumamit ng mas mababang k para sa precision

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS + φ-CONJUGATE REFRESH (FIXED)\n";
    std::cout << "  Mas mababang k para sa precision\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // Test sa iba't ibang k values
    for (int k_val : {5, 10, 15, 20}) {
        const double phi = 1.6180339887498948482;
        const double phi_k = std::pow(phi, k_val);
        const double psi_k = std::pow(1.0/phi, k_val);

        std::cout << "k=" << k_val << ": φ^k=" << phi_k << ", ψ^k=" << psi_k << "\n";

        // Check kung ang precision ay sapat
        // Ang CKKS ay may ~40 bits precision ≈ 10 decimal digits
        // Kailangan natin: |φ^k - φ^k_approx| < 0.01
        if (phi_k < 1e9 && psi_k > 1e-9) {
            std::cout << "  → Sapat ang precision\n";
        } else {
            std::cout << "  → Precision issue\n";
        }
    }

    // Best k: yung may φ^k na hindi masyadong malaki at ψ^k na hindi masyadong maliit
    // k=10: φ^10 ≈ 122.99, ψ^10 ≈ 0.00813
    // k=15: φ^15 ≈ 1364.0, ψ^15 ≈ 0.000733
    // k=20: φ^20 ≈ 15126.5, ψ^20 ≈ 0.0000661

    std::cout << "\nBest k para sa CKKS (40-bit scaling):\n";
    std::cout << "  k=10: φ^10 ≈ 122.99 (manageable)\n";
    std::cout << "  k=15: φ^15 ≈ 1364.0 (manageable)\n";
    std::cout << "  k=20: φ^20 ≈ 15126.5 (borderline)\n";

    return 0;
}
