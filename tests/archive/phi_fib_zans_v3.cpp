// ΦΩ0 — FIBONACCI-ZANS v3: ACTUAL MULTIPLICATION
// base × multiplier = repeated Enc(base) + Enc(0) stabilization
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace lbcrypto;
using namespace std;

vector<int64_t> generate_fibonacci(int64_t limit) {
    vector<int64_t> fib = {1, 2};
    while(fib.back() <= limit) {
        fib.push_back(fib[fib.size()-1] + fib[fib.size()-2]);
    }
    fib.pop_back();
    return fib;
}

vector<int64_t> zeckendorf_decompose(int64_t n) {
    vector<int64_t> fib = generate_fibonacci(n);
    vector<int64_t> result;
    for(int i = fib.size() - 1; i >= 0 && n > 0; i--) {
        if(fib[i] <= n) {
            result.push_back(fib[i]);
            n -= fib[i];
            i--;
        }
    }
    return result;
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — FIBONACCI-ZANS v3: ACTUAL MULTIPLY     ║\n";
    cout <<   "║  base × N = repeated Enc(base) + Enc(0)       ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(65537);
    params.SetSecurityLevel(HEStd_NotSet);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Anchors
    vector<int64_t> zero_vec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));
    auto zero_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    // === TEST: 5 × 7 ===
    cout << "=== TEST: 5 × 7 ===\n";
    vector<int64_t> b5 = {5};
    auto base5 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(b5));
    
    int64_t multiplier = 7;
    auto parts = zeckendorf_decompose(multiplier);
    cout << "Zeckendorf: " << multiplier << " = ";
    for(size_t i = 0; i < parts.size(); i++) {
        cout << parts[i];
        if(i < parts.size()-1) cout << "+";
    }
    cout << "\n";
    
    auto result = zero_ct;
    int total = 0;
    for(auto fp : parts) {
        for(int64_t i = 0; i < fp; i++) {
            result = cc->EvalAdd(result, base5);
            result = cc->EvalAdd(result, anchor0);
            total++;
        }
        Plaintext pt;
        cc->Decrypt(keys.secretKey, result, &pt);
        cout << "  After " << fp << "×base: " << pt->GetPackedValue()[0] << "\n";
    }
    
    Plaintext fpt;
    cc->Decrypt(keys.secretKey, result, &fpt);
    int64_t fv = fpt->GetPackedValue()[0];
    cout << "  5 × 7 = " << fv << " (expected 35)";
    if(fv == 35) cout << " ✅\n\n";
    else cout << " ❌\n\n";

    // === SWEEP ===
    cout << "=== MULTIPLIER SWEEP (base=3) ===\n";
    vector<int64_t> b3 = {3};
    auto base3 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(b3));
    vector<int64_t> mults = {2, 3, 5, 7, 10, 21, 42, 100, 1000};
    
    for(auto mult : mults) {
        auto p = zeckendorf_decompose(mult);
        auto res = zero_ct;
        int adds = 0;
        bool ok = true;
        
        for(auto fp : p) {
            for(int64_t i = 0; i < fp && ok; i++) {
                res = cc->EvalAdd(res, base3);
                res = cc->EvalAdd(res, anchor0);
                adds++;
                if(adds % 5000 == 0) {
                    Plaintext cpt;
                    cc->Decrypt(keys.secretKey, res, &cpt);
                    if(cpt->GetPackedValue()[0] < 0) ok = false;
                }
            }
        }
        
        Plaintext rpt;
        cc->Decrypt(keys.secretKey, res, &rpt);
        int64_t val = rpt->GetPackedValue()[0];
        int64_t exp = 3 * mult;
        
        cout << "  3 × " << setw(4) << mult << " = " << setw(6) << val;
        cout << " (exp " << setw(6) << exp << ")";
        if(val == exp) cout << " ✅";
        else if(!ok) cout << " ❌ CORRUPTED";
        else cout << " ❌";
        cout << " | " << adds << " adds | " << p.size() << " parts\n";
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — FIB-ZANS v3 COMPLETE                   ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
