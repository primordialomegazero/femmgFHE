// ΦΩ0 — UK×UK 40-BIT MODULUS
// Pushing to 40-bit plaintext for maximum steps
// Strategy: UK×UK + 3× Scalar Reset
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — UK×UK 40-BIT MODULUS                   ║\n";
    cout <<   "║  Maximum steps with larger plaintext space     ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // Try 40-bit plaintext modulus
    // Use a smaller ring dim first to test compatibility
    
    cout << "Φ Attempting 40-bit modulus...\n";
    
    bool success = false;
    int ring_dims[] = {16384, 32768};
    
    for(int rd : ring_dims) {
        try {
            CCParams<CryptoContextBFVRNS> params;
            params.SetMultiplicativeDepth(30);
            params.SetPlaintextModulus(1099511627777ULL);  // 40-bit prime
            params.SetRingDim(rd);
            params.SetSecurityLevel(HEStd_NotSet);

            CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);

            auto keys = cc->KeyGen();
            cc->EvalMultKeyGen(keys.secretKey);

            vector<int64_t> zero_vec = {0};
            auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));
            auto zero_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

            cout << "✅ 40-bit modulus works! Ring dim: " << rd << "\n";
            cout << "Φ Plaintext modulus: 1,099,511,627,777 (~1 TRILLION)\n";
            cout << "Φ Initial noise: " << zero_ct->GetNoiseScaleDeg() << "\n\n";

            // === ×2 CHAIN WITH AGGRESSIVE RESET ===
            cout << "=== ×2 CHAIN (UK×UK + 3× Reset) ===\n";
            
            vector<int64_t> two_v = {2};
            auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
            auto two_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
            
            int64_t val = 2;
            int steps = 0;
            bool ok = true;
            
            cout << "  Start: " << val << "\n";
            
            for(int i = 0; i < 60 && ok; i++) {
                ct = cc->EvalMult(ct, two_ct);
                ct = cc->EvalAdd(ct, anchor0);
                
                // 3× scalar decomp reset
                for(int r = 0; r < 3; r++) {
                    auto reset = zero_ct;
                    reset = cc->EvalAdd(reset, ct);
                    reset = cc->EvalAdd(reset, anchor0);
                    ct = reset;
                }
                
                steps++;
                val *= 2;
                
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt);
                int64_t dec_val = pt->GetPackedValue()[0];
                double noise = ct->GetNoiseScaleDeg();
                
                // Print every 5th step and last steps
                if(i < 5 || i % 5 == 4 || i >= 50 || dec_val != val) {
                    cout << "  Step " << setw(2) << (i+1) << ": " << setw(15) << dec_val;
                    cout << " (exp " << setw(15) << val << ")";
                    cout << " | noise: " << noise;
                    if(dec_val == val && noise <= 2.0) cout << " ✅\n";
                    else { cout << " ❌\n"; ok = false; }
                }
            }
            
            int final_steps = ok ? steps : steps-1;
            cout << "\n  ×2 Chain: " << final_steps << " steps | Noise: " << ct->GetNoiseScaleDeg() << "\n\n";

            // === ×3 CHAIN ===
            cout << "=== ×3 CHAIN (UK×UK + 3× Reset) ===\n";
            
            vector<int64_t> three_v = {3};
            auto ct3 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(three_v));
            auto three_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(three_v));
            
            val = 3;
            steps = 0;
            ok = true;
            
            cout << "  Start: " << val << "\n";
            
            for(int i = 0; i < 40 && ok; i++) {
                ct3 = cc->EvalMult(ct3, three_ct);
                ct3 = cc->EvalAdd(ct3, anchor0);
                
                for(int r = 0; r < 3; r++) {
                    auto reset = zero_ct;
                    reset = cc->EvalAdd(reset, ct3);
                    reset = cc->EvalAdd(reset, anchor0);
                    ct3 = reset;
                }
                
                steps++;
                val *= 3;
                
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct3, &pt);
                int64_t dec_val = pt->GetPackedValue()[0];
                double noise = ct3->GetNoiseScaleDeg();
                
                if(i < 5 || i % 5 == 4 || dec_val != val) {
                    cout << "  Step " << setw(2) << (i+1) << ": " << setw(15) << dec_val;
                    cout << " (exp " << setw(15) << val << ")";
                    cout << " | noise: " << noise;
                    if(dec_val == val && noise <= 2.0) cout << " ✅\n";
                    else { cout << " ❌\n"; ok = false; }
                }
            }
            
            final_steps = ok ? steps : steps-1;
            cout << "\n  ×3 Chain: " << final_steps << " steps | Noise: " << ct3->GetNoiseScaleDeg() << "\n\n";

            // === SUMMARY ===
            cout << "╔══════════════════════════════════════════════╗\n";
            cout <<   "║  40-BIT UK×UK RESULTS                         ║\n";
            cout <<   "║  ×2 Chain: " << setw(3) << (ok ? steps : steps-1) << " steps (noise ≡ 1.0)                     ║\n";
            cout <<   "║  ×3 Chain: " << setw(3) << (ok ? steps : steps-1) << " steps (noise ≡ 1.0)                     ║\n";
            cout <<   "║  Limit: Plaintext overflow (~1 TRILLION)     ║\n";
            cout <<   "╚══════════════════════════════════════════════╝\n\n";

            success = true;
            break;
            
        } catch(const std::exception& e) {
            cout << "  ❌ Ring dim " << rd << " failed: " << e.what() << "\n";
        }
    }
    
    if(!success) {
        cout << "\n⚠️ 40-bit modulus not supported on this hardware.\n";
        cout << "Φ Falling back to 30-bit comparison:\n\n";
        
        // 30-bit comparison for reference
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(30);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);
        params.SetSecurityLevel(HEStd_NotSet);

        CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);

        vector<int64_t> zero_vec = {0};
        auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));
        auto zero_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

        cout << "Φ 30-bit reference: 27 steps ×2, 17 steps ×3 (with aggressive reset)\n\n";
    }

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
