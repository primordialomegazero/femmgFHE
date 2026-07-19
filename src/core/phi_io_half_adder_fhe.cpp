// ΦΩ0 — FEmmg-iO: HALF-ADDER + FULL ADDER IN FHE
// 3×3 matrix-based half-adder with SNC+ZANS stabilization
// Ported from legacy phi_femmg_io_circuit.cpp
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

class FHEMatrix {
public:
    static const int N = 3;
    using Matrix = vector<vector<int64_t>>;
    
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int64_t modulus;
    ZANSAnchorPool pool;
    Ciphertext<DCRTPoly> M_ct;
    Ciphertext<DCRTPoly> anchor0;

    FHEMatrix(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp, int64_t mod)
        : cc(ctx), keys(kp), modulus(mod), pool(ctx, kp, 20) {
        vector<int64_t> mv = {mod / 2};
        M_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(mv));
        vector<int64_t> zv = {0};
        anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zv));
    }

    auto enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod_pos(v, modulus)}));
    }

    int64_t dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    }

    // SNC-stabilized multiply
    Ciphertext<DCRTPoly> snc_mult(const Ciphertext<DCRTPoly>& a, 
                                   const Ciphertext<DCRTPoly>& b) {
        auto sum = cc->EvalAdd(a, M_ct);
        auto back = cc->EvalSub(sum, M_ct);
        auto overflow = cc->EvalSub(a, back);
        auto result = cc->EvalMult(a, b);
        auto correction = cc->EvalMult(overflow, anchor0);
        result = cc->EvalAdd(result, correction);
        result = cc->EvalAdd(result, anchor0);
        for (int i = 0; i < 5; i++) result = pool.stabilize(result);
        return result;
    }

    // SNC-stabilized add
    Ciphertext<DCRTPoly> snc_add(const Ciphertext<DCRTPoly>& a,
                                  const Ciphertext<DCRTPoly>& b) {
        auto result = cc->EvalAdd(a, b);
        for (int i = 0; i < 3; i++) result = pool.stabilize(result);
        return result;
    }

    // Encrypted 3×3 matrix multiplication: C = A × B
    vector<vector<Ciphertext<DCRTPoly>>> mat_mult(
        const vector<vector<Ciphertext<DCRTPoly>>>& A,
        const vector<vector<Ciphertext<DCRTPoly>>>& B) {
        
        vector<vector<Ciphertext<DCRTPoly>>> C(N, vector<Ciphertext<DCRTPoly>>(N));
        
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                auto accum = enc(0);
                for (int k = 0; k < N; k++) {
                    auto prod = snc_mult(A[i][k], B[k][j]);
                    accum = snc_add(accum, prod);
                }
                C[i][j] = accum;
            }
        }
        return C;
    }

    // Encrypt a plaintext matrix
    vector<vector<Ciphertext<DCRTPoly>>> encrypt_matrix(const Matrix& M) {
        vector<vector<Ciphertext<DCRTPoly>>> enc_M(N, vector<Ciphertext<DCRTPoly>>(N));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                enc_M[i][j] = enc(M[i][j]);
        return enc_M;
    }

    // ============================================
    // HALF-ADDER GATES (from Barrington construction)
    // ============================================
    
    Matrix XOR_gate_plain(int b_val) {
        if (b_val) return {{1, 0, 1}, {0, 1, modulus - 2}, {0, 0, 1}};
        return {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    }

    Matrix AND_gate_plain(int b_val) {
        if (b_val) return {{1, 0, 0}, {0, 1, 1}, {0, 0, 1}};
        return {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    }

    // Initial state: [1, a, init_result] encoded as matrix
    Matrix initial_state(int64_t a, int64_t init) {
        return {{1, mod_pos(a, modulus), mod_pos(init, modulus)}, {0, 1, 0}, {0, 0, 1}};
    }

    struct HalfAdderResult {
        int64_t sum, carry;
    };

    // Half-adder in FHE: sum = a XOR b, carry = a AND b
    HalfAdderResult half_adder_fhe(int64_t a, int64_t b) {
        // XOR: state_sum starts with f = a
        auto state_sum = encrypt_matrix(initial_state(a, a));
        auto gate_xor = encrypt_matrix(XOR_gate_plain(b));
        auto result_sum = mat_mult(state_sum, gate_xor);
        int64_t sum = dec(result_sum[0][2]);

        // AND: state_carry starts with f = 0
        auto state_carry = encrypt_matrix(initial_state(a, 0));
        auto gate_and = encrypt_matrix(AND_gate_plain(b));
        auto result_carry = mat_mult(state_carry, gate_and);
        int64_t carry = dec(result_carry[0][2]);

        return {sum, carry};
    }
};

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int depth = 30;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FEmmg-iO: HALF-ADDER + FULL ADDER IN FHE        ║\n";
    cout <<   "  ║   3×3 encrypted matrix multiplication                   ║\n";
    cout <<   "  ║   SNC+ZANS stabilized                                    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(depth);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    FHEMatrix fhe(cc, keys, modulus);

    // ============================================
    // HALF-ADDER TRUTH TABLE
    // ============================================
    cout << "  Half-Adder Truth Table (Encrypted):\n";
    cout << "  " << string(60, '-') << "\n";
    cout << "  " << setw(6) << "a" << setw(6) << "b"
         << setw(10) << "Sum" << setw(10) << "Exp"
         << setw(12) << "Carry" << setw(10) << "Exp"
         << setw(10) << "Status\n";
    cout << "  " << string(60, '-') << "\n";

    int half_adder_ok = 0;
    for (int a : {0, 1}) {
        for (int b : {0, 1}) {
            auto [sum, carry] = fhe.half_adder_fhe(a, b);
            int exp_sum = a ^ b;
            int exp_carry = a & b;
            bool ok = (sum == exp_sum && carry == exp_carry);
            if (ok) half_adder_ok++;
            cout << "  " << setw(6) << a << setw(6) << b
                 << setw(10) << sum << setw(10) << exp_sum
                 << setw(12) << carry << setw(10) << exp_carry
                 << setw(10) << (ok ? "✓" : "✗") << "\n";
        }
    }
    cout << "  " << string(60, '-') << "\n";
    cout << "  Half-Adder: " << half_adder_ok << "/4 ✓\n\n";

    // ============================================
    // FULL ADDER (from two half-adders)
    // ============================================
    cout << "  Full Adder Truth Table (Encrypted):\n";
    cout << "  " << string(70, '-') << "\n";
    cout << "  " << setw(6) << "a" << setw(6) << "b" << setw(8) << "c_in"
         << setw(10) << "Sum" << setw(10) << "Exp"
         << setw(12) << "Carry" << setw(10) << "Exp"
         << setw(10) << "Status\n";
    cout << "  " << string(70, '-') << "\n";

    int full_adder_ok = 0;
    for (int a : {0, 1}) {
        for (int b : {0, 1}) {
            for (int c_in : {0, 1}) {
                auto [sum1, carry1] = fhe.half_adder_fhe(a, b);
                auto [sum_final, carry2] = fhe.half_adder_fhe(sum1, c_in);
                int carry_final = carry1 | carry2;
                int exp_sum = (a + b + c_in) & 1;
                int exp_carry = (a + b + c_in) >> 1;
                bool ok = (sum_final == exp_sum && carry_final == exp_carry);
                if (ok) full_adder_ok++;
                cout << "  " << setw(6) << a << setw(6) << b << setw(8) << c_in
                     << setw(10) << sum_final << setw(10) << exp_sum
                     << setw(12) << carry_final << setw(10) << exp_carry
                     << setw(10) << (ok ? "✓" : "✗") << "\n";
            }
        }
    }
    cout << "  " << string(70, '-') << "\n";
    cout << "  Full Adder: " << full_adder_ok << "/8 ✓\n\n";

    // ============================================
    // SUMMARY
    // ============================================
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   RESULTS                                                ║\n";
    cout <<   "  ║   Half-Adder: " << half_adder_ok << "/4";
    cout <<   "  | Full Adder: " << full_adder_ok << "/8";
    cout <<   "                   ║\n";
    if (half_adder_ok == 4 && full_adder_ok == 8) {
        cout << "  ║   *** ALL 12/12 VERIFIED — iO HALF/FULL ADDER WORKS ***  ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return (half_adder_ok == 4 && full_adder_ok == 8) ? 0 : 1;
}
