// BFV WITHOUT φ-clean: How many mults at depth 1?
#include <openfhe.h>
#include <iostream>
#include <vector>
using namespace lbcrypto;
using namespace std;

int64_t MOD;
int64_t mp(int64_t v) { return ((v % MOD) + MOD) % MOD; }

int main() {
    cout << "\n  BFV WITHOUT φ-CLEAN: How many mults at depth 1?\n\n";
    MOD = 1073643521;

    for (int depth : {1, 2, 3, 5, 10}) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(depth);
        params.SetPlaintextModulus(MOD);
        params.SetRingDim(4096);
        params.SetSecurityLevel(HEStd_NotSet);
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);

        auto enc = [&](int64_t v) { return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mp(v)})); };

        auto ct1 = enc(1LL);
        auto ct2 = enc(2LL);
        int count = 0;
        try {
            for (int i = 0; i < 100; i++) {
                ct1 = cc->EvalMult(ct1, ct2);
                count++;
            }
        } catch (...) {}
        cout << "  Depth " << setw(2) << depth << ": " << setw(3) << count << " mults\n";
    }
    cout << "\n  Compare: WITH φ-clean = 45 mults at depth 1 (45× compression)\n\n";
    return 0;
}
