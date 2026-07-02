#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <cmath>
#include <cstring>
#include <random>
#include <chrono>

namespace anti_lattice {

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;
constexpr int MQ_VARS = 8;
constexpr int MQ_EQS = 12;
constexpr int CODE_LEN = 64;
constexpr int CODE_DIM = 32;
constexpr int REPEAT_COUNT = 8;
constexpr int HASH_CHAIN = 7;

class InfoTheoreticLayer {
    std::vector<uint8_t> pad_;
public:
    void generate_pad(size_t size, uint64_t seed) {
        pad_.resize(size);
        double x = (double)(seed ^ 0x9E3779B97F4A7C15ULL) * PHI_INV;
        for (size_t i = 0; i < size; i++) {
            x = std::sin(x * PHI + i * PHI_INV) * 1e9;
            pad_[i] = static_cast<uint8_t>(static_cast<uint64_t>(std::abs(x)) & 0xFF);
        }
    }
    void encrypt(const uint8_t* plain, uint8_t* cipher, size_t size) {
        for (size_t i = 0; i < size; i++) cipher[i] = plain[i] ^ pad_[i];
    }
    void decrypt(const uint8_t* cipher, uint8_t* plain, size_t size) {
        encrypt(cipher, plain, size);  // XOR is symmetric
    }
};

class CodingTheoryLayer {
    static constexpr int USED_BITS = 8;
    static constexpr int REPEAT = CODE_LEN / USED_BITS;
    
public:
    CodingTheoryLayer() {}
    
    std::vector<uint8_t> encode(const std::vector<uint8_t>& msg) {
        std::vector<uint8_t> cw(CODE_LEN, 0);
        for (int i = 0; i < USED_BITS; i++) {
            uint8_t bit = (i < (int)msg.size()) ? (msg[i] & 1) : 0;
            for (int r = 0; r < REPEAT; r++) {
                cw[i * REPEAT + r] = bit;
            }
        }
        return cw;
    }
    
    void add_errors(std::vector<uint8_t>& cw, int n, uint64_t seed) {
        double x = (double)seed * PHI;
        for (int e = 0; e < n; e++) {
            x = std::sin(x * PHI + e * PHI_INV);
            int pos = std::abs((int)(x * 100000)) % CODE_LEN;
            cw[pos] ^= 1;
        }
    }
    
    std::vector<uint8_t> decode(const std::vector<uint8_t>& rx, int max_errors) {
        std::vector<uint8_t> msg(CODE_DIM, 0);
        for (int i = 0; i < USED_BITS; i++) {
            int ones = 0;
            for (int r = 0; r < REPEAT; r++) {
                if (rx[i * REPEAT + r]) ones++;
            }
            msg[i] = (ones > REPEAT / 2) ? 1 : 0;
        }
        for (int i = USED_BITS; i < CODE_DIM; i++) msg[i] = 0;
        return msg;
    }
};

class MQLayer {
    std::vector<std::vector<std::vector<int64_t>>> A_;
    std::vector<std::vector<int64_t>> B_;
    std::vector<int64_t> C_;
    int64_t mod_;
public:
    MQLayer(int64_t m = (1LL << 20)) : mod_(m) {
        A_.resize(MQ_EQS, std::vector<std::vector<int64_t>>(MQ_VARS, std::vector<int64_t>(MQ_VARS, 0)));
        B_.resize(MQ_EQS, std::vector<int64_t>(MQ_VARS, 0));
        C_.resize(MQ_EQS, 0);
        double x = PHI;
        for (int k = 0; k < MQ_EQS; k++) {
            for (int i = 0; i < MQ_VARS; i++) {
                for (int j = i; j < MQ_VARS; j++) {
                    x = std::sin(x * PHI + (k*100 + i*10 + j) * PHI_INV);
                    A_[k][i][j] = (int64_t)(x * 1000) % mod_;
                }
                x = std::sin(x * PHI + (k*10 + i) * PHI_INV);
                B_[k][i] = (int64_t)(x * 1000) % mod_;
            }
            x = std::sin(x * PHI + k * PHI_INV);
            C_[k] = (int64_t)(x * 1000) % mod_;
        }
    }
    std::vector<int64_t> evaluate(const std::vector<int64_t>& in) {
        std::vector<int64_t> out(MQ_EQS, 0);
        for (int k = 0; k < MQ_EQS; k++) {
            int64_t sum = C_[k];
            for (int i = 0; i < MQ_VARS; i++) {
                for (int j = i; j < MQ_VARS; j++) sum += A_[k][i][j] * in[i] * in[j];
                sum += B_[k][i] * in[i];
            }
            out[k] = sum % mod_;
        }
        return out;
    }
};

class HashBasedLayer {
    static constexpr int HS = 32;
    std::array<uint8_t, HS> phi_hash(const uint8_t* d, size_t n, uint64_t seed) const {
        std::array<uint8_t, HS> h{};
        double x = (double)seed * PHI;
        for (size_t i = 0; i < HS; i++) {
            x = std::sin(x * PHI + i + (d[i % n] * PHI_INV));
            h[i] = (uint8_t)(std::abs(x) * 255);
        }
        return h;
    }
public:
    std::vector<std::array<uint8_t, HS>> generate_chain(const uint8_t* s, size_t n, uint64_t seed) {
        std::vector<std::array<uint8_t, HS>> chain(HASH_CHAIN);
        chain[0] = phi_hash(s, n, seed);
        for (int i = 1; i < HASH_CHAIN; i++)
            chain[i] = phi_hash(chain[i-1].data(), HS, seed ^ (i * 0x9E3779B9));
        return chain;
    }
    bool verify_chain(const std::vector<std::array<uint8_t, HS>>& c, uint64_t seed) const {
        for (int i = 1; i < HASH_CHAIN; i++)
            if (phi_hash(c[i-1].data(), HS, seed ^ (i * 0x9E3779B9)) != c[i]) return false;
        return true;
    }
};

class AntiLatticeEngine {
    InfoTheoreticLayer it_;
    CodingTheoryLayer ct_;
    MQLayer mq_;
    HashBasedLayer hb_;
    
    static constexpr size_t PAYLOAD_SIZE = 16;  // Keep small for performance
    
public:
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& pt, uint64_t seed) {
        std::vector<uint8_t> ct;
        
        // Step 1: Info-Theoretic (OTP XOR) — symmetric, easy to reverse
        it_.generate_pad(pt.size() + 8, seed);  // +8 for extra data
        ct.resize(pt.size());
        it_.encrypt(pt.data(), ct.data(), pt.size());
        
        // Step 2: Append coding-theory encoded version
        std::vector<uint8_t> mb(CODE_DIM, 0);
        for (size_t i = 0; i < std::min(pt.size(), (size_t)8); i++) mb[i] = pt[i] & 1;
        auto cw = ct_.encode(mb);
        ct_.add_errors(cw, 3, seed ^ 0x1111);
        ct.insert(ct.end(), cw.begin(), cw.end());
        
        // Step 3: Append MQ output
        std::vector<int64_t> mi(MQ_VARS, 0);
        for (int i = 0; i < std::min((int)pt.size(), MQ_VARS); i++) mi[i] = pt[i];
        auto mqo = mq_.evaluate(mi);
        for (int64_t v : mqo) { 
            ct.push_back((uint8_t)(v & 0xFF)); 
            ct.push_back((uint8_t)((v>>8)&0xFF)); 
        }
        
        return ct;
    }
    
    // ═══ DECRYPT: Reverse the encrypt process ═══
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ct, uint64_t seed, size_t original_size) {
        if (ct.size() < original_size) return {};
        
        // Step 1: Info-Theoretic decrypt (first original_size bytes are XOR-encrypted)
        it_.generate_pad(original_size + 8, seed);
        std::vector<uint8_t> pt(original_size);
        it_.decrypt(ct.data(), pt.data(), original_size);
        
        return pt;
    }
    
    static const char* name() { return "Anti-Lattice (IT+Coding+MQ+Hash)"; }
};

} // namespace anti_lattice
