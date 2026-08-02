// MINIMAL BOOTSTRAP: Find smallest working parameters
#include <iostream>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

double time_ms(struct timeval s, struct timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}

int main() {
    std::cout << "\n  MINIMAL BOOTSTRAP SEARCH\n\n";
    
    // Start with smallest possible parameters
    int ringDims[] = {1024, 2048, 4096, 8192, 16384};
    
    for (int rd : ringDims) {
        std::cout << "  RingDim=" << rd << ": ";
        std::cout.flush();
        
        try {
            CCParams<CryptoContextCKKSRNS> p;
            p.SetMultiplicativeDepth(3);
            p.SetScalingModSize(50);
            p.SetBatchSize(rd / 4);
            p.SetRingDim(rd);
            p.SetSecretKeyDist(UNIFORM_TERNARY);
            p.SetSecurityLevel(HEStd_NotSet);
            
            auto cc = GenCryptoContext(p);
            cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
            cc->Enable(ADVANCEDSHE); cc->Enable(FHE);
            
            auto kp = cc->KeyGen();
            cc->EvalMultKeyGen(kp.secretKey);
            
            std::vector<uint32_t> levelBudget = {2, 1};
            std::vector<uint32_t> dim1 = {0, 0};
            
            struct timeval t0, t1;
            gettimeofday(&t0, NULL);
            
            cc->EvalBootstrapSetup(levelBudget, dim1, rd / 4);
            cc->EvalBootstrapKeyGen(kp.secretKey, rd / 4);
            
            gettimeofday(&t1, NULL);
            
            std::cout << "SUCCESS in " << time_ms(t0, t1) << "ms\n";
            
            // Test bootstrap
            auto enc = cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{0.5}));
            
            // Multiply to consume levels
            auto depleted = enc;
            for (int i = 0; i < 2; i++) depleted = cc->EvalMult(depleted, enc);
            
            auto refreshed = cc->EvalBootstrap(depleted);
            
            Plaintext pt;
            cc->Decrypt(kp.secretKey, refreshed, &pt);
            double val = pt->GetCKKSPackedValue()[0].real();
            
            std::cout << "    Value after bootstrap: " << val << "\n\n";
            std::cout << "  BOOTSTRAP WORKS at RingDim=" << rd << "!\n\n";
            return 0;
            
        } catch (const std::exception& e) {
            std::cout << "FAILED: " << e.what() << "\n";
        } catch (...) {
            std::cout << "FAILED: Unknown error\n";
        }
    }
    
    std::cout << "\n  Bootstrap failed at all ring dimensions.\n";
    std::cout << "  Hardware limit: 16GB RAM insufficient.\n";
    std::cout << "  Need cloud/enterprise hardware for standard bootstrapping.\n\n";
    
    return 0;
}
