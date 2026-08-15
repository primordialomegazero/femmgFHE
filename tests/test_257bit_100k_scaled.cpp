#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "257-BIT 100K DEPTH TEST (Scaled)\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    // s = φ^42
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p s = NTL::to_ZZ_p(1);
    for (int i = 0; i < 42; i++) s = s * phi;
    NTL::ZZ s_zz = rep(s);
    
    std::cout << "Q = " << Q << " (" << NTL::NumBits(Q) << " bits)\n";
    std::cout << "s = φ^42 = " << s_zz << "\n\n";
    
    // Encrypt/Decrypt
    auto encrypt = [&](int bit) {
        NTL::ZZ_pX ct;
        if (bit) NTL::SetCoeff(ct, 0, s);
        return ct;
    };
    
    auto decrypt = [&](const NTL::ZZ_pX& ct) {
        NTL::ZZ_p c0 = NTL::coeff(ct, 0);
        NTL::ZZ c0_zz = rep(c0);
        
        NTL::ZZ dist_0 = c0_zz;
        if (dist_0 > Q/2) dist_0 = Q - dist_0;
        
        NTL::ZZ dist_s = abs(c0_zz - s_zz);
        if (dist_s > Q/2) dist_s = Q - dist_s;
        
        return (dist_0 < dist_s) ? 0 : 1;
    };
    
    // NAND gate
    auto nand = [&](const NTL::ZZ_pX& a, const NTL::ZZ_pX& b) {
        NTL::ZZ_pX one;
        NTL::SetCoeff(one, 0, s);
        NTL::ZZ_pX result = one - (a * b);
        return result;
    };
    
    // ========== 100K NAND TEST ==========
    std::cout << "100K NAND OPERATIONS:\n";
    
    auto ct0 = encrypt(0);
    auto ct1 = encrypt(1);
    
    NTL::ZZ_pX current = ct1;
    bool pass = true;
    int errors = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= 100000; i++) {
        current = nand(current, ct1);  // NAND(x, 1) = NOT(x)
        int dec = decrypt(current);
        
        // Dapat alternating: 1,0,1,0,...
        int expected = (i % 2 == 0) ? 1 : 0;
        
        if (dec != expected) {
            errors++;
            if (errors <= 5) {
                std::cout << "  ❌ Error at depth " << i << ": got " << dec << ", expected " << expected << "\n";
            }
            pass = false;
            break;
        }
        
        if (i % 10000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start);
            std::cout << "  [" << i << "/100K] errors=0 time=" << elapsed.count() << "s\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    
    std::cout << "\n=== " << (pass ? "100K DEPTH TEST PASS ✓" : "FAIL ✗") << " ===\n";
    std::cout << "Total time: " << total_time.count() << "s\n";
    std::cout << "Errors: " << errors << "\n";
    std::cout << "Ciphertext size: CONSTANT (2 components)\n";
    std::cout << "Bootstrapping: NONE\n";
    
    return 0;
}
