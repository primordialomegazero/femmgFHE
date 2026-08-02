#include <iostream>
#include <vector>
using namespace std;
using Matrix = vector<vector<int>>;

Matrix mat_mult(const Matrix& A, const Matrix& B) {
    Matrix C(3, vector<int>(3, 0));
    for (int i=0; i<3; i++) for (int j=0; j<3; j++) for (int k=0; k<3; k++) C[i][j] += A[i][k] * B[k][j];
    return C;
}
Matrix cycle3() { return {{0,1,0},{0,0,1},{1,0,0}}; }
Matrix identity3() { return {{1,0,0},{0,1,0},{0,0,1}}; }
Matrix encode(int bit) { return bit ? cycle3() : identity3(); }
int decode(const Matrix& M) { return (M[0][0] == 0) ? 1 : 0; }
Matrix gate_NAND() { return {{0,1,0},{1,0,0},{0,0,1}}; }
Matrix mat_inv(const Matrix& M) { return mat_mult(M, M); }
Matrix NAND(const Matrix& A, const Matrix& B) {
    auto GN = gate_NAND();
    return mat_mult(mat_mult(mat_mult(mat_mult(A, B), GN), mat_inv(A)), mat_inv(B));
}

int main() {
    // XOR(a,b) = NAND(NAND(a, NAND(a,b)), NAND(b, NAND(a,b)))
    auto XOR = [&](const Matrix& a, const Matrix& b) {
        auto n1 = NAND(a, b);
        return NAND(NAND(a, n1), NAND(b, n1));
    };
    
    // MAJ(a,b,c) = NAND(NAND(NAND(a,b), NAND(b,c)), NAND(NAND(a,b), NAND(a,c))) ... etc
    // Simpler: MAJ = XOR(a,b) NAND c? No. Let's use direct NAND composition.
    // MAJ = (a∧b) ∨ (b∧c) ∨ (a∧c) = NAND(NAND(NAND(a,b), NAND(b,c)), NAND(NAND(a,b), NAND(a,c)))
    
    auto MAJ = [&](const Matrix& a, const Matrix& b, const Matrix& c) {
        auto n_ab = NAND(a, b);
        auto n_bc = NAND(b, c);
        auto n_ac = NAND(a, c);
        auto n1 = NAND(n_ab, n_bc);
        auto n2 = NAND(n_ab, n_ac);
        return NAND(n1, n2);
    };

    cout << "XOR via NAND:\n";
    for (int a : {0, 1}) for (int b : {0, 1}) {
        int r = decode(XOR(encode(a), encode(b)));
        cout << "XOR(" << a << b << ")=" << r << " exp=" << (a^b) << (r==(a^b)?" ✓":" ✗") << "\n";
    }
    
    cout << "\nMAJ via NAND:\n";
    for (int a : {0, 1}) for (int b : {0, 1}) for (int c : {0, 1}) {
        int r = decode(MAJ(encode(a), encode(b), encode(c)));
        int e = (a+b+c >= 2) ? 1 : 0;
        cout << "MAJ(" << a << b << c << ")=" << r << " exp=" << e << (r==e?" ✓":" ✗") << "\n";
    }
    return 0;
}
