// ΦΩ0 — ZANS DIRECT VERIFICATION (No Estimates!)
// Instead of GetNoiseScaleDeg(), we DECRYPT and CHECK
// Every 100K ops: decrypt, verify value is correct
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
    cout <<   "║  ΦΩ0 — ZANS DIRECT DECRYPTION VERIFICATION   ║\n";
    cout <<   "║  No estimates — just decrypt and check       ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    // Create ZANS anchor
    vector<int64_t> zeroVec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeroVec));
    
    // Encrypt test value: 42
    int64_t test_val = 42;
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{test_val}));
    
    cout << "Φ Initial value: " << test_val << "\n";
    cout << "Φ Running ZANS additions with DIRECT DECRYPTION CHECKS...\n\n";
    
    int checkpoints[] = {100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};
    int num_checks = sizeof(checkpoints)/sizeof(checkpoints[0]);
    int next_check = 0;
    
    auto start = high_resolution_clock::now();
    
    for(int i = 1; i <= 1000000; i++) {
        ct = cc->EvalAdd(ct, anchor0);
        
        // Direct verification at checkpoints
        if(next_check < num_checks && i == checkpoints[next_check]) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            int64_t decrypted = pt->GetPackedValue()[0];
            
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            
            bool correct = (decrypted == test_val);
            
            cout << "  [" << setw(8) << i << " ops] ";
            cout << "Decrypted: " << setw(6) << decrypted;
            cout << " | Expected: " << setw(6) << test_val;
            cout << " | " << (correct ? "✅ CORRECT" : "❌ CORRUPTED!");
            cout << " | " << elapsed << "s\n";
            
            if(!correct) {
                cout << "\n❌ ZANS FAILED at " << i << " operations!\n";
                return 1;
            }
            
            next_check++;
        }
    }
    
    auto end = high_resolution_clock::now();
    auto total = duration_cast<seconds>(end - start).count();
    
    // Final verification
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    int64_t final_val = pt->GetPackedValue()[0];
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  DIRECT VERIFICATION RESULTS                 ║\n";
    cout <<   "╠══════════════════════════════════════════════╣\n";
    cout <<   "║  Operations: 1,000,000                       ║\n";
    cout <<   "║  Start value: 42                             ║\n";
    cout <<   "║  Final decrypted: " << setw(28) << final_val << " ║\n";
    cout <<   "║  Expected: 42                                 ║\n";
    cout <<   "║  Checks passed: " << num_checks << "/" << num_checks << "                             ║\n";
    cout <<   "║  Total time: " << setw(33) << total << "s ║\n";
    
    if(final_val == test_val) {
        cout << "║                                              ║\n";
        cout << "║  ✅ ZANS VERIFIED BY DIRECT DECRYPTION       ║\n";
        cout << "║  No estimates. No approximations.            ║\n";
        cout << "║  Just: decrypt → check → CORRECT.            ║\n";
    }
    
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
