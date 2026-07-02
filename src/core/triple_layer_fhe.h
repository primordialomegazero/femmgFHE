#pragma once
#include "femmg_operations.h"
#include "../math/phi_polynomial.h"
#include "../math/anti_lattice.h"
#include <cstring>
#include <chrono>

class TripleLayerFHE {
private:
    FEmmgFHE chaos_fhe_;
    static constexpr int POLY_N = 64;
    
    banach::NDimCiphertext extract_ct(const std::vector<uint8_t>& obfuscated, uint64_t seed) {
        anti_lattice::InfoTheoreticLayer it;
        it.generate_pad(POLY_N * 8, seed);
        std::vector<uint8_t> poly_bytes(POLY_N * 8);
        it.decrypt(obfuscated.data(), poly_bytes.data(), POLY_N * 8);
        banach::NDimCiphertext ct;
        std::memcpy(&ct, poly_bytes.data(), sizeof(ct));
        return ct;
    }
    
    std::vector<uint8_t> pack_ct(const banach::NDimCiphertext& ct, uint64_t seed) {
        std::vector<uint8_t> ct_bytes(sizeof(ct));
        std::memcpy(ct_bytes.data(), &ct, sizeof(ct));
        while (ct_bytes.size() < (size_t)(POLY_N * 8)) ct_bytes.push_back(0);
        anti_lattice::InfoTheoreticLayer it;
        it.generate_pad(POLY_N * 8, seed);
        std::vector<uint8_t> obfuscated(POLY_N * 8);
        it.encrypt(ct_bytes.data(), obfuscated.data(), POLY_N * 8);
        return obfuscated;
    }
    
public:
    TripleLayerFHE() = default;
    
    std::vector<uint8_t> encrypt(int64_t plaintext, uint64_t seed = 0) {
        if (seed == 0) seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        return pack_ct(chaos_fhe_.encrypt(plaintext), seed);
    }
    
    int64_t decrypt(const std::vector<uint8_t>& ct, uint64_t seed = 0) {
        if (seed == 0 || ct.size() < (size_t)(POLY_N * 8)) return 0;
        return chaos_fhe_.decrypt(extract_ct(ct, seed));
    }
    
    std::vector<uint8_t> add(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b, uint64_t seed = 0) {
        if (seed == 0) return {};
        auto cta = extract_ct(a, seed);
        auto ctb = extract_ct(b, seed);
        return pack_ct(chaos_fhe_.add(cta, ctb), seed);
    }
    
    std::vector<uint8_t> multiply(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b, uint64_t seed = 0) {
        if (seed == 0) return {};
        auto cta = extract_ct(a, seed);
        auto ctb = extract_ct(b, seed);
        return pack_ct(chaos_fhe_.multiply(cta, ctb), seed);
    }
    
    static const char* name() { return "Triple-Layer Fibonacci-Grounded FHE"; }
};
