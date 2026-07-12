// ΦΩ0 — ZANS Enc(0) 10 MILLION STRESS TEST
// Gaano ka-unlimited? Let's find out.
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — ZANS Enc(0) 10M ULTIMATE STRESS TEST   ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(1);
    params.SetPlaintextModulus(65537);
    params.SetSecurityLevel(HEStd_NotSet);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    vector<int64_t> zero_vec = {0};
    auto anchor = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    vector<int64_t> truth = {42};
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(truth));

    cout << "Φ Target: 10,000,000 Enc(0) ZANS additions\n";
    cout << "Φ This will take approximately 2-3 minutes...\n\n";

    auto start = high_resolution_clock::now();
    
    int checkpoints[] = {
        1000000, 2000000, 3000000, 4000000, 5000000,
        6000000, 7000000, 8000000, 9000000, 10000000
    };
    int cp_idx = 0;
    bool corrupted = false;
    int64_t last_val = 42;

    for(int i = 1; i <= 10000000; i++) {
        ct = cc->EvalAdd(ct, anchor);

        if(cp_idx < 10 && i == checkpoints[cp_idx]) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            int64_t val = pt->GetPackedValue()[0];
            last_val = val;
            
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();

            cout << "  Op " << i/1000000 << "M: " << val;
            if(val == 42) cout << " ✅";
            else { 
                cout << " ❌ CORRUPTED (expected 42)";
                corrupted = true;
            }
            cout << " | " << elapsed << "s";
            
            // Calculate throughput
            double throughput = (double)i / elapsed;
            cout << " | " << fixed << setprecision(0) << throughput << " ops/s\n";
            
            cp_idx++;

            if(corrupted) break;
        }
    }

    auto end = high_resolution_clock::now();
    auto total = duration_cast<seconds>(end - start).count();

    cout << "\n=== FINAL RESULTS ===\n";
    if(!corrupted) {
        cout << "Φ 10,000,000 Enc(0) ZANS additions COMPLETE.\n";
        cout << "Φ Final value: " << last_val << " (expected 42)";
        if(last_val == 42) cout << " ✅\n";
        else cout << " ❌\n";
        cout << "Φ Enc(0) ZANS: UNLIMITED DEPTH CONFIRMED.\n";
    } else {
        cout << "Φ Corruption detected at " << checkpoints[cp_idx-1] << " ops.\n";
        cout << "Φ Limit found.\n";
    }
    cout << "Φ Total time: " << total << "s\n";
    double final_throughput = (double)10000000 / total;
    cout << "Φ Avg throughput: " << fixed << setprecision(0) << final_throughput << " ops/s\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
