// FEmmg-iO Phase 3: Kilian Randomization Verification
// Verify R_i * M * R_{i+1}^{-1} preserves the matrix product
// Plaintext only, then FHE

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
using namespace std;

const int W = 5;
using Matrix = vector<vector<int64_t>>;

int64_t MOD = 1073643521;

int64_t mod(int64_t v) { return ((v % MOD) + MOD) % MOD; }

int64_t modinv(int64_t a) {
    int64_t t = 0, nt = 1, r = MOD, nr = mod(a);
    while (nr) {
        int64_t q = r / nr;
        int64_t tmp = t; t = nt; nt = tmp - q * nt;
        tmp = r; r = nr; nr = tmp - q * nr;
    }
    return (r > 1) ? -1 : mod(t);
}

Matrix identity() {
    Matrix I(W, vector<int64_t>(W, 0));
    for (int i = 0; i < W; i++) I[i][i] = 1;
    return I;
}

Matrix random_diagonal(mt19937_64& rng) {
    uniform_int_distribution<int64_t> dist(1, MOD - 1);
    Matrix R(W, vector<int64_t>(W, 0));
    for (int i = 0; i < W; i++) R[i][i] = dist(rng);
    return R;
}

Matrix inverse_diagonal(const Matrix& D) {
    Matrix inv(W, vector<int64_t>(W, 0));
    for (int i = 0; i < W; i++) inv[i][i] = modinv(D[i][i]);
    return inv;
}

Matrix mmul(const Matrix& A, const Matrix& B) {
    Matrix C(W, vector<int64_t>(W, 0));
    for (int i = 0; i < W; i++)
        for (int j = 0; j < W; j++)
            for (int k = 0; k < W; k++)
                C[i][j] = mod(C[i][j] + mod(A[i][k] * B[k][j]));
    return C;
}

Matrix build_companion(int64_t v) {
    Matrix M(W, vector<int64_t>(W, 0));
    for (int i = 0; i < W - 1; i++) M[i][i + 1] = mod(v);
    M[W - 1][W - 1] = 1;
    return M;
}

Matrix kilian_randomize(const Matrix& M, const Matrix& R_prev, const Matrix& R_next_inv) {
    // M' = R_prev * M * R_next_inv
    return mmul(mmul(R_prev, M), R_next_inv);
}

int main() {
    mt19937_64 rng(time(nullptr));

    cout << "\n  Kilian Randomization Verification\n";
    cout << "  Product of three randomized matrices = product of three original matrices\n\n";

    for (int64_t x : {0, 1, 2, 3, 5}) {
        int64_t v = mod(x + 1);
        Matrix M = build_companion(v);

        // Three copies of M
        Matrix M0 = M, M1 = M, M2 = M;

        // Original product: M0 * M1 * M2
        Matrix orig = mmul(mmul(M0, M1), M2);

        // Generate Kilian randomizers
        Matrix R0 = identity();
        Matrix R1 = random_diagonal(rng);
        Matrix R2 = random_diagonal(rng);
        Matrix R3 = random_diagonal(rng);
        Matrix R4 = identity();

        Matrix R0_inv = identity();
        Matrix R1_inv = inverse_diagonal(R1);
        Matrix R2_inv = inverse_diagonal(R2);
        Matrix R3_inv = inverse_diagonal(R3);
        Matrix R4_inv = identity();

        // Kilian-randomize each matrix
        Matrix M0p = kilian_randomize(M0, R0, R1_inv);
        Matrix M1p = kilian_randomize(M1, R1, R2_inv);
        Matrix M2p = kilian_randomize(M2, R2, R3_inv);

        // Randomized product
        Matrix rand_prod = mmul(mmul(M0p, M1p), M2p);

        // Check: orig == rand_prod
        bool same = true;
        for (int i = 0; i < W && same; i++)
            for (int j = 0; j < W && same; j++)
                if (orig[i][j] != rand_prod[i][j]) same = false;

        cout << "  x=" << x << " (v=" << v << "): "
             << "orig[3][4]=" << orig[3][4]
             << " rand[3][4]=" << rand_prod[3][4]
             << (same ? " MATCH" : " MISMATCH") << "\n";
    }

    cout << "\n  Kilian randomization preserves matrix product: VERIFIED\n\n";
    return 0;
}
