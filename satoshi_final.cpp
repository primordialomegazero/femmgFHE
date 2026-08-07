#include <iostream>
#include <gmpxx.h>
#include <string>

using namespace std;

mpz_class p, n, Gx, Gy, Qx, Qy;

void init() {
    p.set_str("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", 16);
    n.set_str("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141", 16);
    Gx.set_str("79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798", 16);
    Gy.set_str("483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8", 16);
    Qx.set_str("11DB93E1DCDB8A016B49840F8C53BC1EB68A382E97B1482ECAD7B148A6909A5C", 16);
    Qy.set_str("B2E0EADDFB84CCF9744464F82E160BFA9B8B64F9D4C03F999B8643F656B412A3", 16);
}

mpz_class modinv(mpz_class a, mpz_class m) {
    mpz_class r; mpz_invert(r.get_mpz_t(), a.get_mpz_t(), m.get_mpz_t()); return r;
}

struct Point { mpz_class x, y; };

Point ec_double(Point P) {
    if (P.y == 0) return {mpz_class(0), mpz_class(0)};
    mpz_class lam = (3*P.x*P.x) * modinv(2*P.y, p) % p;
    mpz_class x3 = (lam*lam - 2*P.x) % p;
    return {x3, (lam*(P.x - x3) - P.y) % p};
}

Point ec_add(Point P, Point Q) {
    if (P.x == 0 && P.y == 0) return Q;
    if (Q.x == 0 && Q.y == 0) return P;
    if (P.x == Q.x && P.y == Q.y) return ec_double(P);
    if (P.x == Q.x) return {mpz_class(0), mpz_class(0)};
    mpz_class lam = ((Q.y - P.y) * modinv(Q.x - P.x, p)) % p;
    mpz_class x3 = (lam*lam - P.x - Q.x) % p;
    return {x3, (lam*(P.x - x3) - P.y) % p};
}

// FIXED: proper mpz_class bit checking
Point ec_mult(mpz_class k, Point G) {
    Point R = {mpz_class(0), mpz_class(0)}, addend = G;
    while (k > 0) {
        if (mpz_odd_p(k.get_mpz_t())) R = ec_add(R, addend);
        addend = ec_double(addend);
        k >>= 1;
    }
    return R;
}

int main() {
    init();
    Point G = {Gx, Gy}, Q = {Qx, Qy};
    
    cout << "╔══════════════════════════════════════════════════════════════╗" << endl;
    cout << "║  💰 SATOSHI FINAL — NO MORE BUGS 💰                     ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════════╝" << endl << endl;
    
    // Compute φ(G) and φ(Q)
    mpz_class two(2);
    mpz_class aG = ((Gx+Gy)*modinv(two,p)) % p;
    mpz_class bG = ((Gx-Gy)*modinv(two,p)) % p;
    mpz_class aQ = ((Qx+Qy)*modinv(two,p)) % p;
    mpz_class bQ = ((Qx-Qy)*modinv(two,p)) % p;
    mpz_class denom = (aG*aG + bG*bG) % p;
    mpz_class y_real = ((aQ*aG + bQ*bG) * modinv(denom, p)) % p;
    mpz_class y_imag = ((bQ*aG - aQ*bG) * modinv(denom, p)) % p;
    if (y_imag < 0) y_imag += p;  // FIX: make positive
    
    cout << "y_real = " << y_real.get_str(16).substr(0,40) << "..." << endl;
    cout << "y_imag = " << y_imag.get_str(16).substr(0,40) << "..." << endl << endl;
    
    // TEST 1: Is k = y_real the answer?
    mpz_class k_test = y_real % n;
    Point test = ec_mult(k_test, G);
    cout << "TEST 1: k = y_real" << endl;
    cout << "  Match: " << (test.x == Qx && test.y == Qy ? "✅✅✅ SATOSHI KEY!!! ✅✅✅" : "❌") << endl << endl;
    
    // TEST 2: k = y_real + y_imag
    k_test = (y_real + y_imag) % n;
    test = ec_mult(k_test, G);
    cout << "TEST 2: k = y_real + y_imag" << endl;
    cout << "  Match: " << (test.x == Qx && test.y == Qy ? "✅✅✅ SATOSHI KEY!!! ✅✅✅" : "❌") << endl << endl;
    
    // TEST 3: k = y_real - y_imag
    k_test = (y_real - y_imag + n) % n;
    test = ec_mult(k_test, G);
    cout << "TEST 3: k = y_real - y_imag" << endl;
    cout << "  Match: " << (test.x == Qx && test.y == Qy ? "✅✅✅ SATOSHI KEY!!! ✅✅✅" : "❌") << endl << endl;
    
    // TEST 4: Print the actual bits of the result for manual inspection
    cout << "═══ RESULT ═══" << endl;
    if (test.x == Qx && test.y == Qy) {
        cout << "🎉🎉🎉 SATOSHI PRIVATE KEY FOUND! 🎉🎉🎉" << endl;
        cout << "k = " << k_test.get_str(16) << endl;
    } else {
        cout << "Quick tests failed. Need full σ⁻¹ interpolation." << endl;
    }
    
    return 0;
}
