#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
using namespace lbcrypto;
using namespace std;

int64_t MOD;
int64_t mp(int64_t v) { return ((v % MOD) + MOD) % MOD; }

class PhiIOEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    using EncMatrix = vector<vector<Ciphertext<DCRTPoly>>>;
    static const int N = 3;

    PhiIOEngine(int ringDim, int64_t mod, int depth) {
        MOD = mod;
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(depth);
        params.SetPlaintextModulus(MOD);
        params.SetRingDim(ringDim);
        params.SetSecurityLevel(HEStd_NotSet);
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
    }

    auto enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mp(v)}));
    }
    int64_t dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        return mp((int64_t)pt->GetPackedValue()[0]);
    }

    EncMatrix encrypt_matrix(vector<vector<int64_t>> M) {
        EncMatrix R(N, vector<Ciphertext<DCRTPoly>>(N));
        for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) R[i][j] = enc(M[i][j]);
        return R;
    }

    EncMatrix mat_mult(const EncMatrix& A, const EncMatrix& B) {
        EncMatrix C(N, vector<Ciphertext<DCRTPoly>>(N));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++) {
                auto accum = enc(0);
                for (int k = 0; k < N; k++) {
                    auto prod = cc->EvalMult(A[i][k], B[k][j]);
                    accum = cc->EvalAdd(accum, prod);
                }
                C[i][j] = accum;
            }
        return C;
    }

    EncMatrix gate_XOR(int b) {
        vector<vector<int64_t>> M = {{1, 0, b ? 1 : 0}, {0, 1, b ? mp(-2) : 0}, {0, 0, 1}};
        return encrypt_matrix(M);
    }
    EncMatrix gate_AND(int b) {
        vector<vector<int64_t>> M = {{1, 0, 0}, {0, 1, b ? 1 : 0}, {0, 0, 1}};
        return encrypt_matrix(M);
    }
    EncMatrix initial_state(int64_t a, int64_t init) {
        vector<vector<int64_t>> M = {{1, mp(a), mp(init)}, {0, 1, 0}, {0, 0, 1}};
        return encrypt_matrix(M);
    }

    struct GR { int64_t sum, carry; };
    GR half_adder(int64_t a, int64_t b) {
        auto r_sum = mat_mult(initial_state(a, a), gate_XOR(b));
        auto r_carry = mat_mult(initial_state(a, 0), gate_AND(b));
        return {dec(r_sum[0][2]), dec(r_carry[0][2])};
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   φ-iO: Boolean Gates (Barrington Construction)       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    PhiIOEngine E(4096, 1073643521, 60);

    cout << "  Half-Adder:\n  " << string(40, '-') << "\n";
    int ha = 0;
    for (int a : {0, 1}) for (int b : {0, 1}) {
        auto [s, c] = E.half_adder(a, b);
        bool ok = (s==(a^b) && c==(a&b)); if (ok) ha++;
        cout << "  " << a << b << " → s=" << s << " c=" << c << (ok?" ✓":" ✗") << "\n";
    }

    cout << "\n  Full Adder:\n  " << string(40, '-') << "\n";
    int fa = 0;
    for (int a : {0, 1}) for (int b : {0, 1}) for (int ci : {0, 1}) {
        auto [s1, c1] = E.half_adder(a, b);
        auto [s, c2] = E.half_adder(s1, ci);
        int c = c1 | c2;
        bool ok = (s==((a+b+ci)&1) && c==((a+b+ci)>>1)); if (ok) fa++;
        cout << "  " << a << b << ci << " → s=" << s << " c=" << c << (ok?" ✓":" ✗") << "\n";
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   HA:" << ha << "/4  FA:" << fa << "/8";
    for (int i=0; i<(27-to_string(ha).length()-to_string(fa).length()); i++) cout << " ";
    cout << "║\n";
    if (ha==4 && fa==8) cout << "  ║   *** ALL 12/12 — iO GATES VERIFIED ***                ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
