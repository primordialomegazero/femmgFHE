#include <iostream>
#include <gmpxx.h>
#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;

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

Point ec_mult(mpz_class k, Point G) {
    Point R = {mpz_class(0), mpz_class(0)}, addend = G;
    while (k > 0) {
        if (k.get_ui() & 1) R = ec_add(R, addend);
        addend = ec_double(addend);
        k >>= 1;
    }
    return R;
}

int main() {
    init();
    Point G = {Gx, Gy}, Q = {Qx, Qy};
    
    gmp_printf("╔══════════════════════════════════════════════════════════════╗\n");
    gmp_printf("║  💰 SATOSHI — DIRECT ATTACK — NO MORE DETOURS 💰        ║\n");
    gmp_printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    // Compute φ(G) and φ(Q) in F_p²
    mpz_class aG = ((Gx+Gy)*modinv(mpz_class(2),p)) % p;
    mpz_class bG = ((Gx-Gy)*modinv(mpz_class(2),p)) % p;
    mpz_class aQ = ((Qx+Qy)*modinv(mpz_class(2),p)) % p;
    mpz_class bQ = ((Qx-Qy)*modinv(mpz_class(2),p)) % p;
    mpz_class denom = (aG*aG + bG*bG) % p;
    mpz_class y_real = ((aQ*aG + bQ*bG) * modinv(denom, p)) % p;
    mpz_class y_imag = ((bQ*aG - aQ*bG) * modinv(denom, p)) % p;
    
    gmp_printf("y_real = %Zx\n", y_real.get_mpz_t());
    gmp_printf("y_imag = %Zx\n\n", y_imag.get_mpz_t());
    
    // DIRECT SEARCH: k = y_real * t mod n
    gmp_printf("═══ DIRECT MULTIPLICATIVE SEARCH ═══\n");
    gmp_printf("Searching k = y_real * t mod n for t=1..100000...\n");
    
    auto start = high_resolution_clock::now();
    
    for (long t = 1; t <= 100000; t++) {
        mpz_class k_test = (y_real * t) % n;
        if (k_test <= 0) continue;
        
        Point test = ec_mult(k_test, G);
        if (test.x == Qx && test.y == Qy) {
            auto end = high_resolution_clock::now();
            double sec = duration_cast<milliseconds>(end - start).count() / 1000.0;
            
            gmp_printf("\n🎉🎉🎉 FOUND! t=%ld 🎉🎉🎉\n", t);
            gmp_printf("SATOSHI PRIVATE KEY: %Zx\n", k_test.get_mpz_t());
            gmp_printf("Time: %.2f seconds\n", sec);
            return 0;
        }
        
        // Also try division
        if (t > 1) {
            mpz_class inv_t = modinv(mpz_class(t), n);
            mpz_class k_div = (y_real * inv_t) % n;
            test = ec_mult(k_div, G);
            if (test.x == Qx && test.y == Qy) {
                gmp_printf("\n🎉🎉🎉 FOUND! t=1/%ld 🎉🎉🎉\n", t);
                gmp_printf("SATOSHI PRIVATE KEY: %Zx\n", k_div.get_mpz_t());
                return 0;
            }
        }
        
        if (t % 10000 == 0) gmp_printf("  t=%ld...\n", t);
    }
    
    // Try ±delta around y_real
    gmp_printf("\n═══ DIRECT ±DELTA SEARCH ═══\n");
    gmp_printf("Searching k = y_real + delta for delta=-50000..50000...\n");
    
    for (long delta = -50000; delta <= 50000; delta++) {
        mpz_class k_test = (y_real + delta) % n;
        if (k_test <= 0) continue;
        
        Point test = ec_mult(k_test, G);
        if (test.x == Qx && test.y == Qy) {
            gmp_printf("\n🎉🎉🎉 FOUND! delta=%ld 🎉🎉🎉\n", delta);
            gmp_printf("SATOSHI PRIVATE KEY: %Zx\n", k_test.get_mpz_t());
            return 0;
        }
        if (delta % 10000 == 0) gmp_printf("  delta=%ld...\n", delta);
    }
    
    gmp_printf("\nNot found in direct searches.\n");
    gmp_printf("σ⁻¹ is more complex than simple multiplication/addition.\n");
    gmp_printf("Need: full interpolation from 5 fixed points.\n");
    
    return 0;
}
