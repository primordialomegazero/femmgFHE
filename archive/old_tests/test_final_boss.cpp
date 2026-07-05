// ============================================================
//  FINAL BOSS — Hunt the Last 5 Halimaw
//  Target: ALL GREEN, ZERO HOLES
// ============================================================
#include "../src/core/true_poly_fhe.h"
#include <iostream>
#include <iomanip>
#include <climits>
using namespace std;

void check(const string& name, bool ok, const string& detail) {
    cout << "  " << (ok ? "✅" : "❌") << " " << name;
    if (!detail.empty()) cout << " — " << detail;
    cout << endl;
}

int main() {
    TruePolyFHE fhe;
    uint64_t seed = 0xB055F1F1E;
    
    cout << "====================================================" << endl;
    cout << "  FINAL BOSS — 5 Halimaw Remaining" << endl;
    cout << "====================================================" << endl;

    // ═══ BOSS 1 & 2: INT64_MAX / INT64_MIN ═══
    cout << "\n--- BOSS 1: INT64_MAX ---" << endl;
    {
        // Current: clamps silently
        // Desired: Either support full range OR reject with error
        // Option A: Use __int128 for intermediate math
        // Option B: Reduce DELTA to allow full int64 range
        // Option C: Throw exception on overflow
        
        auto ct = fhe.encrypt(INT64_MAX, seed);
        int64_t dec = fhe.decrypt(ct, seed);
        cout << "  Current behavior: " << dec << " (should be " << INT64_MAX << ")" << endl;
        
        // Check if we can support full range by checking DELTA constraints
        int64_t max_safe = (INT64_MAX / TruePolyFHE::DELTA);
        cout << "  Max safe plaintext: " << max_safe << endl;
        cout << "  INT64_MAX: " << INT64_MAX << endl;
        cout << "  Gap: " << (INT64_MAX - max_safe) << endl;
    }

    cout << "\n--- BOSS 2: INT64_MIN ---" << endl;
    {
        auto ct = fhe.encrypt(INT64_MIN, seed);
        int64_t dec = fhe.decrypt(ct, seed);
        cout << "  Current behavior: " << dec << " (should be " << INT64_MIN << ")" << endl;
    }

    // ═══ BOSS 3 & 4: Tampering Detection ═══
    cout << "\n--- BOSS 3: Tampered coeffs[0] Detection ---" << endl;
    {
        auto ct = fhe.encrypt(42, seed);
        ct.coeffs[0] += 1000;  // Tamper
        int64_t dec = fhe.decrypt(ct, seed);
        
        // Current: still decrypts to 42 because we use msg_part
        // But coeffs are inconsistent — we can detect this!
        int64_t expected_coeff0 = ct.msg_part + ct.noise_part;
        bool tampered = (ct.coeffs[0] != expected_coeff0);
        cout << "  Decrypted: " << dec << " (tampered coeffs[0])" << endl;
        cout << "  Expected coeffs[0]: " << expected_coeff0 << ", Actual: " << ct.coeffs[0] << endl;
        cout << "  Tamper detected: " << (tampered ? "YES" : "NO (need integrity check)") << endl;
    }

    cout << "\n--- BOSS 4: Tampered nonce Detection ---" << endl;
    {
        auto ct = fhe.encrypt(42, seed);
        uint64_t orig_nonce = ct.nonce;
        ct.nonce = 0xDEADBEEF;  // Tamper
        int64_t dec = fhe.decrypt(ct, seed);
        cout << "  Decrypted: " << dec << " (tampered nonce)" << endl;
        cout << "  Original nonce: " << orig_nonce << ", Tampered: " << ct.nonce << endl;
        // Can we detect? If we hash the ciphertext and store MAC...
    }

    // ═══ BOSS 5: Ring Boundary Artifacts ═══
    cout << "\n--- BOSS 5: coeffs[63] Boundary ---" << endl;
    {
        auto ct = fhe.encrypt(1, seed);
        cout << "  Before tamper: coeffs[63]=" << ct.coeffs[63] << endl;
        ct.coeffs[63] = 999999;  // Inject garbage
        int64_t dec = fhe.decrypt(ct, seed);
        cout << "  After tamper: coeffs[63]=" << ct.coeffs[63] << endl;
        cout << "  Decrypted: " << dec << " (should be 1, but noise leaked in)" << endl;
        
        // Root cause: non-modular convolution means coeffs[63] shouldn't affect
        // coeffs[0], but it might via add/mul operations. Let's trace.
        auto ct2 = fhe.encrypt(1, seed);
        auto sum = fhe.add(ct, ct2);
        int64_t dec_sum = fhe.decrypt(sum, seed);
        cout << "  After add with clean ct: " << dec_sum << " (should be 2)" << endl;
    }

    cout << "\n====================================================" << endl;
    cout << "  BOSS ANALYSIS COMPLETE" << endl;
    cout << "====================================================" << endl;
    
    return 0;
}
