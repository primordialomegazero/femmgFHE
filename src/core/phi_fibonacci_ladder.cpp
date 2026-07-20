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

    // φ-power using Fibonacci: φ^n = F[n] × φ + F[n-1] (with F[-1]=1, F[0]=0)
    // So: φ^0 = 0×φ + 1 = 1
    //     φ^1 = 1×φ + 0 = φ
    //     φ^2 = 1×φ + 1 = φ+1
    //     φ^3 = 2×φ + 1 = 2φ+1
    int64_t F[25] = {0, 1};
    for (int i = 2; i < 25; i++) F[i] = (F[i-1] + F[i-2]) % modulus;

    auto phiPower = [&](int n) -> int64_t {
        if (n == 0) return 1;  // φ^0 = 1
        return mod_pos(F[n] * phiF + F[n-1] * 1000, modulus);
    };

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FIBONACCI LADDER φ-BOOTSTRAP v3                    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    // Verify φ-powers
    cout << "φ^0 = " << phiPower(0) << " (should be 1)\n";
    cout << "φ^1 = " << phiPower(1) << " (should be " << phiF << ")\n";
    cout << "φ^2 = " << phiPower(2) << " (should be " << mod_pos(phiF+1000, modulus) << ")\n\n";

    int currentRung = 0;
    int64_t message = 42;
    auto ct = enc(mod_pos(message * phiPower(0), modulus));  // Start at φ^0=1
    auto two = enc(2);
    int64_t expected = message;
    int stepsInRung = 0;
    int maxStepsPerRung = 8;
    bool alive = true;

    for (int step = 1; step <= 50 && alive; step++) {
        ct = cc->EvalMult(ct, two);
        expected = mod_pos(expected * 2, modulus);
        stepsInRung++;
        ct = cc->EvalAdd(ct, enc(0));

        if (stepsInRung >= maxStepsPerRung) {
            // Decode at current rung
            int64_t currentVal = mod_pos(dec(ct) * minv(phiPower(currentRung)), modulus);
            // Climb
            currentRung++;
            int64_t newEncoded = mod_pos(currentVal * phiPower(currentRung), modulus);
            ct = enc(newEncoded);
            stepsInRung = 0;
        }

        if (step % 5 == 0 || step > 45) {
            int64_t val = mod_pos(dec(ct) * minv(phiPower(currentRung)), modulus);
            cout << "Step " << setw(2) << step << " rung=" << currentRung
                 << ": " << setw(10) << val;
            if (val == expected) cout << " ✓";
            else { cout << " ✗ (exp " << expected << ")"; alive = false; }
            cout << "\n";
        }
    }

    cout << "\nRungs climbed: " << currentRung << "\n";
    return 0;
}
