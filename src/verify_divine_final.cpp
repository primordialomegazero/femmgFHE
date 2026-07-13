// Use minimal includes para iwas dependency issues
#include <openfhe/pke/openfhe.h>
#include <iostream>
#include <vector>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "╔══════════════════════════════════════════════════════╗" << endl;
    cout << "║        VERIFY DIVINE: BASIC MULTIPLICATION          ║" << endl;
    cout << "╚══════════════════════════════════════════════════════╝" << endl;
    cout << endl;
    
    try {
        // Setup
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(30);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);
        params.SetSecurityLevel(HEStd_NotSet);
        
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(LEVELEDSHE);
        cc->Enable(KEYSWITCH);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        // Test: 7 * 2^10 = 7168
        cout << "Test: 7 × 2^10 = 7 × 1024 = 7168" << endl;
        cout << "----------------------------------------" << endl;
        
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{7}));
        
        for(int i = 0; i < 10; i++) {
            auto ct2 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
            ct = cc->EvalMult(ct, ct2);
        }
        
        Plaintext result;
        cc->Decrypt(keys.secretKey, ct, &result);
        int64_t val = result->GetPackedValue()[0];
        
        cout << "Result: " << val << endl;
        cout << "Expected: 7168" << endl;
        cout << "Status: " << (val == 7168 ? "✅ PASSED" : "❌ FAILED") << endl;
        cout << endl;
        
        return (val == 7168) ? 0 : 1;
        
    } catch(const exception& e) {
        cerr << "❌ Error: " << e.what() << endl;
        return 1;
    }
}
