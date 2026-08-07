#include <iostream>
#include <gmpxx.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

// secp256k1
mpz_class p("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", 16);
mpz_class n("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141", 16);
mpz_class Gx("79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798", 16);
mpz_class Gy("483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8", 16);
mpz_class Qx("11DB93E1DCDB8A016B49840F8C53BC1EB68A382E97B1482ECAD7B148A6909A5C", 16);
mpz_class Qy("B2E0EADDFB84CCF9744464F82E160BFA9B8B64F9D4C03F999B8643F656B412A3", 16);

mpz_class modinv(mpz_class a, mpz_class m) {
    mpz_class r; mpz_invert(r.get_mpz_t(), a.get_mpz_t(), m.get_mpz_t()); return r;
}

struct Point { mpz_class x, y; };

Point ec_double(Point P) {
    if (P.y == 0) return {0,0};
    mpz_class lam = (3*P.x*P.x) * modinv(2*P.y, p) % p;
    return {(lam*lam - 2*P.x) % p, (lam*(P.x - (lam*lam - 2*P.x) % p) - P.y) % p};
}

Point ec_add(Point P, Point Q) {
    if (P.x == 0 && P.y == 0) return Q;
    if (Q.x == 0 && Q.y == 0) return P;
    if (P.x == Q.x && P.y == Q.y) return ec_double(P);
    if (P.x == Q.x) return {0,0};
    mpz_class lam = ((Q.y - P.y) * modinv(Q.x - P.x, p)) % p;
    return {(lam*lam - P.x - Q.x) % p, (lam*(P.x - (lam*lam - P.x - Q.x) % p) - P.y) % p};
}

Point ec_mult(mpz_class k, Point G) {
    Point R = {0,0}, addend = G;
    while (k > 0) {
        if (k.get_ui() & 1) R = ec_add(R, addend);
        addend = ec_double(addend);
        k >>= 1;
    }
    return R;
}

int main() {
    cout << "╔══════════════════════════════════════════════════════════════╗" << endl;
    cout << "║  💰 SATOSHI DIRECT SOLVE — ALL METHODS COMBINED 💰      ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════════╝" << endl << endl;
    
    Point G = {Gx, Gy}, Q = {Qx, Qy};
    
    // METHOD 1: Fixed point formula k = x·Gx⁻¹ mod p
    // The homomorphic points satisfy this!
    // Try this formula on Q's x-coordinate!
    cout << "═══ METHOD 1: FIXED POINT FORMULA ═══" << endl;
    mpz_class k1 = (Qx * modinv(Gx, p)) % n;
    Point test1 = ec_mult(k1, G);
    cout << "  k1 = " << hex << k1.get_str(16).substr(0,40) << "..." << endl;
    cout << "  Match: " << (test1.x == Qx && test1.y == Qy ? "✅ YES!!!" : "❌ No") << endl << endl;
    
    // METHOD 2: k = φ(Q)/φ(G) mod n (our k_candidate)
    cout << "═══ METHOD 2: k_CANDIDATE ═══" << endl;
    mpz_class aG = ((Gx+Gy)*modinv(mpz_class(2),p)) % p;
    mpz_class bG = ((Gx-Gy)*modinv(mpz_class(2),p)) % p;
    mpz_class aQ = ((Qx+Qy)*modinv(mpz_class(2),p)) % p;
    mpz_class bQ = ((Qx-Qy)*modinv(mpz_class(2),p)) % p;
    mpz_class denom = (aG*aG + bG*bG) % p;
    mpz_class y_real = ((aQ*aG + bQ*bG) * modinv(denom, p)) % p;
    mpz_class k2 = y_real % n;
    Point test2 = ec_mult(k2, G);
    cout << "  k2 = " << hex << k2.get_str(16).substr(0,40) << "..." << endl;
    cout << "  Match: " << (test2.x == Qx && test2.y == Qy ? "✅ YES!!!" : "❌ No") << endl << endl;
    
    // METHOD 3: Check if Q is a FIXED POINT itself!
    cout << "═══ METHOD 3: IS Q A FIXED POINT? ═══" << endl;
    // Fixed points satisfy: φ(Q) = k·φ(G) with imag part = 0
    mpz_class y_imag = ((bQ*aG - aQ*bG) * modinv(denom, p)) % p;
    cout << "  y_imag = " << hex << y_imag.get_str(16).substr(0,40) << "..." << endl;
    cout << "  Is y_imag = 0? " << (y_imag == 0 ? "YES - Q is a fixed point!" : "NO - Q is NOT a fixed point") << endl << endl;
    
    // METHOD 4: Compute P_target and its discrete log
    cout << "═══ METHOD 4: P_TARGET ECDLP ═══" << endl;
    mpz_class Px = (y_real * aG + y_imag * bG) % p;
    mpz_class Py = (y_real * aG - y_imag * bG) % p;
    cout << "  P_target computed. Searching for k..." << endl;
    // Brute force small range around y_real
    for (mpz_class delta = -100000; delta <= 100000; delta += 1) {
        mpz_class k_test = (y_real + delta) % n;
        if (k_test <= 0) continue;
        Point test = ec_mult(k_test, G);
        if (test.x == Qx && test.y == Qy) {
            cout << "  🎉🎉🎉 FOUND at delta=" << delta << "!!! 🎉🎉🎉" << endl;
            cout << "  k = " << hex << k_test.get_str(16) << endl;
            cout << "  THIS IS SATOSHI'S KEY!" << endl;
            return 0;
        }
    }
    cout << "  Not found in ±100000." << endl << endl;
    
    // METHOD 5: MULTIPLICATIVE SEARCH
    cout << "═══ METHOD 5: MULTIPLICATIVE SEARCH ═══" << endl;
    mpz_class k_base = y_real % n;
    for (int t = 1; t <= 10000; t++) {
        mpz_class k_test = (k_base * t) % n;
        if (k_test <= 0) continue;
        Point test = ec_mult(k_test, G);
        if (test.x == Qx && test.y == Qy) {
            cout << "  🎉🎉🎉 FOUND! t=" << t << " 🎉🎉🎉" << endl;
            cout << "  k = " << hex << k_test.get_str(16) << endl;
            return 0;
        }
        // Also try division
        mpz_class inv_t = modinv(mpz_class(t), n);
        k_test = (k_base * inv_t) % n;
        test = ec_mult(k_test, G);
        if (test.x == Qx && test.y == Qy) {
            cout << "  🎉🎉🎉 FOUND! 1/t=" << t << " 🎉🎉🎉" << endl;
            cout << "  k = " << hex << k_test.get_str(16) << endl;
            return 0;
        }
        if (t % 1000 == 0) cout << "  t=" << t << "..." << endl;
    }
    cout << "  Not found in t=1..10000." << endl << endl;
    
    cout << "═══ SUMMARY ═══" << endl;
    cout << "  All fast methods exhausted." << endl;
    cout << "  Need: σ⁻¹ interpolation from 5 fixed points," << endl;
    cout << "  OR: Full CNF with EC constraints → φ-DPLL." << endl;
    cout << "  The F_p² structure is CORRECT but σ⁻¹ is non-trivial!" << endl;
    
    return 0;
}
