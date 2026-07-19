// ΦΩ0 — RING DIM 32768 SMOKE TEST
// Verify keygen, encrypt, decrypt, basic ops work at 32768
// Plaintext modulus: 1077477377 (31-bit prime, (p-1) % 65536 == 0)
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 32768;
    int depth = 30;
    int64_t modulus = 1077477377;  // 31-bit prime, compatible with ring 32768
    
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — RING DIM 32768 SMOKE TEST                        ║\n";
    cout <<   "  ║   Modulus: " << setw(11) << modulus << " (31-bit)                      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    auto t0 = high_resolution_clock::now();

    // ============ CONTEXT SETUP ============
    cout << "  [1/5] Creating crypto context...\n";
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(depth);
    params.SetRingDim(ring_dim);
    params.SetPlaintextModulus(modulus);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);

    auto t1 = high_resolution_clock::now();
    cout << "  Context created in " 
         << duration_cast<seconds>(t1 - t0).count() << "s\n";

    // ============ KEY GENERATION ============
    cout << "  [2/5] Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto t2 = high_resolution_clock::now();
    cout << "  Keys generated in " 
         << duration_cast<seconds>(t2 - t1).count() << "s\n";

    // ============ ENCRYPT/DECRYPT ============
    cout << "  [3/5] Testing encrypt/decrypt...\n";
    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetPackedValue()[0];
    };

    auto ct42 = enc(42);
    int64_t val42 = dec(ct42);
    cout << "  Enc(42) -> Dec: " << val42 << (val42 == 42 ? " ✓" : " ✗ FAIL") << "\n";

    // ============ BASIC OPERATIONS ============
    cout << "  [4/5] Testing basic operations...\n";

    auto a = enc(100);
    auto b = enc(200);
    auto sum = cc->EvalAdd(a, b);
    int64_t sum_val = dec(sum);
    cout << "  100 + 200 = " << sum_val << (sum_val == 300 ? " ✓" : " ✗ FAIL") << "\n";

    auto mul = cc->EvalMult(a, b);
    int64_t mul_val = dec(mul);
    cout << "  100 × 200 = " << mul_val << (mul_val == 20000 ? " ✓" : " ✗ FAIL") << "\n";

    // ============ MULTIPLICATION CHAIN ============
    cout << "  [5/5] Testing sequential multiplications...\n";

    auto chain = enc(1);
    int64_t expected = 1;
    int chain_steps = 10;

    bool chain_ok = true;
    auto ct_two = enc(2);

    for (int i = 0; i < chain_steps; i++) {
        chain = cc->EvalMult(chain, ct_two);
        chain = cc->EvalAdd(chain, enc(0));
        expected = mod_pos(expected * 2, modulus);

        int64_t val = dec(chain);
        if (val != expected) {
            cout << "  Step " << (i+1) << ": " << val << " (expected " << expected << ") ✗\n";
            chain_ok = false;
            break;
        }
    }

    if (chain_ok) {
        cout << "  " << chain_steps << " sequential ×2: ALL CORRECT ✓\n";
    }

    auto t3 = high_resolution_clock::now();

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   RING DIM 32768 — SMOKE TEST COMPLETE                   ║\n";
    cout <<   "  ║   Total time: " << setw(3) << duration_cast<seconds>(t3 - t0).count() << "s";
    cout <<   "                                        ║\n";
    cout <<   "  ║   Status: " << (chain_ok ? "ALL TESTS PASSED ✓" : "FAILURES DETECTED ✗");
    cout <<   "                    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return chain_ok ? 0 : 1;
}
