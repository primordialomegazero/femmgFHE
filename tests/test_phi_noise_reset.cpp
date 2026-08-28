// PHI NOISE RESET — Golden Ratio as Natural Bootstrap
// Testing if φ rotation can reset noise without traditional bootstrapping

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI NOISE RESET\n";
    std::cout << "  Golden Ratio as Natural Bootstrap\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(65537);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](int64_t val) {
        std::vector<int64_t> vec(slots, 0);
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // φ-related values sa modulo space
    // φ = 1.618... → 40503 sa mod 65537 (closest integer)
    const int64_t PHI_INT = 40503;  // 65537 * 0.618...
    const int64_t PHI_SQ_INT = 25034;  // 65537 * 0.382...
    
    auto ct_phi = make_ct(PHI_INT);
    auto ct_one = make_ct(1);
    auto ct_zero = make_ct(0);

    std::cout << "PHI NOISE RESET TEST:\n";
    std::cout << "====================\n\n";
    
    // Test: 100 gates with φ rotation every 10 gates
    auto ct_a = ct_zero;
    auto ct_b = ct_one;
    
    int errors = 0;
    int phi_rotations = 0;
    
    for (int i = 0; i < 100; i++) {
        // NAND operation
        auto ct_and = cc->EvalMult(ct_a, ct_b);
        auto ct_nand = cc->EvalSub(ct_one, ct_and);
        
        // φ rotation every 10 gates (natural refresh cycle)
        if (i > 0 && i % 10 == 0) {
            ct_nand = cc->EvalAdd(ct_nand, ct_phi);
            phi_rotations++;
        }
        
        // Binary correction
        auto ct_corrected = cc->EvalMult(ct_nand, ct_nand);
        
        // Check result
        int64_t result = decrypt_val(ct_corrected);
        int bit = (result > 32768) ? 1 : 0;
        
        // Expected: alternating
        int expected = (i % 2 == 0) ? 1 : 0;
        
        if (bit != expected) {
            errors++;
        }
        
        ct_a = ct_b;
        ct_b = ct_corrected;
        
        if (i % 10 == 0) {
            std::cout << "  Gate " << i << ": bit=" << bit 
                      << " expected=" << expected
                      << " φ_rotations=" << phi_rotations
                      << " errors=" << errors << "\n";
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n";
    std::cout << "  Gates: 100\n";
    std::cout << "  φ Rotations: " << phi_rotations << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Error rate: " << (errors * 100.0 / 100) << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ HOLY GRAIL!" : 
              (errors < 10 ? "⚠️ CLOSE!" : "✗ NEEDS WORK")) << "\n";
    std::cout << "========================================\n";
    
    return 0;
}
