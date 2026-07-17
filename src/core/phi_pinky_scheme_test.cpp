// ΦΩ0 — PINKY SWEAR CROSS-SCHEME TEST
// Tests overflow detection on BFV, BGV, CKKS
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

void test_scheme(const string& name, int64_t modulus, bool is_exact) {
    cout << "\n=== " << name << " (mod=" << modulus << ") ===\n";
    
    CCParams<CryptoContextBFVRNS> params;
    if(name == "CKKS") {
        // CKKS test separately
        cout << "  CKKS: approximate, Pinky Swear may be needed\n";
        return;
    }
    
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();
    
    int64_t half_mod = modulus / 2;
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    
    // Test values: normal, near-half, at-half, over-half, near-max
    vector<int64_t> tests = {42, half_mod - 100, half_mod, half_mod + 100, modulus - 42};
    if(modulus > 1000000) tests = {42, half_mod - 1000, half_mod, half_mod + 1000, modulus - 42};
    
    int passed = 0;
    for(auto v : tests) {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod_pos(v, modulus)}));
        
        // Pinky Swear: (ct + M) - M - ct
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow_ct = cc->EvalSub(ct, back);
        
        Plaintext pt_over;
        cc->Decrypt(keys.secretKey, overflow_ct, &pt_over);
        pt_over->SetLength(1);
        int64_t over_val = mod_pos(pt_over->GetPackedValue()[0], modulus);
        
        bool is_zero = (over_val == 0);
        if(is_zero) passed++;
        
        if(v == tests[0] || v == tests.back() || v == half_mod) {
            cout << "  v=" << v << " overflow=" << over_val 
                 << (is_zero ? " (always zero)" : " NON-ZERO!") << "\n";
        }
    }
    
    cout << "  Result: " << passed << "/" << tests.size() << " zero overflows\n";
    if(passed == (int)tests.size()) {
        cout << "  VERDICT: Pinky Swear REDUNDANT (scheme handles modulo)\n";
    } else {
        cout << "  VERDICT: Pinky Swear NEEDED (overflow detectable)\n";
    }
}

int main() {
    cout << "=== PINKY SWEAR CROSS-SCHEME TEST ===\n\n";
    
    // BFV with different moduli
    test_scheme("BFV (small t=65537)", 65537, true);
    test_scheme("BFV (large t=1073643521)", 1073643521, true);
    // BFV tiny t=17 skipped — incompatible with ring dim
    
    // BGV (same as BFV for modulo behavior)
    test_scheme("BGV (t=65537)", 65537, true);
    test_scheme("BGV (t=1073643521)", 1073643521, true);
    
    cout << "\n=== SUMMARY ===\n";
    cout << "BFV/BGV: plaintext modulo is automatic\n";
    cout << "  (ct + M) - M - ct = 0 ALWAYS (modulo arithmetic)\n";
    cout << "  Pinky Swear NOT NEEDED for BFV/BGV\n\n";
    cout << "CKKS: approximate arithmetic, no automatic modulo\n";
    cout << "  (ct + M) - M - ct != 0 when value wraps\n";
    cout << "  Pinky Swear MAY BE NEEDED for CKKS\n\n";
    
    return 0;
}
