#pragma once
#include <gmp.h>
#include <string>
#include <iostream>

const char* P_HEX = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F";
const char* GX_HEX = "79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798";
const char* GY_HEX = "483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8";

struct Secp256k1Point {
    mpz_t x, y;
    bool is_infinity;
    Secp256k1Point() { mpz_inits(x, y, NULL); is_infinity = false; }
    ~Secp256k1Point() { mpz_clears(x, y, NULL); }
};

class Secp256k1Context {
public:
    mpz_t p, a, b, n;
    Secp256k1Point G;
    
    Secp256k1Context() {
        mpz_inits(p, a, b, n, G.x, G.y, NULL);
        mpz_set_str(p, P_HEX, 16);
        mpz_set_str(a, "0", 16);
        mpz_set_str(b, "7", 16);
        mpz_set_str(n, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141", 16);
        mpz_set_str(G.x, GX_HEX, 16);
        mpz_set_str(G.y, GY_HEX, 16);
        G.is_infinity = false;
    }
    
    ~Secp256k1Context() { mpz_clears(p, a, b, n, G.x, G.y, NULL); }
    
    void mod_add(mpz_t r, const mpz_t x, const mpz_t y) { 
        mpz_add(r, x, y); mpz_mod(r, r, p); 
    }
    
    void mod_sub(mpz_t r, const mpz_t x, const mpz_t y) { 
        mpz_sub(r, x, y); mpz_mod(r, r, p); 
    }
    
    void mod_mul(mpz_t r, const mpz_t x, const mpz_t y) { 
        mpz_mul(r, x, y); mpz_mod(r, r, p); 
    }
    
    void mod_inv(mpz_t r, const mpz_t x) {
        mpz_invert(r, x, p);
    }
    
    void point_add(Secp256k1Point& R, const Secp256k1Point& P, const Secp256k1Point& Q) {
        if (P.is_infinity) { 
            mpz_set(R.x, Q.x); mpz_set(R.y, Q.y); 
            R.is_infinity = Q.is_infinity; return; 
        }
        if (Q.is_infinity) { 
            mpz_set(R.x, P.x); mpz_set(R.y, P.y); 
            R.is_infinity = P.is_infinity; return; 
        }
        
        mpz_t tmp1, tmp2;
        mpz_inits(tmp1, tmp2, NULL);
        
        // Check if P = -Q (same x, opposite y)
        if (mpz_cmp(P.x, Q.x) == 0) {
            mpz_add(tmp1, P.y, Q.y);
            mpz_mod(tmp1, tmp1, p);
            
            if (mpz_cmp_ui(tmp1, 0) == 0) {
                R.is_infinity = true;
                mpz_clears(tmp1, tmp2, NULL);
                return;
            }
        }
        
        mpz_t lambda, num, den;
        mpz_inits(lambda, num, den, NULL);
        
        if (mpz_cmp(P.x, Q.x) == 0 && mpz_cmp(P.y, Q.y) == 0) {
            // Point doubling
            mpz_mul(num, P.x, P.x);
            mpz_mul_ui(num, num, 3);
            mpz_mod(num, num, p);
            
            mpz_mul_ui(den, P.y, 2);
            mpz_mod(den, den, p);
        } else {
            // Point addition
            mod_sub(num, Q.y, P.y);
            mod_sub(den, Q.x, P.x);
        }
        
        mod_inv(tmp1, den);
        mod_mul(lambda, num, tmp1);
        
        // x3 = λ² - x1 - x2
        mpz_mul(tmp1, lambda, lambda);
        mod_sub(tmp1, tmp1, P.x);
        mod_sub(R.x, tmp1, Q.x);
        
        // y3 = λ(x1 - x3) - y1
        mod_sub(tmp1, P.x, R.x);
        mod_mul(tmp2, lambda, tmp1);
        mod_sub(R.y, tmp2, P.y);
        
        R.is_infinity = false;
        
        mpz_clears(lambda, num, den, tmp1, tmp2, NULL);
    }
    
    void point_mul(Secp256k1Point& R, const mpz_t k, const Secp256k1Point& P) {
        R.is_infinity = true;
        Secp256k1Point base;
        mpz_set(base.x, P.x);
        mpz_set(base.y, P.y);
        base.is_infinity = P.is_infinity;
        
        mpz_t temp_k;
        mpz_init_set(temp_k, k);
        
        while (mpz_cmp_ui(temp_k, 0) > 0) {
            if (mpz_odd_p(temp_k)) {
                Secp256k1Point sum;
                point_add(sum, R, base);
                mpz_set(R.x, sum.x);
                mpz_set(R.y, sum.y);
                R.is_infinity = sum.is_infinity;
            }
            
            Secp256k1Point doubled;
            point_add(doubled, base, base);
            mpz_set(base.x, doubled.x);
            mpz_set(base.y, doubled.y);
            base.is_infinity = doubled.is_infinity;
            
            mpz_fdiv_q_2exp(temp_k, temp_k, 1);
        }
        
        mpz_clear(temp_k);
    }
    
    bool on_curve(const Secp256k1Point& P) {
        if (P.is_infinity) return true;
        
        mpz_t lhs, rhs, x3;
        mpz_inits(lhs, rhs, x3, NULL);
        
        mpz_mul(lhs, P.y, P.y);
        mpz_mod(lhs, lhs, p);
        
        mpz_mul(x3, P.x, P.x);
        mpz_mul(x3, x3, P.x);
        mpz_mod(x3, x3, p);
        
        mpz_add(rhs, x3, b);
        mpz_mod(rhs, rhs, p);
        
        bool result = (mpz_cmp(lhs, rhs) == 0);
        mpz_clears(lhs, rhs, x3, NULL);
        return result;
    }
};
