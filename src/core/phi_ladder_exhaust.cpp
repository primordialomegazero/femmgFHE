#include <openfhe.h>
#include <iostream>
using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int64_t minv(int64_t a, int64_t mod = 1073643521) {
    int64_t t = 0, nt = 1, r = mod, nr = a;
    while (nr) { int64_t q = r/nr; int64_t tmp=t; t=nt; nt=tmp-q*nt; tmp=r; r=nr; nr=tmp-q*nr; }
    return t < 0 ? t + mod : t;
}

int main() {
    int64_t modulus = 1073643521;
    int64_t phiF = 1618;

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    };

    int64_t F[100] = {0, 1};
    for (int i = 2; i < 100; i++) F[i] = (F[i-1] + F[i-2]) % modulus;

    auto phiPower = [&](int n) -> int64_t {
        if (n == 0) return 1;
        return mod_pos(F[n] * phiF + F[n-1] * 1000, modulus);
    };

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FIBONACCI LADDER: EXHAUSTIVE TEST                 ║\n";
    cout <<   "  ║   Push to the absolute limit                             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    int currentRung = 0;
    int64_t message = 1;  // Start with 1 to avoid overflow longer
    auto ct = enc(mod_pos(message * phiPower(0), modulus));
    auto two = enc(2);
    int64_t expected = message;
    int stepsInRung = 0;
    int maxStepsPerRung = 10;  // Push harder
    int totalSteps = 1000;      // GO BIG
    int totalRungs = 0;
    bool alive = true;
    int failsAt = -1;

    for (int step = 1; step <= totalSteps && alive; step++) {
        ct = cc->EvalMult(ct, two);
        expected = mod_pos(expected * 2, modulus);
        stepsInRung++;
        ct = cc->EvalAdd(ct, enc(0));

        if (stepsInRung >= maxStepsPerRung) {
            int64_t currentVal = mod_pos(dec(ct) * minv(phiPower(currentRung)), modulus);
            currentRung++;
            totalRungs++;
            int64_t newEncoded = mod_pos(currentVal * phiPower(currentRung), modulus);
            ct = enc(newEncoded);
            stepsInRung = 0;
        }

        if (step % 100 == 0 || step == totalSteps) {
            int64_t val = mod_pos(dec(ct) * minv(phiPower(currentRung)), modulus);
            cout << "Step " << setw(3) << step << " rung=" << setw(2) << currentRung
                 << ": " << setw(12) << val;
            if (val == expected) cout << " ✓";
            else { cout << " ✗ FAIL"; alive = false; failsAt = step; }
            cout << "\n";
        }
        
        if (step == 500) {
            cout << "... 100 steps reached ...\n";
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   RESULTS                                                  ║\n";
    cout <<   "  ║   Total steps: " << setw(5) << (failsAt > 0 ? failsAt : totalSteps);
    for (int i = 0; i < 18; i++) cout << " ";
    cout << "║\n";
    cout <<   "  ║   Total rungs: " << setw(5) << totalRungs;
    for (int i = 0; i < 18; i++) cout << " ";
    cout << "║\n";
    if (alive) {
        cout << "  ║   *** " << totalSteps << " STEPS — FIBONACCI LADDER WORKS! ***";
        for (int i = 0; i < (16 - to_string(totalSteps).length()); i++) cout << " ";
        cout << "║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return 0;
}
