// DM-DGR FHE: Minimal bootstrap attempt
// RingDim=2048 for memory, depth=4

#include <iostream>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

double time_ms(struct timeval s, struct timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}

int main() {
    std::cout << "\n  DM-DGR FHE: Minimal bootstrap test\n";
    std::cout << "  RingDim=2048\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(4);
    p.SetScalingModSize(50);
    p.SetBatchSize(512);
    p.SetRingDim(2048);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);
    
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "  Attempting bootstrap setup...\n";
    std::cout.flush();
    
    try {
        std::vector<uint32_t> levelBudget = {3, 1};
        std::vector<uint32_t> dim1 = {0, 0};
        cc->EvalBootstrapSetup(levelBudget, dim1, 256);
        cc->EvalBootstrapKeyGen(kp.secretKey, 256);
        std::cout << "  BOOTSTRAP SETUP: SUCCESS!\n\n";
        
        // Test: encrypt, multiply to deplete, bootstrap, verify
        auto enc_val = cc->Encrypt(kp.publicKey,
            cc->MakeCKKSPackedPlaintext(std::vector<double>{0.5}));
        
        std::cout << "  Initial level: " << enc_val->GetLevel() << "\n";
        
        // Deplete levels
        auto depleted = enc_val;
        for (int i = 0; i < 3; i++) {
            depleted = cc->EvalMult(depleted, enc_val);
        }
        std::cout << "  After 3 mults: level=" << depleted->GetLevel() << "\n";
        
        // Bootstrap
        struct timeval t0, t1;
        gettimeofday(&t0, NULL);
        auto refreshed = cc->EvalBootstrap(depleted);
        gettimeofday(&t1, NULL);
        
        std::cout << "  After bootstrap: level=" << refreshed->GetLevel() << "\n";
        std::cout << "  Bootstrap time: " << std::fixed << std::setprecision(0) 
                  << time_ms(t0, t1) << "ms\n";
        
        // Verify value preserved
        Plaintext pt_before, pt_after;
        cc->Decrypt(kp.secretKey, depleted, &pt_before);
        cc->Decrypt(kp.secretKey, refreshed, &pt_after);
        
        std::cout << "  Value before: " << pt_before->GetCKKSPackedValue()[0].real() << "\n";
        std::cout << "  Value after:  " << pt_after->GetCKKSPackedValue()[0].real() << "\n";
        std::cout << "  VALUE PRESERVED: " 
                  << (std::abs(pt_before->GetCKKSPackedValue()[0].real() - 
                               pt_after->GetCKKSPackedValue()[0].real()) < 0.01 ? "YES" : "NO") << "\n\n";
        
    } catch (const std::exception& e) {
        std::cout << "  Bootstrap failed: " << e.what() << "\n\n";
    }
    
    std::cout << "  If bootstrap succeeded: FHE achieved.\n";
    std::cout << "  If failed: Use cloud hardware or implement custom bootstrap.\n\n";
    
    return 0;
}
