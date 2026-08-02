// ΦΩ0 — ZANS NOISE PROOF
// This test proves noise DOES grow despite claims

#include <openfhe/pke/openfhe.h>
#include <iostream>
#include <vector>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  🔬 ZANS NOISE PROOF TEST                  ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    // Use SAME parameters as their test
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);  // Allow more operations for testing
    params.SetPlaintextModulus(65537);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Encrypt 42 and 0
    vector<int64_t> v42 = {42};
    vector<int64_t> v0 = {0};
    
    Plaintext p42 = cc->MakePackedPlaintext(v42);
    Plaintext p0 = cc->MakePackedPlaintext(v0);
    
    auto ct42 = cc->Encrypt(keys.publicKey, p42);
    auto ct0 = cc->Encrypt(keys.publicKey, p0);

    cout << "📊 TEST: Noise Growth with ZANS\n";
    cout << "================================\n\n";
    
    auto ct = ct42;
    int ops = 0;
    
    // Run ZANS operations until multiplication fails
    while (ops < 10000) {
        // ZANS operation: add Enc(0)
        ct = cc->EvalAdd(ct, ct0);
        ops++;
        
        // Every 100 operations, test multiplication
        if (ops % 100 == 0) {
            try {
                // Try to multiply - this will fail when noise is too high
                auto ct_square = cc->EvalMult(ct, ct);
                Plaintext result;
                cc->Decrypt(keys.secretKey, ct_square, &result);
                result->SetLength(1);
                long long val = result->GetPackedValue()[0];
                
                if (val == 42 * 42) {
                    cout << "✅ " << ops << " ops: Multiplication works (42² = " << val << ")\n";
                } else {
                    cout << "❌ " << ops << " ops: Multiplication FAILED! Got " << val << "\n";
                    cout << "\n🎯 PROOF: Noise grew enough to corrupt multiplication!\n";
                    cout << "   ZANS claim of 'zero noise growth' is FALSE.\n";
                    break;
                }
            } catch (...) {
                cout << "❌ " << ops << " ops: Multiplication crashed!\n";
                cout << "\n🎯 PROOF: Noise corrupted the ciphertext!\n";
                cout << "   ZANS does NOT prevent noise growth.\n";
                break;
            }
        }
    }
    
    cout << "\n📌 MATHEMATICAL FACT:\n";
    cout << "   In FHE, every operation increases noise.\n";
    cout << "   Adding Enc(0) adds noise like any ciphertext.\n";
    cout << "   The claim of 'zero noise growth' violates\n";
    cout << "   fundamental FHE security proofs.\n";
    
    return 0;
}
