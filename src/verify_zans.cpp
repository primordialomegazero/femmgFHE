#include <openfhe/pke/openfhe.h>
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace lbcrypto;
using namespace std;
using namespace chrono;

int main() {
    cout << "╔══════════════════════════════════════════════════════╗" << endl;
    cout << "║           ZANS STABILITY TEST                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════════╝" << endl;
    cout << endl;
    
    try {
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
        
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
        auto zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        
        cout << "ZANS Test: 42 + (0 × N) = 42" << endl;
        cout << "Adding Enc(0) should NOT change the value" << endl;
        cout << "----------------------------------------" << endl;
        
        auto start = high_resolution_clock::now();
        
        const int STEPS = 100000;
        for(int i = 0; i < STEPS; i++) {
            ct = cc->EvalAdd(ct, zero);
            if((i+1) % 10000 == 0) {
                Plaintext result;
                cc->Decrypt(keys.secretKey, ct, &result);
                int64_t val = result->GetPackedValue()[0];
                auto now = high_resolution_clock::now();
                auto elapsed = duration_cast<milliseconds>(now - start).count();
                cout << "  " << setw(6) << (i+1) << " additions: "
                     << "Value = " << val
                     << " | Time: " << fixed << setprecision(2) 
                     << (elapsed / 1000.0) << "s" << endl;
                
                if(val != 42) {
                    cout << "❌ FAILED: Value changed to " << val << endl;
                    return 1;
                }
            }
        }
        
        Plaintext final;
        cc->Decrypt(keys.secretKey, ct, &final);
        
        cout << endl;
        cout << "═══════════════════════════════════════════════════════" << endl;
        cout << "FINAL: 42 + (0 × 100,000) = " << final->GetPackedValue()[0] << endl;
        cout << "✅ ZANS STABLE: 42 == 42" << endl;
        cout << "═══════════════════════════════════════════════════════" << endl;
        
        return 0;
        
    } catch(const exception& e) {
        cerr << "❌ Error: " << e.what() << endl;
        return 1;
    }
}
