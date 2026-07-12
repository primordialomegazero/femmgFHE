// ΦΩ0 — CKKS BOOTSTRAP DEBUG
// Isolate the segfault in EvalBootstrapSetup
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — CKKS BOOTSTRAP DEBUG                   ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    // Try 1: Minimal CKKS with FHE, no bootstrap setup
    cout << "=== TRY 1: CKKS + FHE, NO BOOTSTRAP SETUP ===\n";
    try {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetRingDim(16384);
        params.SetMultiplicativeDepth(3);
        params.SetScalingModSize(50);
        params.SetBatchSize(8);
        
        CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        cc->Enable(ADVANCEDSHE);
        cc->Enable(FHE);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        cc->EvalSumKeyGen(keys.secretKey);
        
        cout << "  ✅ CKKS+FHE context created (no bootstrap setup)\n";
        cout << "  Ring: " << cc->GetRingDimension() << "\n\n";
        
        // Try bootstrap setup with VERY minimal budget
        cout << "=== TRY 2: BOOTSTRAP SETUP (levelBudget={5,4}) ===\n";
        
        CCParams<CryptoContextCKKSRNS> params2;
        params2.SetRingDim(16384);
        params2.SetMultiplicativeDepth(3);
        params2.SetScalingModSize(50);
        params2.SetBatchSize(8);
        
        CryptoContext<DCRTPoly> cc2 = GenCryptoContext(params2);
        cc2->Enable(PKE);
        cc2->Enable(KEYSWITCH);
        cc2->Enable(LEVELEDSHE);
        cc2->Enable(ADVANCEDSHE);
        cc2->Enable(FHE);
        
        auto keys2 = cc2->KeyGen();
        cc2->EvalMultKeyGen(keys2.secretKey);
        cc2->EvalSumKeyGen(keys2.secretKey);
        
        vector<uint32_t> levelBudget = {5, 4};
        vector<uint32_t> dim1 = {0, 0};
        
        cout << "  Calling EvalBootstrapSetup...\n";
        cc2->EvalBootstrapSetup(levelBudget, dim1);
        cout << "  ✅ Bootstrap setup succeeded!\n\n";
        
        // Try actual bootstrap
        cout << "=== TRY 3: FULL BOOTSTRAP ===\n";
        
        vector<double> truth = {1.618};
        Plaintext pt = cc2->MakeCKKSPackedPlaintext(truth);
        auto ct = cc2->Encrypt(keys2.publicKey, pt);
        cout << "  Encrypted φ\n";
        
        auto degraded = cc2->EvalSquare(ct);
        degraded = cc2->Relinearize(degraded);
        degraded = cc2->ModReduce(degraded);
        cout << "  One square done\n";
        
        cout << "  Calling EvalBootstrap...\n";
        auto renewed = cc2->EvalBootstrap(degraded);
        cout << "  ✅ Bootstrap complete!\n";
        
        Plaintext result_pt;
        cc2->Decrypt(keys2.secretKey, renewed, &result_pt);
        auto final = result_pt->GetRealPackedValue();
        cout << "  φ² ≈ " << final << " (expected ~" << (1.618*1.618) << ")\n\n";
        
    } catch(const exception& e) {
        cout << "  ❌ Error: " << e.what() << "\n\n";
    }
    
    // Try 2: Different bootstrap parameters
    cout << "=== TRY 4: BOOTSTRAP SETUP (levelBudget={4,3}) ===\n";
    try {
        CCParams<CryptoContextCKKSRNS> params3;
        params3.SetRingDim(16384);
        params3.SetMultiplicativeDepth(2);
        params3.SetScalingModSize(50);
        params3.SetBatchSize(8);
        
        CryptoContext<DCRTPoly> cc3 = GenCryptoContext(params3);
        cc3->Enable(PKE);
        cc3->Enable(KEYSWITCH);
        cc3->Enable(LEVELEDSHE);
        cc3->Enable(ADVANCEDSHE);
        cc3->Enable(FHE);
        
        auto keys3 = cc3->KeyGen();
        cc3->EvalMultKeyGen(keys3.secretKey);
        cc3->EvalSumKeyGen(keys3.secretKey);
        
        vector<uint32_t> levelBudget = {4, 3};
        vector<uint32_t> dim1 = {0, 0};
        
        cout << "  Calling EvalBootstrapSetup...\n";
        cc3->EvalBootstrapSetup(levelBudget, dim1);
        cout << "  ✅ Bootstrap setup succeeded!\n\n";
        
    } catch(const exception& e) {
        cout << "  ❌ Error: " << e.what() << "\n\n";
    }
    
    // Try 3: No bootstrap setup, check if we can at least use CKKS+FHE
    cout << "=== TRY 5: CKKS+FHE BASIC OPERATIONS ===\n";
    try {
        CCParams<CryptoContextCKKSRNS> params4;
        params4.SetRingDim(16384);
        params4.SetMultiplicativeDepth(3);
        params4.SetScalingModSize(50);
        params4.SetBatchSize(8);
        
        CryptoContext<DCRTPoly> cc4 = GenCryptoContext(params4);
        cc4->Enable(PKE);
        cc4->Enable(KEYSWITCH);
        cc4->Enable(LEVELEDSHE);
        cc4->Enable(ADVANCEDSHE);
        cc4->Enable(FHE);
        
        auto keys4 = cc4->KeyGen();
        cc4->EvalMultKeyGen(keys4.secretKey);
        cc4->EvalSumKeyGen(keys4.secretKey);
        
        vector<double> truth = {3.14159};
        Plaintext pt = cc4->MakeCKKSPackedPlaintext(truth);
        auto ct = cc4->Encrypt(keys4.publicKey, pt);
        
        auto squared = cc4->EvalSquare(ct);
        squared = cc4->Relinearize(squared);
        squared = cc4->ModReduce(squared);
        
        Plaintext result_pt;
        cc4->Decrypt(keys4.secretKey, squared, &result_pt);
        auto final = result_pt->GetRealPackedValue();
        
        cout << "  π² ≈ " << final << " (expected ~" << (3.14159*3.14159) << ")\n";
        cout << "  ✅ CKKS+FHE basic operations work!\n";
        cout << "  ⚠️  But EvalBootstrapSetup segfaults.\n\n";
        
    } catch(const exception& e) {
        cout << "  ❌ Error: " << e.what() << "\n\n";
    }
    
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";
    
    return 0;
}
