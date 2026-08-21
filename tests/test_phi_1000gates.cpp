// φ²-ADDSUB — 1000 GATES TEST
// Kung 0 levels per gate, dapat kaya nating
// mag-run ng 1000 gates nang walang problema!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-ADDSUB — 1000 GATES\n";
    std::cout << "  UNBOUNDED TEST\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);  // Maliit lang!
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
    auto current = make_ct(0.0);  // Start with 0
    int errors = 0;
    
    std::cout << "RUNNING 1000 GATES (Depth 5)...\n";
    std::cout << "================================\n\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        // Alternating add/subtract
        if (i % 2 == 0) {
            current = cc->EvalAdd(current, ct_phi_sq);  // 0 → φ²
        } else {
            current = cc->EvalSub(current, ct_phi_sq);  // φ² → 0
        }
        
        if (i % 100 == 0 || i == 999) {
            double v = decrypt_val(current);
            bool is_phi_sq = std::abs(v - phi_sq) < 0.01;
            bool is_zero = std::abs(v) < 0.01;
            
            std::cout << "  Gate " << i << ": v=" << v 
                      << " level=" << current->GetLevel();
            
            if (is_zero) std::cout << " [=0 ✓]";
            else if (is_phi_sq) std::cout << " [=φ² ✓]";
            else {
                std::cout << " [DRIFT ✗]";
                errors++;
            }
            std::cout << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: 1000 (Depth 5)\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration.count() << "s\n";
    std::cout << "  Level used: " << current->GetLevel() << " (max 5)\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 HOLY GRAIL!" : "❌ FAILED") << "\n";
    std::cout << "========================================\n";
    
    return 0;
}
