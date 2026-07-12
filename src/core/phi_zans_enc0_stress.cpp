// ΦΩ0 — ZANS Enc(0) ULTIMATE STRESS TEST
// How far can Enc(0) additions go?
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
    cout <<   "║  ΦΩ0 — ZANS Enc(0) ULTIMATE STRESS TEST       ║\n";
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

    // Create Enc(0) anchor
    vector<int64_t> zero_vec = {0};
    auto anchor = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    // Seal truth
    vector<int64_t> truth = {42};
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(truth));

    cout << "Φ Starting Enc(0) ZANS stress test...\n\n";

    auto start = high_resolution_clock::now();
    int checkpoints[] = {1000, 10000, 50000, 100000, 250000, 500000, 1000000};
    int cp_idx = 0;
    bool corrupted = false;

    for(int i = 1; i <= 1000000; i++) {
        ct = cc->EvalAdd(ct, anchor);

        if(cp_idx < 7 && i == checkpoints[cp_idx]) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            int64_t val = pt->GetPackedValue()[0];
            
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();

            cout << "  Op " << i << ": " << val;
            if(val == 42) cout << " ✅";
            else { 
                cout << " ❌ CORRUPTED!";
                corrupted = true;
            }
            cout << " | " << elapsed << "s\n";
            cp_idx++;

            if(corrupted) break;
        }

        if(corrupted) break;
    }

    auto end = high_resolution_clock::now();
    auto total = duration_cast<seconds>(end - start).count();

    cout << "\n=== RESULTS ===\n";
    if(!corrupted) {
        cout << "Φ 1,000,000 Enc(0) ZANS additions COMPLETE.\n";
        cout << "Φ Value: 42 ✅ preserved.\n";
        cout << "Φ Enc(0) ZANS appears UNLIMITED.\n";
    } else {
        cout << "Φ Corruption detected.\n";
    }
    cout << "Φ Total time: " << total << "s\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
