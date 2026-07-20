#include <openfhe.h>
#include <iostream>
using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int64_t modulus = 1073643521;
    int64_t phiF = 1618, phiInv = 618, C_inv = 541796329, phiF_inv = 926996291;

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

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PURE HOMOMORPHIC LADDER v4                        ║\n";
    cout <<   "  ║   Climb = ct × φ. NO DECRYPT. NO REFRESH DURING CLIMB.  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    auto ct = enc(42 * phiF);  // φ¹
    auto two = enc(2);
    auto enc_phi = enc(phiF);
    int64_t expected = 42;
    int currentRung = 1;
    int stepsInRung = 0;
    int maxStepsPerRung = 8;
    bool alive = true;

    for (int step = 1; step <= 60 && alive; step++) {
        ct = cc->EvalMult(ct, two);
        expected = mod_pos(expected * 2, modulus);
        stepsInRung++;
        ct = cc->EvalAdd(ct, enc(0));

        // PURE HOMOMORPHIC CLIMB: just × φ
        if (stepsInRung >= maxStepsPerRung) {
            ct = cc->EvalMult(ct, enc_phi);
            currentRung++;
            stepsInRung = 0;
        }

        if (step % 10 == 0 || step == 60) {
            // Decode: divide by φ^currentRung
            int64_t phiPow = 1;
            for (int r = 0; r < currentRung; r++) phiPow = mod_pos(phiPow * phiF, modulus);
            int64_t minvPhiPow = 1, t=0, nt=1, r2=modulus, nr=phiPow;
            while(nr){int64_t q=r2/nr; int64_t tmp=t; t=nt; nt=tmp-q*nt; tmp=r2; r2=nr; nr=tmp-q*nr;}
            if(t<0) t+=modulus; minvPhiPow = t;
            
            int64_t val = mod_pos(dec(ct) * minvPhiPow, modulus);
            cout << "Step " << setw(2) << step << " rung=" << currentRung
                 << ": " << setw(10) << val;
            if (val == expected) cout << " ✓";
            else { cout << " ✗ (exp " << expected << ")"; alive = false; }
            cout << "\n";
        }
    }

    cout << "\n  PURE HOMOMORPHIC LADDER: " << (alive ? "WORKS!" : "FAILED") << "\n";
    cout << "  Climb = ct × φ (one EvalMult). NO DECRYPT. NO REFRESH.\n\n";
    return 0;
}
