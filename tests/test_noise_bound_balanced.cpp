// NOISE BOUND for BALANCED MASKING
// Hanggang ilang e bago mag-fail ang decryption?

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "NOISE BOUND — BALANCED MASKING\n";
    std::cout << "==============================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    
    // Compute ψ^(2k) = ψ^84
    NTL::ZZ psi_2k = NTL::to_ZZ(1);
    for (int i = 0; i < 84; i++) {
        psi_2k = (psi_2k * psi) % Q;
    }
    
    std::cout << "ψ^84 = " << psi_2k << "\n";
    std::cout << "Q/2 = " << Q/2 << "\n\n";
    
    // Noise bound: e·ψ^84 < Q/2
    // e_max = Q / (2·ψ^84)
    NTL::ZZ e_max = Q / (2 * psi_2k);
    std::cout << "Maximum e (theoretical): " << e_max << "\n";
    std::cout << "e_max bits: " << NTL::NumBits(e_max) << "\n\n";
    
    // Test: e values and decryption success
    std::cout << "e value | e·ψ^84 | < Q/2? | Decrypt m=1\n";
    std::cout << "--------|--------|--------|-----------\n";
    
    for (long e_test : {1L, 10L, 100L, 1000L, 10000L}) {
        NTL::ZZ e = NTL::to_ZZ(e_test);
        NTL::ZZ noise = (e * psi_2k) % Q;
        bool within_bound = noise < Q/2;
        
        // Simulate decrypt para sa m=1
        NTL::ZZ v_psi = (1 + noise) % Q;
        NTL::ZZ dist_1 = (v_psi > 1) ? v_psi - 1 : 1 - v_psi;
        if (dist_1 > Q/2) dist_1 = Q - dist_1;
        NTL::ZZ dist_0 = (v_psi < Q/2) ? v_psi : Q - v_psi;
        bool dec = dist_1 < dist_0;
        
        std::cout << e_test << " | " << noise << " | " 
                  << (within_bound ? "YES" : "NO") << " | "
                  << (dec ? "1 ✓" : "0 ✗") << "\n";
    }
    
    return 0;
}
