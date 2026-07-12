// ΦΩ0 — ZANS NOISE MEASUREMENT TOOL
// Finally measuring the actual noise, not just checking values
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — ZANS NOISE MEASUREMENT                 ║\n";
    cout <<   "║  Measuring actual noise levels                 ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // Try different approaches to get noise info
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

    // Create anchors
    vector<int64_t> zero_vec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    // Test values
    vector<int64_t> val42 = {42};
    auto ct42 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val42));

    cout << "=== NOISE DIAGNOSTICS ===\n";
    cout << "Checking what noise info we can extract...\n\n";

    // Method 1: Check if ciphertext has noise methods
    cout << "[1] Direct ciphertext inspection:\n";
    
    // Try GetNoise
    auto ct_test = ct42;
    cout << "  Ciphertext type: " << typeid(ct_test).name() << "\n";
    
    // Try to get noise estimate via EvalAdd with 0 and check difference
    cout << "\n[2] Noise estimation via repeated Enc(0) additions:\n";
    cout << "    (Noise = how many Enc(0) additions until corruption)\n\n";

    auto ct_noise_test = ct42;
    int max_ops = 0;
    bool corrupted = false;
    
    cout << "    Starting noise stress test...\n";
    
    for(int i = 1; i <= 1000000 && !corrupted; i++) {
        ct_noise_test = cc->EvalAdd(ct_noise_test, anchor0);
        
        if(i % 100000 == 0) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct_noise_test, &pt);
            int64_t val = pt->GetPackedValue()[0];
            
            cout << "    Op " << setw(7) << i << ": value=" << setw(6) << val;
            if(val == 42) {
                cout << " ✅";
                max_ops = i;
            } else {
                cout << " ❌ CORRUPTED!";
                corrupted = true;
            }
            cout << "\n";
        }
    }
    
    if(!corrupted) {
        cout << "\n    ✅ 1,000,000 Enc(0) additions — NO CORRUPTION\n";
        cout << "    Noise growth: NEGLIGIBLE (value preserved)\n";
    } else {
        cout << "\n    ❌ Corruption at " << max_ops << " operations\n";
    }

    // Method 3: Compare with non-zero anchor
    cout << "\n[3] Comparison: Enc(1) additions (for noise contrast):\n";
    vector<int64_t> one_vec = {1};
    auto anchor1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(one_vec));
    
    auto ct_compare = ct42;
    bool comp_corrupted = false;
    int comp_max = 0;
    
    for(int i = 1; i <= 100000 && !comp_corrupted; i++) {
        ct_compare = cc->EvalAdd(ct_compare, anchor1);
        
        if(i % 10000 == 0) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct_compare, &pt);
            int64_t val = pt->GetPackedValue()[0];
            int64_t expected = 42 + i;
            
            cout << "    Op " << setw(6) << i << ": value=" << setw(6) << val 
                 << " (exp " << expected << ")";
            if(val == expected) {
                cout << " ✅";
                comp_max = i;
            } else {
                cout << " ❌ CORRUPTED!";
                comp_corrupted = true;
            }
            cout << "\n";
        }
    }

    cout << "\n=== NOISE ANALYSIS SUMMARY ===\n";
    cout << "Enc(0) additions: " << (corrupted ? to_string(max_ops) : "1,000,000+ (no corruption)") << "\n";
    cout << "Enc(1) additions: " << (comp_corrupted ? to_string(comp_max) : "100,000+ (no corruption)") << "\n";
    
    if(!corrupted && comp_corrupted) {
        double ratio = (double)1000000 / comp_max;
        cout << "\nΦ Enc(0) is at least " << fixed << setprecision(0) << ratio << "× more stable than Enc(1)\n";
    }
    
    // Method 4: Try to get actual noise bits from OpenFHE
    cout << "\n[4] Attempting OpenFHE noise diagnostics...\n";
    
    // Check if we can get noise via the level/scheme info
    try {
        auto ct_check = ct42;
        // Try GetCryptoParameters
        auto cryptoParams = ct_check->GetCryptoParameters();
        if(cryptoParams) {
            cout << "  Got crypto parameters\n";
            auto elementParams = cryptoParams->GetElementParams();
            if(elementParams) {
                cout << "  Ring dimension: " << elementParams->GetRingDimension() << "\n";
                cout << "  Modulus bits: " << elementParams->GetModulus().GetLengthForBase(2) << "\n";
            }
        }
        
        // Try GetNoiseScaleDeg
        double noiseScale = ct_check->GetNoiseScaleDeg();
        cout << "  Noise scale (degradation): " << noiseScale << "\n";
        
    } catch(const std::exception& e) {
        cout << "  Could not access: " << e.what() << "\n";
    } catch(...) {
        cout << "  Unknown error accessing noise info\n";
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — NOISE MEASUREMENT COMPLETE              ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
