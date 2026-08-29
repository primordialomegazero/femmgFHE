// SPEED TEST: Different parameters
#include <iostream>
#include <vector>
#include <chrono>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "φ-SPEED TEST\n\n";
    
    // TEST 1: Light params (depth 10)
    cout << "TEST 1: Depth 10, 100 ops\n";
    {
        CCParams<CryptoContextCKKSRNS> p1;
        p1.SetMultiplicativeDepth(10);
        p1.SetScalingModSize(30);
        p1.SetBatchSize(8);
        p1.SetSecurityLevel(HEStd_128_classic);
        
        auto cc1 = GenCryptoContext(p1);
        cc1->Enable(PKE);
        cc1->Enable(KEYSWITCH);
        cc1->Enable(LEVELEDSHE);
        
        auto kp1 = cc1->KeyGen();
        
        vector<double> v(8, 1.0);
        auto pt1 = cc1->MakeCKKSPackedPlaintext(v);
        auto ct1 = cc1->Encrypt(kp1.publicKey, pt1);
        
        auto start = high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            ct1 = cc1->EvalAdd(ct1, ct1);
        }
        auto end = high_resolution_clock::now();
        
        cout << "  Time: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
        cout << "  Level: " << ct1->GetLevel() << "\n\n";
    }
    
    // TEST 2: Ultra light (depth 1)
    cout << "TEST 2: Depth 1, 1000 ops\n";
    {
        CCParams<CryptoContextCKKSRNS> p2;
        p2.SetMultiplicativeDepth(1);
        p2.SetScalingModSize(20);
        p2.SetBatchSize(8);
        p2.SetSecurityLevel(HEStd_128_classic);
        
        auto cc2 = GenCryptoContext(p2);
        cc2->Enable(PKE);
        cc2->Enable(KEYSWITCH);
        cc2->Enable(LEVELEDSHE);
        
        auto kp2 = cc2->KeyGen();
        
        vector<double> v(8, 1.0);
        auto pt2 = cc2->MakeCKKSPackedPlaintext(v);
        auto ct2 = cc2->Encrypt(kp2.publicKey, pt2);
        
        auto start = high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) {
            ct2 = cc2->EvalAdd(ct2, ct2);
        }
        auto end = high_resolution_clock::now();
        
        cout << "  Time: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
        cout << "  Level: " << ct2->GetLevel() << "\n\n";
    }
    
    return 0;
}
