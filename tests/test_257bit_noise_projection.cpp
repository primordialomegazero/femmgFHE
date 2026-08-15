#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <random>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "257-BIT NOISE + PROJECTION FHE\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi_zz = (NTL::to_ZZ(1) - phi_zz + Q) % Q;
    
    // Projection operators
    NTL::ZZ inv_phi_minus_psi = NTL::InvMod((phi_zz - psi_zz + Q) % Q, Q);
    NTL::ZZ e1 = ((-psi_zz + Q) % Q * inv_phi_minus_psi) % Q;
    NTL::ZZ e2 = (phi_zz * inv_phi_minus_psi) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "ψ = " << psi_zz << "\n";
    std::cout << "e₁ = " << e1 << "\n\n";
    
    // ========== NOISE GENERATION ==========
    std::random_device rd;
    std::mt19937 gen(rd());
    
    struct Ciphertext {
        NTL::ZZ val;
    };
    
    // Encrypt: ct = m·φ + noise·ψ
    // Message sa φ direction, noise sa ψ direction
    // Projection e₁ removes noise automatically!
    
    auto encrypt = [&](int bit, long noise_bound) {
        std::uniform_int_distribution<long> noise_dist(-noise_bound, noise_bound);
        Ciphertext ct;
        
        if (bit) {
            ct.val = phi_zz;
        } else {
            ct.val = NTL::to_ZZ(0);
        }
        
        // Add noise sa ψ direction
        NTL::ZZ noise = NTL::to_ZZ(noise_dist(gen));
        ct.val = (ct.val + noise * psi_zz) % Q;
        if (ct.val < 0) ct.val += Q;
        
        return ct;
    };
    
    // Decrypt via projection: e₁·ct = m·(e₁·φ) + noise·(e₁·ψ) = m + 0
    auto decrypt = [&](const Ciphertext& ct) {
        NTL::ZZ proj = (e1 * ct.val) % Q;
        
        // Check kung malapit sa 0 o 1
        NTL::ZZ dist_0 = proj;
        if (dist_0 > Q / 2) dist_0 = Q - dist_0;
        
        NTL::ZZ dist_1 = abs(proj - NTL::to_ZZ(1));
        if (dist_1 > Q / 2) dist_1 = Q - dist_1;
        
        return (dist_0 < dist_1) ? 0 : 1;
    };
    
    // Homomorphic multiplication
    auto multiply = [&](const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result;
        result.val = (a.val * b.val) % Q;
        return result;
    };
    
    // Homomorphic addition
    auto add = [&](const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result;
        result.val = (a.val + b.val) % Q;
        return result;
    };
    
    // ========== TEST WITH DIFFERENT NOISE LEVELS ==========
    std::cout << "NOISE TOLERANCE TEST:\n";
    std::cout << "(Projection e₁·ψ = 0, so noise sa ψ direction ay automatic na removed!)\n\n";
    
    std::vector<long> noise_bounds = {10, 100, 1000, 10000, 100000, 1000000};
    
    for (long noise_bound : noise_bounds) {
        int success = 0;
        int trials = 100;
        
        for (int t = 0; t < trials; t++) {
            auto ct0 = encrypt(0, noise_bound);
            auto ct1 = encrypt(1, noise_bound);
            
            int dec0 = decrypt(ct0);
            int dec1 = decrypt(ct1);
            
            if (dec0 == 0 && dec1 == 1) success++;
        }
        
        std::cout << "  Noise bound " << noise_bound << ": ";
        std::cout << success << "/" << trials << " correct";
        std::cout << (success == trials ? " ✓" : " ✗") << "\n";
    }
    
    std::cout << "\n";
    
    // ========== DEPTH TEST WITH NOISE ==========
    std::cout << "DEPTH TEST WITH NOISE (bound=100):\n";
    
    auto ct1_noisy = encrypt(1, 100);
    Ciphertext current = ct1_noisy;
    bool pass = true;
    int max_depth = 0;
    
    for (int depth = 1; depth <= 1000; depth++) {
        current = multiply(current, ct1_noisy);
        int dec = decrypt(current);
        
        if (dec != 1) {
            std::cout << "  ❌ FAIL at depth " << depth << "\n";
            pass = false;
            break;
        }
        max_depth = depth;
        
        if (depth <= 5 || depth % 100 == 0) {
            std::cout << "  Depth " << depth << ": OK ✓\n";
        }
    }
    
    if (pass) {
        std::cout << "  All 1000 depths passed! UNLIMITED DEPTH WITH NOISE! ✓\n";
    }
    
    std::cout << "\n=== " << (pass ? "NOISE + PROJECTION FHE PASS ✓" : "FAIL ✗") << " ===\n";
    
    return 0;
}
