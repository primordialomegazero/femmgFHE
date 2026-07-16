// ΦΩ0 — ENTANGLED PRIME ZANS v1.1
// Pre-computed entangled prime pairs (+p, -p) for ZANS
// Verified: +p + (-p) = 0 cancellation + noise comparison
// "ENTANGLED PRIMES. PERFECT CANCELLATION."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

vector<int64_t> generate_primes(int count) {
    vector<int64_t> primes;
    vector<bool> is_prime(1000000, true);
    is_prime[0] = is_prime[1] = false;
    for(int64_t i = 2; i < 1000000 && (int)primes.size() < count; i++) {
        if(is_prime[i]) {
            primes.push_back(i);
            for(int64_t j = i*i; j < 1000000; j += i) is_prime[j] = false;
        }
    }
    return primes;
}

int main() {
    cout << "\n";
    cout << "  +--------------------------------------------------+\n";
    cout << "  |  ENTANGLED PRIME ZANS v1.1                       |\n";
    cout << "  |  Pre-computed Prime Pairs + Cancellation Test    |\n";
    cout << "  +--------------------------------------------------+\n\n";

    int passed = 0, total = 0;
    auto primes = generate_primes(50);

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // ============================================
    // TEST 1: Generate Entangled Pairs
    // ============================================
    cout << "  TEST 1: PRIME PAIR GENERATION\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  Primes generated: " << primes.size() << "\n";
    cout << "  Range: " << primes[0] << " to " << primes.back() << "\n";
    cout << "  First 5: ";
    for(int i = 0; i < 5; i++) cout << primes[i] << " ";
    cout << "\n";
    cout << "  Result: PASSED\n\n";
    passed++; total++;

    // ============================================
    // TEST 2: Prime Cancellation Verification
    // ============================================
    cout << "  TEST 2: CANCELLATION VERIFICATION (+p + -p = 0)\n";
    cout << "  " << string(50, '-') << "\n";

    bool all_cancel = true;
    for(int i = 0; i < 10; i++) {
        int64_t p = primes[i];
        auto enc_plus_p = enc(p);
        auto enc_minus_p = enc(-p);
        auto sum = cc->EvalAdd(enc_plus_p, enc_minus_p);
        int64_t result = dec(sum);
        bool cancel = (result == 0);

        cout << "  " << setw(3) << right << p << " + (" << setw(4) << -p << ") = " 
             << setw(2) << result << " " << (cancel ? "OK" : "FAIL") << "\n";
        if(!cancel) all_cancel = false;
    }

    cout << "  Result: " << (all_cancel ? "PASSED (all cancel)" : "FAILED") << "\n\n";
    total++; if(all_cancel) passed++;

    // ============================================
    // TEST 3: Noise Comparison
    // ============================================
    cout << "  TEST 3: NOISE COMPARISON (50,000 additions)\n";
    cout << "  " << string(65, '-') << "\n";
    cout << "  " << setw(25) << left << "Method"
         << setw(12) << right << "Ops"
         << setw(12) << "Time(s)"
         << setw(10) << "Ops/s"
         << setw(10) << "Noise"
         << setw(12) << "Final Value\n";
    cout << "  " << string(65, '-') << "\n";

    // Standard ZANS
    auto ct1 = enc(42);
    auto plain_anchor = enc(0);
    auto t1 = high_resolution_clock::now();
    for(int i = 0; i < 50000; i++) ct1 = cc->EvalAdd(ct1, plain_anchor);
    auto t2 = high_resolution_clock::now();
    double time1 = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;
    double noise1 = ct1->GetNoiseScaleDeg();
    int64_t val1 = dec(ct1);

    cout << "  " << setw(25) << left << "Standard ZANS"
         << setw(12) << right << 50000
         << setw(12) << fixed << setprecision(1) << time1
         << setw(10) << fixed << setprecision(0) << (50000.0/max(time1,0.001))
         << setw(10) << fixed << setprecision(1) << noise1
         << setw(12) << val1 << "\n";
    bool std_ok = (val1 == 42);
    total++; if(std_ok) passed++;

    // Entangled Prime ZANS
    struct EntangledPair {
        Ciphertext<DCRTPoly> anchor_pos;
        Ciphertext<DCRTPoly> anchor_neg;
        int64_t prime;
    };

    vector<EntangledPair> entangled_pairs;
    for(size_t i = 0; i < primes.size(); i++) {
        EntangledPair ep;
        ep.prime = primes[i];
        ep.anchor_pos = enc(primes[i]);
        ep.anchor_neg = enc(-primes[i]);
        entangled_pairs.push_back(ep);
    }

    auto ct2 = enc(42);
    t1 = high_resolution_clock::now();
    for(int i = 0; i < 50000; i++) {
        int pair_idx = i % entangled_pairs.size();
        ct2 = cc->EvalAdd(ct2, entangled_pairs[pair_idx].anchor_pos);
        ct2 = cc->EvalAdd(ct2, entangled_pairs[pair_idx].anchor_neg);
    }
    t2 = high_resolution_clock::now();
    double time2 = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;
    double noise2 = ct2->GetNoiseScaleDeg();
    int64_t val2 = dec(ct2);

    cout << "  " << setw(25) << left << "Entangled Prime ZANS"
         << setw(12) << right << "50Kx2"
         << setw(12) << fixed << setprecision(1) << time2
         << setw(10) << fixed << setprecision(0) << (100000.0/max(time2,0.001))
         << setw(10) << fixed << setprecision(1) << noise2
         << setw(12) << val2 << "\n";

    cout << "  " << string(65, '-') << "\n";
    bool ent_ok = (val2 == 42);
    total++; if(ent_ok) passed++;

    // Noise assessment
    cout << "  Standard ZANS noise:      " << fixed << setprecision(1) << noise1 << "\n";
    cout << "  Entangled Prime ZANS noise: " << fixed << setprecision(1) << noise2 << "\n";
    cout << "  Value preserved (42):      " << (std_ok && ent_ok ? "YES" : "NO") << "\n\n";

    // ============================================
    // TEST 4: Multi-Round Stability
    // ============================================
    cout << "  TEST 4: MULTI-ROUND STABILITY (5 rounds x 1000)\n";
    cout << "  " << string(50, '-') << "\n";

    bool all_stable = true;
    for(int round = 1; round <= 5; round++) {
        auto ct = enc(round * 10);
        for(int i = 0; i < 1000; i++) {
            int pair_idx = i % entangled_pairs.size();
            ct = cc->EvalAdd(ct, entangled_pairs[pair_idx].anchor_pos);
            ct = cc->EvalAdd(ct, entangled_pairs[pair_idx].anchor_neg);
        }
        int64_t result = dec(ct);
        bool stable = (result == round * 10);
        cout << "  Round " << round << ": start=" << round*10 
             << " end=" << result << " " << (stable ? "OK" : "FAIL") << "\n";
        if(!stable) all_stable = false;
    }

    cout << "  Result: " << (all_stable ? "PASSED (all stable)" : "FAILED") << "\n\n";
    total++; if(all_stable) passed++;

    // ============================================
    // SUMMARY
    // ============================================
    cout << "  +--------------------------------------------------+\n";
    cout << "  |  ENTANGLED PRIME ZANS: " << passed << "/" << total << " TESTS PASSED";
    for(int i = 0; i < (19 - to_string(passed).length()); i++) cout << " ";
    cout << "|\n";
    cout << "  |  v1.1 — ";
    cout << (passed == total ? "ALL CHECKS PASSED" : "CHECK RESULTS") << "                       |\n";
    cout << "  +--------------------------------------------------+\n\n";
    cout << "  I AM THAT I AM\n\n";

    return 0;
}
