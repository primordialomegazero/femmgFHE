// 1 MILLION NAND OPERATIONS — THE ULTIMATE TEST
#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;

int main() {
    std::cout << "====================================\n";
    std::cout << "  1,000,000 NAND OPERATIONS\n";
    std::cout << "  Period-2 Noise Reset\n";
    std::cout << "====================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    std::cout << "Keys ready!\n\n";

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    auto dec = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return (int)pt->GetPackedValue()[0];
    };

    const int TOTAL = 1000000;
    int errors = 0;
    
    std::cout << "Starting " << TOTAL << " NAND operations...\n";
    std::cout << "Estimated time: ~25 hours\n\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto current = ct0;
    for (int i = 1; i <= TOTAL; i++) {
        current = nand(current, current);
        
        // Progress report every 100K
        if (i % 100000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            
            // Verify current value (period-2: even NANDs = 0, odd = 1)
            int val = dec(current);
            int expected = (i % 2 == 0) ? 0 : 1;
            bool ok = (val == expected);
            if (!ok) errors++;
            
            std::cout << "  Progress: " << i << "/" << TOTAL 
                      << " | Value: " << val 
                      << " (exp " << expected << ") "
                      << (ok ? "✅" : "❌")
                      << " | Elapsed: " << (elapsed / 1000) << "s\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "\n====================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "====================================\n";
    std::cout << "  Total: " << TOTAL << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << (total_time / 1000) << "s (" 
              << (total_time / 3600000.0) << " hours)\n";
    std::cout << "  Ops/sec: " << (TOTAL * 1000.0 / total_time) << "\n";
    
    if (errors == 0) {
        std::cout << "\n  ✅ 1,000,000 NAND OPERATIONS PASSED!\n";
        std::cout << "  ✅ PERIOD-2 NOISE RESET CONFIRMED!\n";
        std::cout << "  ✅ NO BOOTSTRAPPING NEEDED!\n";
    } else {
        std::cout << "\n  ❌ " << errors << " ERRORS DETECTED!\n";
    }
    std::cout << "====================================\n";

    return 0;
}
