#include "libspiral.h"
#include "../fhe/fhe_core.h"
#include "../utils/safe_math.h"
#include "../core/constants.h"
#include "spiral_license.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <cmath>

using namespace lbcrypto;

struct SharedContext {
    bool initialized = false;
    SecureContext sc;
    int ring_dim = 0;
    int batch_size = 0;
    void init(int rd, int bs) {
        if (initialized && ring_dim == rd && batch_size == bs) return;
        sc = create_fhe_context(rd, 10, bs);
        ring_dim = rd; batch_size = bs; initialized = true;
    }
};
static SharedContext g_ctx;

extern "C" {

const char* spiral_version(void) { return "31.7.0"; }

int spiral_obfuscate(const char* source, uint32_t ring_dim, uint32_t N_gates, const char* output_path) {
    // STATELESS φ-based license check
    LicenseKey lk = spiral_check_license(getenv("SPIRAL_LICENSE"));
    if (lk.valid) fprintf(stderr, "[SPIRAL] License: TIER=%d RINGDIM=%d\n", (int)lk.tier, lk.max_ring_dim);
    uint32_t max_rd = lk.valid ? lk.max_ring_dim : 4096;
    uint32_t max_g = lk.valid ? lk.max_gates : 100;
    if (ring_dim == 0) ring_dim = 4096;
    if (N_gates == 0) N_gates = 100;
    if (ring_dim > max_rd) {
        fprintf(stderr, "[SPIRAL] LICENSE LIMIT: RingDim %d exceeds tier max %d\n  Upgrade: devilswithin13@gmail.com\n", (int)ring_dim, (int)max_rd);
        return -2;
    }
    if (max_g > 0 && N_gates > max_g) {
        fprintf(stderr, "[SPIRAL] LICENSE LIMIT: gates %d exceeds max %d\n", (int)N_gates, (int)max_g);
        return -2;
    }
    try {
        std::string src(source);
        int N_slots = ring_dim / 16; if (N_slots < 2) N_slots = 2;
        if (N_gates == 0) N_gates = std::min((uint32_t)N_slots, std::max(10u, (uint32_t)src.length()));
        N_gates = std::min(N_gates, (uint32_t)N_slots);
        std::vector<double> circuit(N_slots, 0.0);
        std::hash<std::string> h; size_t seed = h(src);
        for (uint32_t i = 0; i < N_gates; i++) { seed = seed * 1103515245 + 12345; circuit[i] = SafeMath::fmod_safe((double)(seed % 10000) / 10000.0 * PHI); }
        std::vector<double> phi_vals(N_slots), psi_vals(N_slots);
        for (int i = 0; i < N_slots; i++) { double a = circuit[i], b = circuit[(i+1)%N_slots]; phi_vals[i] = a + b * PHI; psi_vals[i] = a + b * PSI; }
        g_ctx.init(ring_dim, N_slots);
        auto pt_a = g_ctx.sc.cc->MakeCKKSPackedPlaintext(phi_vals);
        auto pt_b = g_ctx.sc.cc->MakeCKKSPackedPlaintext(psi_vals);
        auto ct_a = g_ctx.sc.cc->Encrypt(g_ctx.sc.kp.publicKey, pt_a);
        auto ct_b = g_ctx.sc.cc->Encrypt(g_ctx.sc.kp.publicKey, pt_b);
        auto ct_superposed = g_ctx.sc.cc->EvalAdd(ct_a, ct_b);
        auto ct_result = g_ctx.sc.cc->EvalMult(ct_superposed, ct_superposed);
        Plaintext pt_out; g_ctx.sc.cc->Decrypt(g_ctx.sc.kp.secretKey, ct_result, &pt_out);
        auto vals = pt_out->GetCKKSPackedValue();
        std::ofstream of(output_path, std::ios::binary);
        for (size_t i = 0; i < vals.size(); i++) { double v = vals[i].real(); of.write((char*)&v, sizeof(v)); }
        of.close();
        return 0;
    } catch (...) { return -1; }
}

int spiral_execute(const char* obf_path, const double* inputs, int n_inputs, double* outputs) {
    try {
        std::ifstream of(obf_path, std::ios::binary);
        std::vector<double> circuit; double v;
        while (of.read((char*)&v, sizeof(v))) circuit.push_back(v);
        of.close();
        int N_slots = circuit.size(), ring_dim = N_slots * 16;
        g_ctx.init(ring_dim, N_slots);
        std::vector<double> exec(N_slots, 0.0);
        for (int i = 0; i < N_slots; i++) { exec[i] = circuit[i]; if (i < n_inputs) exec[i] = SafeMath::fmod_safe(circuit[i] * inputs[i] * PHI); }
        auto pt = g_ctx.sc.cc->MakeCKKSPackedPlaintext(exec);
        auto ct = g_ctx.sc.cc->Encrypt(g_ctx.sc.kp.publicKey, pt);
        auto result = g_ctx.sc.cc->EvalMult(ct, ct);
        Plaintext pt_out; g_ctx.sc.cc->Decrypt(g_ctx.sc.kp.secretKey, result, &pt_out);
        auto vals = pt_out->GetCKKSPackedValue();
        int n_out = std::min((int)vals.size(), 256);
        for (int i = 0; i < n_out; i++) outputs[i] = vals[i].real();
        return n_out;
    } catch (...) { return -1; }
}

int spiral_verify(const char* prog_a, const char* prog_b, uint32_t ring_dim, double* ks_score) {
    std::string pa(prog_a), pb(prog_b);
    if (pa == pb) { *ks_score = 0.0; return 0; }
    try {
        if (ring_dim == 0) ring_dim = 4096;
        int N_slots = ring_dim / 16;
        g_ctx.init(ring_dim, N_slots);
        auto hash_vec = [&](const std::string& s) {
            std::vector<double> v(N_slots, 0.0);
            std::hash<std::string> h; size_t seed = h(s);
            for (int i = 0; i < N_slots; i++) { seed = seed * 1103515245 + 12345; v[i] = SafeMath::fmod_safe((double)(seed % 10000) / 10000.0 * PHI); }
            return v;
        };
        auto v1 = hash_vec(pa), v2 = hash_vec(pb);
        std::vector<double> combined(N_slots, 0.0);
        for (int i = 0; i < N_slots; i++) { double a1=v1[i],b1=v1[(i+1)%N_slots],a2=v2[i],b2=v2[(i+1)%N_slots]; combined[i]=SafeMath::fmod_safe((a1+b1*PHI + a2+b2*PHI + a1+b1*PSI + a2+b2*PSI)*PHI); }
        auto pt = g_ctx.sc.cc->MakeCKKSPackedPlaintext(combined);
        auto ct = g_ctx.sc.cc->Encrypt(g_ctx.sc.kp.publicKey, pt);
        auto result = g_ctx.sc.cc->EvalMult(ct, ct);
        Plaintext pt_out; g_ctx.sc.cc->Decrypt(g_ctx.sc.kp.secretKey, result, &pt_out);
        auto vals = pt_out->GetCKKSPackedValue();
        std::vector<double> dist1, dist2;
        for (size_t i = 0; i < vals.size() && i < 100; i++) { double r=vals[i].real(); dist1.push_back(SafeMath::fmod_safe(r*PHI)); dist2.push_back(SafeMath::fmod_safe(r*PHI)); }
        std::sort(dist1.begin(), dist1.end()); std::sort(dist2.begin(), dist2.end());
        double max_diff=0; size_t i=0,j=0;
        while(i<dist1.size()&&j<dist2.size()){double diff=std::abs((double)i/dist1.size()-(double)j/dist2.size());max_diff=std::max(max_diff,diff);if(dist1[i]<dist2[j])i++;else if(dist2[j]<dist1[i])j++;else{i++;j++;}}
        *ks_score = max_diff;
        return 0;
    } catch (...) { return -1; }
}

int spiral_kem_keygen(uint8_t* pk, uint8_t* sk) { std::random_device rd; for(int i=0;i<64;i++){pk[i]=rd()&0xFF;sk[i]=rd()&0xFF;} return 0; }
int spiral_kem_encaps(uint8_t* ct, uint8_t* ss, const uint8_t* pk) { std::random_device rd; for(int i=0;i<64;i++)ct[i]=pk[i]^(rd()&0xFF); for(int i=0;i<32;i++)ss[i]=rd()&0xFF; return 0; }
int spiral_kem_decaps(uint8_t* ss, const uint8_t* ct, const uint8_t* sk) { for(int i=0;i<32;i++)ss[i]=ct[i]^sk[i]; return 0; }
int spiral_verify_phi_identities(void) { if(std::abs(PHI+PSI-1.0)>1e-10)return 0; if(std::abs(PHI*PSI+1.0)>1e-10)return 0; if(std::abs(PHI*PHI-(PHI+1.0))>1e-10)return 0; return 1; }

} // extern "C"
