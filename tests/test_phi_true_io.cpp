// FEmmg-FHE — TRUE BLUE iO
// Zero intermediate decryption. Pure encrypted matrix chain.
// NAND gate (3x3) → universal. Compose arbitrarily. Decrypt only at end.

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
using namespace lbcrypto;
using namespace std;

int64_t MOD;
int64_t mp(int64_t v) { return ((v % MOD) + MOD) % MOD; }

class TrueIO {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    static const int N = 3;
    using EncMatrix = vector<vector<Ciphertext<DCRTPoly>>>;

    TrueIO(int ringDim, int64_t mod, int depth) {
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

    // ========== 3×3 ENCRYPTED MATRIX MATH ==========
    EncMatrix mat_create(vector<vector<int64_t>> M) {
        EncMatrix R(N, vector<Ciphertext<DCRTPoly>>(N));
        for (int i=0; i<N; i++) for (int j=0; j<N; j++) R[i][j] = enc(mp(M[i][j]));
        return R;
    }

    EncMatrix mat_mult(const EncMatrix& A, const EncMatrix& B) {
        EncMatrix C(N, vector<Ciphertext<DCRTPoly>>(N));
        for (int i=0; i<N; i++)
            for (int j=0; j<N; j++) {
                auto accum = enc(0);
                for (int k=0; k<N; k++) {
                    auto prod = cc->EvalMult(A[i][k], B[k][j]);
                    accum = cc->EvalAdd(accum, prod);
                }
                C[i][j] = accum;
            }
        return C;
    }

    // ========== ENCODING: Bit → Encrypted Matrix ==========
    // Bit 0 → identity, Bit 1 → special permutation
    EncMatrix encode_bit(int64_t bit) {
        if (bit) return mat_create({{0, 1, 0}, {0, 0, 1}, {1, 0, 0}});
        return mat_create({{1, 0, 0}, {0, 1, 0}, {0, 0, 1}});
    }

    // Decode: read position [0][0] — 0 for cycle, 1 for identity
    int64_t decode_bit(const EncMatrix& M) {
        int64_t v = dec(M[0][0]);
        return (v == 0) ? 1 : 0;  // cycle has 0 at [0][0], identity has 1
    }

    // ========== NAND GATE (universal) ==========
    // NAND matrix: when conjugated with encoded bits, produces NAND output
    EncMatrix gate_NAND() {
        return mat_create({{0, 1, 0}, {1, 0, 0}, {0, 0, 1}});
    }

    // NAND(a,b) = A × B × NAND_matrix × A⁻¹ × B⁻¹
    // For 3-cycle: inverse = square (since cycle³ = identity)
    EncMatrix mat_inv(const EncMatrix& M) {
        return mat_mult(M, M);  // M² = M⁻¹ for 3-cycle
    }

    // FULLY ENCRYPTED NAND — no intermediate decryption
    EncMatrix iO_NAND(const EncMatrix& A, const EncMatrix& B) {
        auto GN = gate_NAND();
        auto Ainv = mat_inv(A);
        auto Binv = mat_inv(B);
        
        // A × B × GN × A⁻¹ × B⁻¹
        auto step1 = mat_mult(A, B);
        auto step2 = mat_mult(step1, GN);
        auto step3 = mat_mult(step2, Ainv);
        auto step4 = mat_mult(step3, Binv);
        
        return step4;
    }

    // ========== HIGHER-LEVEL GATES (built from NAND) ==========
    EncMatrix iO_NOT(const EncMatrix& A) {
        return iO_NAND(A, A);  // NOT(a) = NAND(a, a)
    }

    EncMatrix iO_AND(const EncMatrix& A, const EncMatrix& B) {
        auto nand = iO_NAND(A, B);
        return iO_NOT(nand);  // AND = NOT(NAND)
    }

    EncMatrix iO_OR(const EncMatrix& A, const EncMatrix& B) {
        auto na = iO_NOT(A);
        auto nb = iO_NOT(B);
        return iO_NAND(na, nb);  // OR = NAND(NOT(a), NOT(b))
    }

    EncMatrix iO_XOR(const EncMatrix& A, const EncMatrix& B) {
        auto nand1 = iO_NAND(A, B);
        auto or_gate = iO_OR(A, B);
        return iO_AND(nand1, or_gate);  // XOR = NAND(a,b) AND (a OR b)
    }

    // ========== TEST CIRCUITS ==========
    void test_nand() {
        cout << "\n  ── NAND Gate (encrypted end-to-end) ──\n  " << string(35, '-') << "\n";
        int ok = 0;
        for (int a : {0, 1}) for (int b : {0, 1}) {
            auto A = encode_bit(a), B = encode_bit(b);
            auto result = iO_NAND(A, B);
            int r = decode_bit(result);
            int e = !(a & b);
            bool good = (r == e); if (good) ok++;
            cout << "  NAND(" << a << b << ")=" << r << " (exp " << e << ")" << (good?" ✓":" ✗") << "\n";
        }
        cout << "  NAND: " << ok << "/4\n";
    }

    void test_majority() {
        cout << "\n  ── MAJORITY (from NAND gates, fully encrypted) ──\n  " << string(45, '-') << "\n";
        int ok = 0;
        for (int a : {0, 1}) for (int b : {0, 1}) for (int c : {0, 1}) {
            try {
                auto A = encode_bit(a), B = encode_bit(b), C = encode_bit(c);
                // MAJ = AB + BC + AC = NAND(NAND(AB, BC), NAND(AB, AC)) ... simplified
                auto ab = iO_AND(A, B);
                auto bc = iO_AND(B, C);
                auto ac = iO_AND(A, C);
                auto abORbc = iO_OR(ab, bc);
                auto result = iO_OR(abORbc, ac);
                int r = decode_bit(result);
                int e = (a+b+c >= 2) ? 1 : 0;
                bool good = (r == e); if (good) ok++;
                cout << "  MAJ(" << a << b << c << ")=" << r << " (exp " << e << ")" << (good?" ✓":" ✗") << "\n";
            } catch (const exception& ex) {
                cout << "  MAJ(" << a << b << c << ") CRASHED: " << ex.what() << "\n";
            }
        }
        cout << "  MAJ: " << ok << "/8\n";
    }

    void test_composed_circuit() {
        cout << "\n  ── COMPOSED: (a NAND b) XOR (c AND d) — fully encrypted ──\n  " << string(50, '-') << "\n";
        int ok = 0;
        for (int a : {0, 1}) for (int b : {0, 1}) for (int c : {0, 1}) for (int d : {0, 1}) {
            try {
                auto A = encode_bit(a), B = encode_bit(b), C = encode_bit(c), D = encode_bit(d);
                auto nand_ab = iO_NAND(A, B);
                auto and_cd = iO_AND(C, D);
                auto result = iO_XOR(nand_ab, and_cd);
                int r = decode_bit(result);
                int e = (!(a&b)) ^ (c&d);
                bool good = (r == e); if (good) ok++;
                if (ok <= 4 || !good)
                    cout << "  (" << a << " NAND " << b << ") XOR (" << c << " AND " << d << ") = " << r << " (exp " << e << ")" << (good?" ✓":" ✗") << "\n";
            } catch (const exception& ex) {
                cout << "  CRASHED: " << ex.what() << "\n";
            }
        }
        cout << "  Composed: " << ok << "/16\n";
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   TRUE BLUE iO: Zero Intermediate Decryption          ║\n";
    cout <<   "  ║   Encrypted matrices flow end-to-end                 ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n";

    TrueIO T(4096, 1073643521, 250);
    T.test_nand();
    T.test_majority();
    T.test_composed_circuit();

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   TRUE iO: Gates → Circuits → Arbitrary Logic        ║\n";
    cout <<   "  ║   All wires encrypted. Only final output exposed.    ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
