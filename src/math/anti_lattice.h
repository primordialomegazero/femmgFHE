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
        encrypt(cipher, plain, size);
    }
};

class CodingTheoryLayer {
    std::vector<std::vector<uint8_t>> G_;
public:
    CodingTheoryLayer() {
        G_.resize(CODE_DIM, std::vector<uint8_t>(CODE_LEN, 0));
        double x = PHI;
        for (int i = 0; i < CODE_DIM; i++)
            for (int j = 0; j < CODE_LEN; j++) {
                x = std::sin(x * PHI + (i * CODE_LEN + j) * PHI_INV);
                G_[i][j] = (std::abs(x) > 0.5) ? 1 : 0;
            }
    }
    std::vector<uint8_t> encode(const std::vector<uint8_t>& msg) {
        std::vector<uint8_t> cw(CODE_LEN, 0);
        for (int i = 0; i < CODE_DIM; i++)
            if (msg[i])
                for (int j = 0; j < CODE_LEN; j++) cw[j] ^= G_[i][j];
        return cw;
    }
    void add_errors(std::vector<uint8_t>& cw, int n, uint64_t seed) {
        double x = (double)seed * PHI;
        for (int e = 0; e < n; e++) {
            x = std::sin(x * PHI + e * PHI_INV);
            cw[std::abs((int)(x * CODE_LEN)) % CODE_LEN] ^= 1;
        }
    }
    std::vector<uint8_t> decode(std::vector<uint8_t>& rx, int) {
        std::vector<uint8_t> msg(CODE_DIM, 0);
        for (int i = 0; i < CODE_DIM; i++) {
            int ones = 0;
            for (int j = 0; j < CODE_LEN; j++) if (G_[i][j] && rx[j]) ones++;
            msg[i] = (ones > 0) ? 1 : 0;
        }
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
    std::array<uint8_t, HS> phi_hash(const uint8_t* d, size_t n, uint64_t seed) {
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
    bool verify_chain(const std::vector<std::array<uint8_t, HS>>& c, uint64_t seed) {
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
public:
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& pt, uint64_t seed) {
        std::vector<uint8_t> ct;
        it_.generate_pad(pt.size(), seed);
        ct.resize(pt.size());
        it_.encrypt(pt.data(), ct.data(), pt.size());
        std::vector<uint8_t> mb(CODE_DIM, 0);
        for (size_t i = 0; i < std::min(pt.size(), (size_t)CODE_DIM); i++) mb[i] = pt[i] & 1;
        auto cw = ct_.encode(mb);
        ct_.add_errors(cw, 3, seed ^ 0x1111);
        std::vector<int64_t> mi(MQ_VARS, 0);
        for (int i = 0; i < std::min((int)cw.size(), MQ_VARS); i++) mi[i] = cw[i];
        auto mqo = mq_.evaluate(mi);
        for (int64_t v : mqo) { ct.push_back((uint8_t)(v & 0xFF)); ct.push_back((uint8_t)((v>>8)&0xFF)); }
        return ct;
    }
    static const char* name() { return "Anti-Lattice (IT+Coding+MQ+Hash)"; }
};

} // namespace anti_lattice
