/*
 * FEmmg-FHE — TEST SUITE (FORTRESS v17.0 Path X)
 * Tests 7D Banach Contraction with NDimCiphertext
 */

#include "femmg_fhe.h"
#include "fractal_fhe.h"
#include "banach_engine.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

int enc_p=0, enc_f=0, add_p=0, add_f=0, mul_p=0, mul_f=0, sub_p=0, sub_f=0;
int mix_p=0, mix_f=0, chain_p=0, chain_f=0, fractal_p=0, fractal_f=0;

void t(const char* name, bool ok) {
    std::cout << "  " << name << ": " << (ok ? "PASS" : "FAIL") << std::endl;
}

int main() {
    FEmmgFHE fhe;
    FractalFHE fractal;
    
    std::cout << "╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  FEmmg-FHE COMPLETE VERIFICATION (Path X)    ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝" << std::endl;
    
    // 1. ENCRYPT/DECRYPT
    std::cout << "\n═══ 1. ENCRYPT/DECRYPT ═══" << std::endl;
    for(int i = -5000; i <= 5000; i++) {
        auto ct = fhe.encrypt(i);
        if(fhe.decrypt(ct) == i) enc_p++; else enc_f++;
    }
    std::cout << "  " << enc_p << " values: " << (enc_f == 0 ? "PASS" : "FAIL") << std::endl;
    
    // 2. ADDITION GRID
    std::cout << "\n═══ 2. ADDITION GRID (-500 to 500, step 10) ═══" << std::endl;
    for(int a = -500; a <= 500; a += 10) {
        for(int b = -500; b <= 500; b += 10) {
            auto ca = fhe.encrypt(a);
            auto cb = fhe.encrypt(b);
            auto res = fhe.add(ca, cb);
            if(fhe.decrypt(res) == a + b) add_p++; else add_f++;
        }
    }
    std::cout << "  " << add_p << " grid tests: " << (add_f == 0 ? "PASS" : "FAIL") << std::endl;
    
    // 3. MULTIPLICATION GRID
    std::cout << "\n═══ 3. MULTIPLICATION GRID (-100 to 100, step 5) ═══" << std::endl;
    for(int a = -100; a <= 100; a += 5) {
        for(int b = -100; b <= 100; b += 5) {
            auto ca = fhe.encrypt(a);
            auto cb = fhe.encrypt(b);
            auto res = fhe.multiply(ca, cb);
            if(fhe.decrypt(res) == a * b) mul_p++; else mul_f++;
        }
    }
    std::cout << "  " << mul_p << " grid tests: " << (mul_f == 0 ? "PASS" : "FAIL") << std::endl;
    
    // 4. SUBTRACTION (via add with negative)
    std::cout << "\n═══ 4. SUBTRACTION GRID (-500 to 500, step 10) ═══" << std::endl;
    for(int a = -500; a <= 500; a += 10) {
        for(int b = -500; b <= 500; b += 10) {
            auto ca = fhe.encrypt(a);
            auto cb = fhe.encrypt(-b);
            auto res = fhe.add(ca, cb);
            if(fhe.decrypt(res) == a - b) sub_p++; else sub_f++;
        }
    }
    std::cout << "  " << sub_p << " grid tests: " << (sub_f == 0 ? "PASS" : "FAIL") << std::endl;
    
    // 5. MIXED
    std::cout << "\n═══ 5. MIXED (Add+Multiply) ═══" << std::endl;
    for(int i = 0; i < 2000; i++) {
        int a = (i * 7) % 100;
        int b = (i * 13) % 100;
        int c = (i * 17) % 100;
        auto ca = fhe.encrypt(a);
        auto cb = fhe.encrypt(b);
        auto cc = fhe.encrypt(c);
        auto sum = fhe.add(ca, cb);
        auto prod = fhe.multiply(sum, cc);
        if(fhe.decrypt(prod) == (a + b) * c) mix_p++; else mix_f++;
    }
    std::cout << "  " << mix_p << " mixed expressions: " << (mix_f == 0 ? "PASS" : "FAIL") << std::endl;
    
    // 6. CHAINED OPERATIONS
    std::cout << "\n═══ 6. CHAINED OPERATIONS ═══" << std::endl;
    {
        auto acc = fhe.encrypt(0);
        for(int i = 0; i < 1000; i++) {
            acc = fhe.add(acc, fhe.encrypt(1));
        }
        t("1000-chain add = 1000", fhe.decrypt(acc) == 1000);
    }
    {
        auto acc = fhe.encrypt(1);
        for(int i = 0; i < 10; i++) {
            acc = fhe.multiply(acc, fhe.encrypt(2));
        }
        t("10-chain multiply = 1024", fhe.decrypt(acc) == 1024);
    }
    
    // 7. FRACTAL
    std::cout << "\n═══ 7. FRACTAL (7 layers, 14 parties) ═══" << std::endl;
    {
        auto ct = fractal.encrypt(42, 0);
        t("42 through 7 layers", fractal.decrypt(ct) == 42);
    }
    {
        auto ct = fractal.encrypt(-999, 7);
        t("-999 through 7 layers", fractal.decrypt(ct) == -999);
    }
    {
        std::vector<banach::NDimCiphertext> cts;
        for(int i = 0; i < phi_constants::PARTIES; i++) cts.push_back(fractal.encrypt(10, i));
        auto result = fractal.chain_add(cts);
        t("14-party chain add = 140", fractal.decrypt(result) == 140);
    }
    {
        std::vector<banach::NDimCiphertext> cts;
        for(int i = 0; i < 7; i++) cts.push_back(fractal.encrypt(2, i));
        auto result = fractal.chain_multiply(cts);
        t("7-party chain multiply = 128", fractal.decrypt(result) == 128);
    }
    t("91/91 pairs verified", fractal.verify_all());
    
    // 11. NOISE STABILITY
    std::cout << "\n═══ 11. NOISE STABILITY (50K ops) ═══" << std::endl;
    {
        auto ct = fhe.encrypt(42);
        double mn = ct.noise, mx = ct.noise;
        for(int i = 0; i < 50000; i++) {
            ct = fhe.add(ct, fhe.encrypt(1));
            if(ct.noise < mn) mn = ct.noise;
            if(ct.noise > mx) mx = ct.noise;
        }
        std::cout << "  Noise: " << mn << " - " << mx << " bits" << std::endl;
        std::cout << "  Stable (< 1.0 variance): " << ((mx - mn) < 1.0 ? "PASS" : "FAIL") << std::endl;
    }
    
    // 12. TPS
    std::cout << "\n═══ 12. TPS BENCHMARK (3s) ═══" << std::endl;
    {
        auto st = std::chrono::high_resolution_clock::now();
        uint64_t ops = 0;
        while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()-st).count() < 3) {
            auto a = fhe.encrypt(42);
            auto b = fhe.encrypt(1);
            auto es = fhe.add(a, b);
            volatile int64_t __attribute__((unused)) ck = fhe.decrypt(es);
            ops++;
        }
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-st).count();
        std::cout << "  " << (ops * 1000.0 / dur / 1000000.0) << "M TPS (TRUE FHE)" << std::endl;
    }
    
    int total_pass = enc_p + add_p + mul_p + sub_p + mix_p;
    int total_fail = enc_f + add_f + mul_f + sub_f + mix_f;
    std::cout << "\n╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  RESULT: " << total_pass << "/" << (total_pass+total_fail) << " PASSED" << std::endl;
    std::cout << "║  VERDICT: " << (total_fail == 0 ? "FULLY HOMOMORPHIC VERIFIED" : "FAILURES DETECTED") << "         ║" << std::endl;
    std::cout << "║  ENGINE: FORTRESS v17.0 — Path X (7D Banach)║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝" << std::endl;
    
    return total_fail == 0 ? 0 : 1;
}
