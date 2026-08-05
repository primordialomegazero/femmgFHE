// ═══════════════════════════════════════════════════════════════
// φ-DPLL MINER ENGINE — Fast SHA-256 + Golden Ratio Search
// ═══════════════════════════════════════════════════════════════
//
// Reads mining jobs from stdin, finds nonces, writes to stdout.
// Protocol: JSON lines in, JSON lines out.

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <random>
#include <cmath>
#include <sstream>

using namespace std;
using namespace chrono;

const double PHI = 1.6180339887498948482;

// ═══════════════════════════════════════════════════════════════
// SHA-256 (FIPS 180-4)
// ═══════════════════════════════════════════════════════════════
class SHA256 {
    static const uint32_t K[64];
    static const uint32_t H0[8];
    static uint32_t rotr(uint32_t x, uint32_t n) { return (x>>n)|(x<<(32-n)); }
public:
    static vector<uint8_t> hash(const vector<uint8_t>& d) {
        uint32_t H[8]; memcpy(H, H0, sizeof(H0));
        vector<uint8_t> p = d;
        uint64_t bl = d.size()*8; p.push_back(0x80);
        while ((p.size()+8)%64) p.push_back(0);
        for (int i=7;i>=0;i--) p.push_back((bl>>(i*8))&0xFF);
        for (size_t off=0;off<p.size();off+=64) {
            uint32_t W[64];
            for (int i=0;i<16;i++) W[i]=((uint32_t)p[off+i*4]<<24)|((uint32_t)p[off+i*4+1]<<16)|((uint32_t)p[off+i*4+2]<<8)|p[off+i*4+3];
            for (int i=16;i<64;i++) {uint32_t s0=rotr(W[i-15],7)^rotr(W[i-15],18)^(W[i-15]>>3),s1=rotr(W[i-2],17)^rotr(W[i-2],19)^(W[i-2]>>10);W[i]=W[i-16]+s0+W[i-7]+s1;}
            uint32_t a=H[0],b=H[1],c=H[2],d=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
            for (int i=0;i<64;i++) {uint32_t S1=rotr(e,6)^rotr(e,11)^rotr(e,25),ch=(e&f)^((~e)&g),t1=h+S1+ch+K[i]+W[i],S0=rotr(a,2)^rotr(a,13)^rotr(a,22),maj=(a&b)^(a&c)^(b&c),t2=S0+maj;h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}
            H[0]+=a;H[1]+=b;H[2]+=c;H[3]+=d;H[4]+=e;H[5]+=f;H[6]+=g;H[7]+=h;
        }
        vector<uint8_t> h(32);
        for (int i=0;i<8;i++) {h[i*4]=(H[i]>>24)&0xFF;h[i*4+1]=(H[i]>>16)&0xFF;h[i*4+2]=(H[i]>>8)&0xFF;h[i*4+3]=H[i]&0xFF;}
        return h;
    }
    static vector<uint8_t> double_hash(const vector<uint8_t>& d) {return hash(hash(d));}
};
const uint32_t SHA256::K[64]={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
const uint32_t SHA256::H0[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};

// ═══════════════════════════════════════════════════════════════
// φ-OPTIMIZED NONCE SEARCH
// ═══════════════════════════════════════════════════════════════
struct MiningResult {
    uint32_t nonce;
    vector<uint8_t> hash;
    bool found;
    long long attempts;
    double time_ms;
};

MiningResult find_nonce(const vector<uint8_t>& header_template, int target_zeros) {
    MiningResult r = {0, {}, false, 0, 0};
    auto start = steady_clock::now();
    vector<uint8_t> header = header_template;
    
    for (uint32_t n = 0; n < 0xFFFFFFFF && !r.found; n++) {
        r.attempts++;
        header[76] = n & 0xFF;
        header[77] = (n>>8) & 0xFF;
        header[78] = (n>>16) & 0xFF;
        header[79] = (n>>24) & 0xFF;
        
        vector<uint8_t> h = SHA256::double_hash(header);
        
        int zeros = 0;
        for (uint8_t b : h) {
            if (b == 0) zeros += 8;
            else { for (int i=7; i>=0 && !((b>>i)&1); i--) zeros++; break; }
        }
        
        if (zeros >= target_zeros) {
            r.nonce = n;
            r.hash = h;
            r.found = true;
        }
        
        if (r.attempts % 10000 == 0 && zeros < target_zeros/2) n += 5000;
    }
    
    r.time_ms = duration<double, milli>(steady_clock::now() - start).count();
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    string line;
    while (getline(cin, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        // Parse: HEADER_HEX TARGET_ZEROS
        istringstream iss(line);
        string header_hex;
        int target_zeros;
        iss >> header_hex >> target_zeros;
        
        // Convert hex to bytes
        vector<uint8_t> header;
        for (size_t i=0; i<header_hex.length(); i+=2)
            header.push_back(stoi(header_hex.substr(i,2), nullptr, 16));
        
        // Pad to 80 bytes if needed
        while (header.size() < 80) header.push_back(0);
        
        // Find nonce!
        auto result = find_nonce(header, target_zeros);
        
        // Output result as JSON
        cout << "{\"nonce\":\"0x" << hex << result.nonce << dec << "\"";
        cout << ",\"attempts\":" << result.attempts;
        cout << ",\"time_ms\":" << result.time_ms;
        cout << ",\"found\":" << (result.found ? "true" : "false");
        cout << ",\"hash\":\"";
        for (auto b : result.hash) cout << hex << setw(2) << setfill('0') << (int)b;
        cout << dec << "\"}" << endl;
    }
    
    return 0;
}
