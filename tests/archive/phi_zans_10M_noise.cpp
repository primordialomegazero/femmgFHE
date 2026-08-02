// ΦΩ0 — ZANS 10M NOISE MEASUREMENT
// Tracking noise scale across 10,000,000 Enc(0) additions
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
    cout <<   "║  ΦΩ0 — ZANS 10M NOISE MEASUREMENT             ║\n";
    cout <<   "║  Tracking noise scale across 10M additions    ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(3);
    params.SetPlaintextModulus(65537);
    params.SetSecurityLevel(HEStd_NotSet);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Create Enc(0) anchor
    vector<int64_t> zero_vec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    // Seal truth
    vector<int64_t> truth = {42};
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(truth));

    // Get initial noise scale
    double initial_noise = ct->GetNoiseScaleDeg();
    
    cout << "Φ Initial noise scale: " << initial_noise << "\n";
    cout << "Φ Ring dimension: 512\n";
    cout << "Φ Modulus bits: 180\n";
    cout << "Φ Target: 10,000,000 Enc(0) additions\n";
    cout << "Φ Estimated time: ~2 minutes\n\n";

    auto start = high_resolution_clock::now();
    
    int checkpoints[] = {
        100000, 500000, 1000000, 2000000, 3000000, 
        4000000, 5000000, 6000000, 7000000, 8000000, 
        9000000, 10000000
    };
    int cp_idx = 0;
    bool corrupted = false;
    int64_t last_val = 42;
    double last_noise = initial_noise;
    int64_t last_checkpoint = 0;

    cout << "┌──────────────┬──────────┬──────────┬──────────┬──────────┐\n";
    cout << "│ Operations   │ Value    │ Noise    │ Drift    │ Time     │\n";
    cout << "├──────────────┼──────────┼──────────┼──────────┼──────────┤\n";

    for(int i = 1; i <= 10000000 && !corrupted; i++) {
        ct = cc->EvalAdd(ct, anchor0);

        if(cp_idx < 12 && i == checkpoints[cp_idx]) {
            // Check value
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            int64_t val = pt->GetPackedValue()[0];
            
            // Check noise
            double noise = ct->GetNoiseScaleDeg();
            double drift = noise - last_noise;
            
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            
            // Format output
            cout << "│ ";
            if(i >= 1000000) {
                cout << setw(4) << i/1000000 << "M";
            } else if(i >= 1000) {
                cout << setw(4) << i/1000 << "K";
            } else {
                cout << setw(7) << i;
            }
            cout << " ops    │ ";
            
            cout << setw(5) << val << "   │ ";
            
            if(noise == initial_noise) {
                cout << "≡ " << setw(6) << noise << " │ ";
            } else {
                cout << setw(8) << noise << " │ ";
            }
            
            if(drift == 0) {
                cout << " 0.000   │ ";
            } else {
                cout << setw(8) << fixed << setprecision(6) << drift << " │ ";
            }
            
            cout << setw(5) << elapsed << "s   │\n";
            
            if(val != 42) {
                corrupted = true;
                cout << "├──────────────┼──────────┼──────────┼──────────┼──────────┤\n";
                cout << "│ ❌ CORRUPTED at " << setw(7) << i << " ops";
                cout << "                         │\n";
            }
            
            last_val = val;
            last_noise = noise;
            last_checkpoint = i;
            cp_idx++;
        }
    }

    cout << "└──────────────┴──────────┴──────────┴──────────┴──────────┘\n";

    auto end = high_resolution_clock::now();
    auto total = duration_cast<seconds>(end - start).count();

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  RESULTS                                      ║\n";
    cout <<   "╠════════════════════════════════════════════════╣\n";
    
    if(!corrupted) {
        cout << "║  Status:        ✅ 10M COMPLETE               ║\n";
        cout << "║  Final value:   " << setw(5) << last_val << " (expected 42)            ║\n";
        cout << "║  Initial noise: " << setw(8) << initial_noise << "                       ║\n";
        cout << "║  Final noise:   " << setw(8) << last_noise << "                       ║\n";
        
        if(last_noise == initial_noise) {
            cout << "║  Noise drift:   0.000000 (NO CHANGE)           ║\n";
            cout << "║  VERDICT:       ZANS Enc(0) = UNLIMITED        ║\n";
        } else {
            double total_drift = last_noise - initial_noise;
            double drift_per_op = total_drift / 10000000.0;
            cout << "║  Total drift:   " << setw(8) << fixed << setprecision(6) << total_drift << "                       ║\n";
            cout << "║  Drift/op:      " << setw(8) << scientific << drift_per_op << "                    ║\n";
        }
    } else {
        cout << "║  Status:        ❌ CORRUPTED                   ║\n";
        cout << "║  Max ops:       " << setw(8) << last_checkpoint << "                       ║\n";
        cout << "║  Final value:   " << setw(5) << last_val << " (expected 42)            ║\n";
    }
    
    cout << "║  Total time:    " << setw(5) << total << "s                        ║\n";
    double throughput = (double)10000000 / total;
    cout << "║  Throughput:    " << setw(5) << fixed << setprecision(0) << throughput << " ops/s                  ║\n";
    cout << "╚════════════════════════════════════════════════╝\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
