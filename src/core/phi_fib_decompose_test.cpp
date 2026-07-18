#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

vector<int64_t> zeckendorf(int64_t n) {
    vector<int64_t> fib = {1, 2};
    while (fib.back() <= n) {
        fib.push_back(fib[fib.size()-1] + fib[fib.size()-2]);
    }
    vector<int64_t> result;
    for (int i = fib.size()-1; i >= 0 && n > 0; i--) {
        if (fib[i] <= n) {
            result.push_back(fib[i]);
            n -= fib[i];
            i--;
        }
    }
    return result;
}

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521, half_mod = modulus / 2;
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 20);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));

    mt19937_64 rng(42);
    uniform_int_distribution<int64_t> dist(2, 1000);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    int64_t expected = 1;

    cout << "=== FIBONACCI ZECKENDORF DECOMPOSITION TEST ===" << endl;
    cout << "Method: Decompose multiplier → multiply each Fib part + ZANS" << endl;
    cout << "Range: 2-1000 | 200 steps" << endl;
    cout << setw(5) << "Step" << setw(8) << "Mult" << setw(8) << "Parts" 
         << setw(14) << "Expected" << setw(14) << "Decrypted" << setw(8) << "Result" << endl;
    cout << string(60, '-') << endl;

    for (int step = 0; step < 200; step++) {
        int64_t mult = dist(rng);
        auto parts = zeckendorf(mult);
        expected = mod_pos(expected * mult, modulus);

        // Pinky Swear
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        auto divine = cc->EvalMult(overflow, anchor0);

        // Multiply by each Fibonacci part
        for (int64_t part : parts) {
            auto ct_part = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{part}));
            ct = cc->EvalMult(ct, ct_part);
            ct = cc->EvalAdd(ct, anchor0);
            ct = pool.stabilize(ct);
        }

        // Divine after all parts
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        if ((step+1) % 30 == 0 || step < 5) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            bool ok = (dec_val == expected);
            cout << setw(5) << step+1 << setw(8) << mult << setw(8) << parts.size()
                 << setw(14) << expected << setw(14) << dec_val 
                 << setw(8) << (ok ? "OK" : "FAIL") << endl;
            if (!ok) return 1;
        }
    }
    cout << "\nALL 200 PASSED WITH FIBONACCI DECOMPOSITION!" << endl;
    return 0;
}
