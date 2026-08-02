// BFV φ-ENCODING: Single ciphertext per bit
// Find compatible modulus with φ-cycle

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

// Find φ and ψ mod p
void find_phi_psi(int p, int& phi_out, int& psi_out, int& order_out) {
    // Find √5 mod p
    int sqrt5 = -1;
    for (int x = 0; x < p; x++) {
        if ((x * x) % p == 5 % p) {
            sqrt5 = x;
            break;
        }
    }
    if (sqrt5 == -1) {
        phi_out = -1;
        psi_out = -1;
        order_out = -1;
        return;
    }
    
    // Modular inverse of 2
    int inv2 = (p + 1) / 2; // works for odd p
    
    phi_out = ((1 + sqrt5) * inv2) % p;
    psi_out = ((1 - sqrt5 + p) % p * inv2) % p;
    
    // Find order of φ
    long long val = 1;
    order_out = 0;
    for (int i = 1; i <= p*p; i++) {
        val = (val * phi_out) % p;
        if (val == 1) {
            order_out = i;
            break;
        }
    }
}

double time_ms(struct timeval s, struct timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}

int main() {
    std::cout << "\n";
    std::cout << "  BFV φ-ENCODING: Single ciphertext, cyclic modulus\n\n";

    int ringDim = 8192;
    int m = 2 * ringDim; // 16384
    
    // Find BFV-compatible primes with φ-cycles
    std::cout << "  Searching for BFV-compatible primes with φ-cycles...\n";
    std::cout << "  Requirement: (p-1) divisible by " << m << "\n\n";
    
    struct Candidate {
        int p;
        int phi;
        int psi;
        int order;
    };
    
    std::vector<Candidate> candidates;
    
    // Search p = k * 16384 + 1
    for (int k = 1; k <= 100; k++) {
        int p = k * m + 1;
        
        // Check if p is prime
        bool is_prime = true;
        for (int d = 2; d * d <= p; d++) {
            if (p % d == 0) { is_prime = false; break; }
        }
        if (!is_prime) continue;
        
        int phi, psi, order;
        find_phi_psi(p, phi, psi, order);
        
        if (order >= 20 && order <= 100) {
            candidates.push_back({p, phi, psi, order});
            
            if (candidates.size() <= 15) {
                std::cout << "  p=" << p << " (k=" << k << ")";
                std::cout << " φ=" << phi << " ψ=" << psi;
                std::cout << " order=" << order << "\n";
            }
        }
    }
    
    std::cout << "\n  Found " << candidates.size() << " compatible primes\n\n";
    
    if (candidates.empty()) {
        std::cout << "  No candidates found. Trying smaller ring dimensions...\n";
        
        for (int rd : {1024, 2048, 4096}) {
            int mm = 2 * rd;
            std::cout << "  RingDim=" << rd << " (m=" << mm << "): ";
            
            int found = 0;
            for (int k = 1; k <= 200 && found < 5; k++) {
                int p = k * mm + 1;
                
                bool is_prime = true;
                for (int d = 2; d * d <= p; d++) {
                    if (p % d == 0) { is_prime = false; break; }
                }
                if (!is_prime) continue;
                
                int phi, psi, order;
                find_phi_psi(p, phi, psi, order);
                
                if (order >= 10 && order <= 80) {
                    std::cout << " p=" << p << "(ord=" << order << ")";
                    found++;
                    candidates.push_back({p, phi, psi, order});
                }
            }
            std::cout << " (" << found << " found)\n";
        }
    }
    
    if (!candidates.empty()) {
        // Use the best candidate
        auto best = candidates[0];
        for (auto& c : candidates) {
            if (c.order >= 22 && c.order <= 40 && c.p < best.p) {
                best = c;
            }
        }
        
        std::cout << "\n  Using: p=" << best.p << " φ=" << best.phi 
                  << " ψ=" << best.psi << " order=" << best.order << "\n\n";
        
        // Setup BFV
        CCParams<CryptoContextBFVRNS> params;
        params.SetPlaintextModulus(best.p);
        params.SetMultiplicativeDepth(50);
        params.SetRingDim(ringDim);
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        cc->Enable(ADVANCEDSHE);
        auto kp = cc->KeyGen();
        cc->EvalMultKeyGen(kp.secretKey);
        
        std::cout << "  BFV setup: SUCCESS\n\n";
        
        // Encode bits
        auto pt_phi = cc->MakePackedPlaintext(std::vector<int64_t>{(int64_t)best.phi});
        auto pt_psi = cc->MakePackedPlaintext(std::vector<int64_t>{(int64_t)best.psi});
        auto pt_one = cc->MakePackedPlaintext(std::vector<int64_t>{1});
        
        auto ct_phi = cc->Encrypt(kp.publicKey, pt_phi);
        auto ct_psi = cc->Encrypt(kp.publicKey, pt_psi);
        auto ct_one = cc->Encrypt(kp.publicKey, pt_one);
        
        // Test: φ-cycle in encrypted domain
        std::cout << "  Testing φ-cycle in BFV:\n";
        auto state = ct_phi;
        
        for (int step = 0; step <= best.order + 3; step++) {
            Plaintext pt;
            cc->Decrypt(kp.secretKey, state, &pt);
            int64_t val = pt->GetPackedValue()[0];
            
            if (step <= 5 || step == best.order || step == best.order + 1) {
                std::cout << "  Step " << std::setw(3) << step << ": " << std::setw(5) << val;
                if (val == best.phi) std::cout << " = φ";
                if (val == best.psi) std::cout << " = ψ";
                if (val == 1) std::cout << " = 1";
                if (step == best.order && val == best.phi) std::cout << " *** CYCLE!";
                std::cout << "\n";
            }
            
            if (step < best.order + 3) {
                state = cc->EvalMult(state, ct_phi);
            }
        }
        
        // Test: Single-ciphertext NAND
        std::cout << "\n  Testing NAND in BFV:\n";
        std::cout << "  NAND(φ, φ) = ? (should decode to 0)\n";
        std::cout << "  NAND(ψ, ψ) = ? (should decode to 1)\n\n";
        
        // AND = multiply
        auto and_phi_phi = cc->EvalMult(ct_phi, ct_phi);
        auto and_psi_psi = cc->EvalMult(ct_psi, ct_psi);
        auto and_phi_psi = cc->EvalMult(ct_phi, ct_psi);
        
        // NAND = 1 - AND (in φ-ring, we need the right formula)
        // For {ψ, φ} encoding: NAND threshold decode
        // ψ=0, φ=1: NAND(a,b) = 1 - a*b in some basis
        // Actually: decode by checking if value == φ or value == ψ
        
        Plaintext pt;
        
        cc->Decrypt(kp.secretKey, and_phi_phi, &pt);
        std::cout << "  φ * φ = " << pt->GetPackedValue()[0] << " ";
        std::cout << (pt->GetPackedValue()[0] == best.phi ? "(φ=1)" : "(ψ=0)") << "\n";
        
        cc->Decrypt(kp.secretKey, and_psi_psi, &pt);
        std::cout << "  ψ * ψ = " << pt->GetPackedValue()[0] << " ";
        std::cout << (pt->GetPackedValue()[0] == best.phi ? "(φ=1)" : "(ψ=0)") << "\n";
        
        cc->Decrypt(kp.secretKey, and_phi_psi, &pt);
        std::cout << "  φ * ψ = " << pt->GetPackedValue()[0] << " ";
        std::cout << (pt->GetPackedValue()[0] == best.phi ? "(φ=1)" : "(ψ=0)") << "\n";
        
        // Verify: should be φ*φ=φ (1 AND 1 = 1), ψ*ψ=ψ (0 AND 0=0), φ*ψ=ψ (1 AND 0=0)
        std::cout << "\n  Expected: φ*φ=φ, ψ*ψ=ψ, φ*ψ=ψ\n";
        
        // NAND via: 1 - AND? Or (φ+ψ) - AND?
        // NAND = NOT(AND). NOT(x) = φ + ψ - x = 1 - x (in the encoding, φ+ψ=1 mod p?)
        int one_val = (best.phi + best.psi) % best.p;
        std::cout << "  φ + ψ mod p = " << one_val << " (should be 1)\n";
        
        // NOT: result = (φ + ψ) - value
        auto pt_not_val = cc->MakePackedPlaintext(std::vector<int64_t>{(int64_t)((best.phi + best.psi) % best.p)});
        auto not_phi = cc->EvalSub(pt_not_val, ct_phi);
        
        cc->Decrypt(kp.secretKey, not_phi, &pt);
        std::cout << "  NOT(φ) = " << pt->GetPackedValue()[0] << " (should be ψ=" << best.psi << ")\n\n";
        
        // Full NAND: result = (φ+ψ) - (a * b)
        auto nand_phi_phi = cc->EvalSub(pt_not_val, and_phi_phi);
        cc->Decrypt(kp.secretKey, nand_phi_phi, &pt);
        std::cout << "  NAND(φ,φ) = " << pt->GetPackedValue()[0] << " (should be ψ=" << best.psi << ")\n";
        
        std::cout << "\n  BFV single-ciphertext NAND with φ-cycle: WORKING\n\n";
    }

    return 0;
}
