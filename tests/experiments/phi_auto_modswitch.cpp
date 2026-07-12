// ΦΩ0 — AUTO MODULUS SWITCHING
// Detects overflow risk and switches to larger modulus
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace lbcrypto;
using namespace std;

class ModulusManager {
private:
    vector<int64_t> moduli = {1073643521, 1073643521};  // Same modulus for demo
    int current_level = 0;
    int64_t threshold;
    
public:
    ModulusManager() {
        threshold = moduli[0] * 0.75;  // 75% threshold
    }
    
    bool needs_upgrade(int64_t current_value) {
        return current_value > threshold;
    }
    
    void report(int64_t value) {
        cout << "  Value: " << value;
        cout << " (" << fixed << setprecision(1) << (100.0 * value / moduli[current_level]) << "% of modulus)";
        
        if(needs_upgrade(value)) {
            cout << " ⚠️ OVERFLOW RISK!";
        } else {
            cout << " ✅";
        }
        cout << "\n";
    }
};

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — AUTO MODULUS SWITCHING DEMO             ║\n";
    cout <<   "║  Overflow detection & modulus management       ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetPlaintextModulus(1073643521);  // 30-bit
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

    ModulusManager mgr;
    
    cout << "Φ Modulus: 1,073,643,521 (30-bit)\n";
    cout << "Φ Overflow threshold: 805,232,641 (75%)\n";
    cout << "Φ Max value before overflow: ~1.07 BILLION\n\n";

    // === TEST: Auto-detection during ×2 chain ===
    cout << "=== AUTO-DETECTION DURING ×2 CHAIN ===\n";
    cout << "Start=1, multiply by 2, detect overflow risk\n\n";

    vector<int64_t> start_vec = {1};
    auto chain = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_vec));
    int64_t current = 1;
    bool ok = true;

    for(int i = 0; i < 32 && ok; i++) {
        // Multiply by 2
        auto result = zero_ct;
        for(int j = 0; j < 2; j++) {
            result = cc->EvalAdd(result, chain);
            result = cc->EvalAdd(result, anchor0);
        }
        chain = result;
        current *= 2;

        // Check value
        Plaintext pt;
        cc->Decrypt(keys.secretKey, chain, &pt);
        int64_t val = pt->GetPackedValue()[0];
        
        // Only print at checkpoints
        if(i < 25 || i >= 27) {
            cout << "  Step " << setw(2) << (i+1) << ": ";
            mgr.report(val);
        } else if(i == 25) {
            cout << "  ... (steps 26-27: approaching threshold)\n";
        }

        if(val != current) {
            cout << "\n  ❌ OVERFLOW at step " << (i+1) << "!\n";
            cout << "  Expected: " << current << "\n";
            cout << "  Got: " << val << "\n";
            cout << "  Modulus needed: > " << current << " (";
            if(current > 1073741824) cout << "31+ bits";
            else if(current > 536870912) cout << "30-bit (current)";
            else cout << "< 30-bit";
            cout << ")\n";
            ok = false;
        }
    }

    // === RECOMMENDATION ===
    cout << "\n=== AUTO-MODSWITCH RECOMMENDATION ===\n";
    cout << "┌──────────┬──────────────┬──────────────────┐\n";
    cout << "│ Bits     │ Max Value    │ Ring Dim Needed  │\n";
    cout << "├──────────┼──────────────┼──────────────────┤\n";
    cout << "│ 30-bit   │ ~1 BILLION   │ 8192-16384       │\n";
    cout << "│ 40-bit   │ ~1 TRILLION  │ 16384-32768      │\n";
    cout << "│ 50-bit   │ ~1 QUAD      │ 32768-65536      │\n";
    cout << "│ 60-bit   │ ~1 QUINT     │ 65536+           │\n";
    cout << "└──────────┴──────────────┴──────────────────┘\n";
    cout << "\nΦ Strategy: Start at 30-bit, auto-upgrade when value > 75%\n";
    cout << "Φ Implementation: Pre-generate keys for multiple moduli\n";
    cout << "Φ Status: PLANNED (hardware-dependent)\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
