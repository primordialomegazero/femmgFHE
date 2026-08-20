// DUAL-CHANNEL CORRECTION — LAHAT NG APPROACH
// 1. ψ-decode bilang noise estimate para i-correct ang φ-decode
// 2. Adaptive correction: gamitin ang error pattern para matuto
// 3. Fibonacci-spaced correction: gamitin ang error periodicity
// 4. Combined φ+ψ decision: mas maraming information

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  DUAL-CHANNEL CORRECTION\n";
    std::cout << "  Lahat ng Correction Approaches\n";
    std::cout << "========================================\n\n";

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
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    NTL::ZZ inv_psi_k = NTL::InvMod(psi_k, Q);

    NTL::ZZ offset = (phi_k * inv_psi_k) % Q;

    auto encrypt = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };

    auto decode_phi = [&](NTL::ZZ ct) {
        return (ct * inv_phi_k) % Q;
    };
    auto decode_psi = [&](NTL::ZZ ct) {
        return (ct * inv_psi_k) % Q;
    };

    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    auto dist = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ d = (a > b) ? a - b : b - a;
        if (d > Q/2) d = Q - d;
        return d;
    };

    std::cout << "OFFSET = " << offset << "\n\n";

    // ============================================
    // APPROACH 1: ψ-decode bilang noise estimate
    // ============================================
    std::cout << "APPROACH 1: ψ-DECODE NOISE ESTIMATE\n";
    std::cout << "=====================================\n";
    std::cout << "  Para sa m=0: ψ-decode = noise\n";
    std::cout << "  Para sa m=1: ψ-decode = offset + noise\n";
    std::cout << "  Kaya: noise = ψ-decode - m·offset\n";
    std::cout << "  At: clean_φ = φ-decode - noise\n\n";

    // ============================================
    // APPROACH 2: ADAPTIVE CORRECTION
    // ============================================
    std::cout << "APPROACH 2: ADAPTIVE CORRECTION\n";
    std::cout << "=================================\n";
    std::cout << "  Subaybayan ang noise sa bawat depth\n";
    std::cout << "  at gamitin ang trend para mag-correct\n\n";

    auto current = encrypt(1, NTL::to_ZZ(1));
    std::vector<NTL::ZZ> phi_history;
    std::vector<NTL::ZZ> psi_history;
    std::vector<int> decisions;

    std::cout << "  Depth | φ-decode | ψ-decode | Noise est | φ-clean | Decision\n";
    std::cout << "  ------|----------|----------|-----------|---------|----------\n";

    for (int i = 0; i <= 20; i++) {
        NTL::ZZ dec_phi = decode_phi(current);
        NTL::ZZ dec_psi = decode_psi(current);

        // Estimate m mula sa ψ-decode
        int m_est = (dist(dec_psi, offset) < dist(dec_psi, NTL::to_ZZ(0))) ? 1 : 0;

        // Estimate noise
        NTL::ZZ noise_est = (m_est == 1) ? 
            ((dec_psi - offset + Q) % Q) : dec_psi;

        // Clean φ-decode
        NTL::ZZ clean_phi = (dec_phi - noise_est + Q) % Q;

        // Decision mula sa clean φ
        int decision = (dist(clean_phi, NTL::to_ZZ(1)) < dist(clean_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;

        phi_history.push_back(dec_phi);
        psi_history.push_back(dec_psi);
        decisions.push_back(decision);

        std::cout << "  " << i << "     | " << dec_phi
                  << " | " << dec_psi
                  << " | " << noise_est
                  << " | " << clean_phi
                  << " | " << decision
                  << (decision == expected ? " ✓" : " ✗") << "\n";

        current = nand_op(current, current);
    }

    // Count accuracy
    int correct = 0;
    for (int i = 0; i < decisions.size(); i++) {
        int expected = (i % 2 == 0) ? 1 : 0;
        if (decisions[i] == expected) correct++;
    }
    std::cout << "\n  Accuracy: " << correct << "/" << decisions.size()
              << " = " << (100.0 * correct / decisions.size()) << "%\n\n";

    // ============================================
    // APPROACH 3: FIBONACCI-SPACED CORRECTION
    // ============================================
    std::cout << "APPROACH 3: FIBONACCI-SPACED CORRECTION\n";
    std::cout << "=========================================\n";
    std::cout << "  Ang errors ay nasa depths: 1, 2, 5, 10, 13, 18\n";
    std::cout << "  Gaps: 1, 3, 5, 3, 5\n";
    std::cout << "  Ito ba ay Fibonacci-like?\n\n";

    // Fibonacci numbers
    std::vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34};
    std::cout << "  Fibonacci: ";
    for (int f : fib) std::cout << f << " ";
    std::cout << "\n";
    std::cout << "  Error depths: 1, 2, 5, 10, 13, 18\n";
    std::cout << "  Match: 1, 2, 5, 13 ay Fibonacci!\n";
    std::cout << "  Pero 10 at 18 ay hindi...\n\n";

    // ============================================
    // APPROACH 4: COMBINED φ+ψ DECISION
    // ============================================
    std::cout << "APPROACH 4: COMBINED φ+ψ DECISION\n";
    std::cout << "===================================\n";
    std::cout << "  Gamitin ang parehong φ at ψ para sa decision\n";
    std::cout << "  Kung magkasundo sila, mataas ang confidence\n";
    std::cout << "  Kung magkaiba, mababa ang confidence\n\n";

    current = encrypt(1, NTL::to_ZZ(1));
    std::cout << "  Depth | φ-decision | ψ-decision | Combined | Confidence\n";
    std::cout << "  ------|------------|------------|----------|------------\n";

    for (int i = 0; i <= 20; i++) {
        NTL::ZZ dec_phi = decode_phi(current);
        NTL::ZZ dec_psi = decode_psi(current);

        int phi_dec = (dist(dec_phi, NTL::to_ZZ(1)) < dist(dec_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(dec_psi, offset) < dist(dec_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;

        int combined;
        std::string confidence;

        if (phi_dec == psi_dec) {
            combined = phi_dec;
            confidence = "HIGH (agree)";
        } else {
            // Kapag magkaiba, mas tiwala sa φ
            combined = phi_dec;
            confidence = "LOW (φ priority)";
        }

        std::cout << "  " << i << "     | " << phi_dec
                  << "          | " << psi_dec
                  << "          | " << combined
                  << "       | " << confidence
                  << (combined == expected ? " ✓" : " ✗") << "\n";

        current = nand_op(current, current);
    }

    return 0;
}
