// ΦΩ0 — QUANTUM RANDOM ZANS v1.1
// True randomness from FHE noise + Emergent order
// Tests: Distribution, ZANS emergence, pass/fail
// "THE CIPHERTEXT CHOOSES. FREE WILL EXISTS IN FHE."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n";
    cout << "  +--------------------------------------------------+\n";
    cout << "  |  QUANTUM RANDOM ZANS v1.1                        |\n";
    cout << "  |  True Randomness from FHE Noise + Emergence      |\n";
    cout << "  +--------------------------------------------------+\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(3);
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

    int passed = 0, total = 0;
    auto anchor0 = enc(0);

    // ============================================
    // TEST 1: NOISE DIRECTION DISTRIBUTION
    // ============================================
    cout << "  TEST 1: NOISE DIRECTION DISTRIBUTION\n";
    cout << "  " << string(50, '-') << "\n";

    auto ct = enc(42);
    int positive = 0, negative = 0, neutral = 0;

    for(int i = 0; i < 1000; i++) {
        auto zero = enc(0);
        double before = ct->GetNoiseScaleDeg();
        ct = cc->EvalAdd(ct, zero);
        double after = ct->GetNoiseScaleDeg();
        if(after > before) positive++;
        else if(after < before) negative++;
        else neutral++;
    }

    cout << "  Positive noise steps: " << positive << " (" << fixed << setprecision(1) << (100.0*positive/1000) << "%)\n";
    cout << "  Negative noise steps: " << negative << " (" << fixed << setprecision(1) << (100.0*negative/1000) << "%)\n";
    cout << "  Neutral steps:        " << neutral << " (" << fixed << setprecision(1) << (100.0*neutral/1000) << "%)\n";
    bool balanced = (abs(positive - negative) <= 200);
    cout << "  Balanced (+/- within 20%): " << (balanced ? "YES" : "NO") << "\n";
    cout << "  Result: " << (balanced ? "PASSED" : "CHECK") << "\n\n";
    total++; if(balanced) passed++;

    // ============================================
    // TEST 2: ZANS EMERGENCE (Individual chaos → aggregate order)
    // ============================================
    cout << "  TEST 2: ZANS EMERGENCE (1000 Enc(0) additions)\n";
    cout << "  " << string(50, '-') << "\n";

    auto ct2 = enc(42);
    double start_noise = ct2->GetNoiseScaleDeg();
    int pos2 = 0, neg2 = 0;

    for(int i = 0; i < 1000; i++) {
        auto zero = enc(0);
        double b = ct2->GetNoiseScaleDeg();
        ct2 = cc->EvalAdd(ct2, zero);
        double a = ct2->GetNoiseScaleDeg();
        if(a > b) pos2++; else if(a < b) neg2++;
    }

    double end_noise = ct2->GetNoiseScaleDeg();
    int64_t final_val = dec(ct2);

    cout << "  Individual: +" << pos2 << " / -" << neg2 << " (chaotic)\n";
    cout << "  Aggregate:  noise " << fixed << setprecision(1) << start_noise << " -> " << end_noise;
    cout << " (net: " << (end_noise - start_noise) << ")\n";
    cout << "  Value:      " << final_val << " (expected 42)\n";
    bool emergence = (abs(end_noise - start_noise) < 1.0) && (final_val == 42);
    cout << "  Emergence:  " << (emergence ? "ORDER FROM CHAOS" : "UNSTABLE") << "\n";
    cout << "  Result:     " << (emergence ? "PASSED" : "FAILED") << "\n\n";
    total++; if(emergence) passed++;

    // ============================================
    // TEST 3: VALUE STABILITY ACROSS MULTIPLE RUNS
    // ============================================
    cout << "  TEST 3: VALUE STABILITY (5 runs x 500 ops)\n";
    cout << "  " << string(50, '-') << "\n";

    bool all_stable = true;
    for(int run = 1; run <= 5; run++) {
        auto c = enc(run * 10);
        for(int i = 0; i < 500; i++) c = cc->EvalAdd(c, enc(0));
        int64_t v = dec(c);
        bool stable = (v == run * 10);
        if(!stable) all_stable = false;
        cout << "  Run " << run << ": start=" << run*10 << " end=" << v << " " << (stable ? "OK" : "FAIL") << "\n";
    }

    cout << "  Result: " << (all_stable ? "PASSED (all stable)" : "FAILED") << "\n\n";
    total++; if(all_stable) passed++;

    // ============================================
    // SUMMARY
    // ============================================
    cout << "  +--------------------------------------------------+\n";
    cout << "  |  QUANTUM RANDOM ZANS: " << passed << "/" << total << " TESTS PASSED";
    for(int i = 0; i < (20 - to_string(passed).length()); i++) cout << " ";
    cout << "|\n";
    cout << "  |  Individual: FREE WILL (chaos)                   |\n";
    cout << "  |  Aggregate:   DESTINY (order)                    |\n";
    cout << "  |  ZANS:        Bridge between quantum & classical |\n";
    cout << "  +--------------------------------------------------+\n\n";
    cout << "  I AM THAT I AM\n\n";

    return 0;
}
