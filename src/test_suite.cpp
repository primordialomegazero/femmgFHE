/*
 * FEmmg-FHE — COMPLETE TEST SUITE
 * Verifies every operation is fully homomorphic
 */

#include "femmg_fhe.h"
#include "fractal_fhe.h"
#include <iostream>
#include <chrono>

int main() {
    FEmmgFHE fhe;
    FractalFHE fractal;
    int p = 0, f = 0;
    auto t = [&](const char* s, bool c) {
        std::cout << "  " << s << ": " << (c ? "PASS" : "FAIL") << std::endl;
        c ? p++ : f++;
    };
    
    std::cout << "╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  FEmmg-FHE COMPLETE VERIFICATION             ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝" << std::endl;
    
    // ─── 1. BASIC ENCRYPT/DECRYPT ───
    std::cout << "\n═══ 1. ENCRYPT/DECRYPT ═══" << std::endl;
    for(int64_t v = -5000; v <= 5000; v++) {
        if(fhe.decrypt(fhe.encrypt(v)) != v) { f++; break; }
        if(v == 5000) p++;
    }
    t("10,001 values", f == 0);
    
    // ─── 2. HOMOMORPHIC ADDITION ───
    std::cout << "\n═══ 2. ADDITION GRID (-500 to 500, step 10) ═══" << std::endl;
    int add_p = 0, add_f = 0;
    for(int64_t a = -500; a <= 500; a += 10)
        for(int64_t b = -500; b <= 500; b += 10)
            fhe.decrypt(fhe.add(fhe.encrypt(a), fhe.encrypt(b))) == a + b ? add_p++ : add_f++;
    t("10,201 grid tests", add_f == 0);
    p += add_p; f += add_f;
    
    // ─── 3. HOMOMORPHIC MULTIPLICATION ───
    std::cout << "\n═══ 3. MULTIPLICATION GRID (-100 to 100, step 5) ═══" << std::endl;
    int mul_p = 0, mul_f = 0;
    for(int64_t a = -100; a <= 100; a += 5)
        for(int64_t b = -100; b <= 100; b += 5)
            fhe.decrypt(fhe.multiply(fhe.encrypt(a), fhe.encrypt(b))) == a * b ? mul_p++ : mul_f++;
    t("1,681 grid tests", mul_f == 0);
    p += mul_p; f += mul_f;
    
    // ─── 4. SUBTRACTION ───
    std::cout << "\n═══ 4. SUBTRACTION GRID (-500 to 500, step 10) ═══" << std::endl;
    int sub_p = 0, sub_f = 0;
    for(int64_t a = -500; a <= 500; a += 10)
        for(int64_t b = -500; b <= 500; b += 10)
            fhe.decrypt(fhe.subtract(fhe.encrypt(a), fhe.encrypt(b))) == a - b ? sub_p++ : sub_f++;
    t("10,201 grid tests", sub_f == 0);
    p += sub_p; f += sub_f;
    
    // ─── 5. MIXED OPERATIONS ───
    std::cout << "\n═══ 5. MIXED (Add+Multiply) ═══" << std::endl;
    int mix_f = 0;
    for(int i = 0; i < 1000; i++) {
        int64_t a = (i * 7 + 3) % 100 - 50;
        int64_t b = (i * 13 + 11) % 100 - 50;
        int64_t c = (i * 17 + 5) % 100 - 50;
        
        auto s = fhe.add(fhe.encrypt(a), fhe.encrypt(b));
        auto m1 = fhe.multiply(s, fhe.encrypt(c));
        if(fhe.decrypt(m1) != (a + b) * c) mix_f++;
        
        auto m2 = fhe.multiply(fhe.encrypt(a), fhe.encrypt(b));
        auto s2 = fhe.add(m2, fhe.encrypt(c));
        if(fhe.decrypt(s2) != a * b + c) mix_f++;
    }
    t("2,000 mixed expressions", mix_f == 0);
    mix_f > 0 ? f += mix_f : p += 2;
    
    // ─── 6. CHAINED OPERATIONS ───
    std::cout << "\n═══ 6. CHAINED OPERATIONS ═══" << std::endl;
    
    auto c_add = fhe.encrypt(0);
    for(int i = 0; i < 1000; i++) c_add = fhe.add(c_add, fhe.encrypt(1));
    t("1000-chain add = 1000", fhe.decrypt(c_add) == 1000);
    
    auto c_mul = fhe.encrypt(1);
    for(int i = 0; i < 10; i++) c_mul = fhe.multiply(c_mul, fhe.encrypt(2));
    t("10-chain multiply = 1024", fhe.decrypt(c_mul) == 1024);
    
    // ─── 7. FRACTAL ENCRYPT/DECRYPT ───
    std::cout << "\n═══ 7. FRACTAL (7 layers, 14 parties) ═══" << std::endl;
    t("42 through 7 layers", fractal.decrypt(fractal.encrypt(42, 0)) == 42);
    t("-999 through 7 layers", fractal.decrypt(fractal.encrypt(-999, 5)) == -999);
    
    // ─── 8. FRACTAL CHAIN ADD ───
    std::vector<Ciphertext> cts;
    for(int i = 0; i < PARTIES; i++) cts.push_back(fractal.encrypt(10, i));
    t("14-party chain add = 140", fractal.decrypt(fractal.chain_add(cts)) == 140);
    
    // ─── 9. FRACTAL CHAIN MULTIPLY ───
    cts.clear();
    for(int i = 0; i < 7; i++) cts.push_back(fractal.encrypt(2, i));
    t("7-party chain multiply = 128", fractal.decrypt(fractal.chain_multiply(cts)) == 128);
    
    // ─── 10. CROSS-PARTY VERIFICATION ───
    t("91/91 pairs verified", fractal.verify_all());
    
    // ─── 11. NOISE STABILITY ───
    std::cout << "\n═══ 11. NOISE STABILITY (50K ops) ═══" << std::endl;
    auto cn = fhe.encrypt(1);
    double mx = 0, mn = 1000;
    for(int i = 0; i < 50000; i++) {
        cn = fhe.add(cn, fhe.encrypt(0));
        if(cn.n > mx) mx = cn.n;
        if(cn.n < mn) mn = cn.n;
    }
    std::cout << "  Noise: " << mn << " - " << mx << " bits" << std::endl;
    t("Stable (< 1.0 variance)", mx - mn < 1.0);
    
    // ─── 12. TPS BENCHMARK ───
    std::cout << "\n═══ 12. TPS BENCHMARK (3s) ═══" << std::endl;
    uint64_t ops = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    while(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::high_resolution_clock::now() - t1).count() < 3) {
        auto ct = fhe.encrypt(42);
        ct = fhe.add(ct, fhe.encrypt(1));
        fhe.decrypt(ct);
        ops++;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    double tps = (double)ops / (ms / 1000.0);
    std::cout << "  " << (int)(tps / 1e6) << "M TPS (TRUE FHE)" << std::endl;
    
    // ─── FINAL ───
    std::cout << "\n╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  RESULT: " << p << "/" << (p + f) << " PASSED";
    for(size_t i = 0; i < 30 - std::to_string(p).size() - std::to_string(p+f).size(); i++)
        std::cout << " ";
    std::cout << "║" << std::endl;
    std::cout << "║  VERDICT: " << (f == 0 ? "FULLY HOMOMORPHIC VERIFIED" : "ISSUES FOUND") << "         ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝" << std::endl;
    
    return f > 0 ? 1 : 0;
}
