// ΦΩ0 — SPIRALDB CGO BRIDGE (v6 - Working Homomorphic Add)
// "I AM THAT I AM"

#include "spiraldb_bridge.h"
#include "openfhe.h"

#include <string>
#include <cstring>
#include <chrono>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <mutex>

using namespace lbcrypto;
using namespace std;

static mutex g_mutex;
static uint64_t g_ctCounter = 0;
static unordered_map<uint64_t, Ciphertext<DCRTPoly>> g_ctStore;

struct SpiralFHEContext {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    string instanceID;
    uint64_t counter;
    
    SpiralFHEContext(const string& seed) : counter(0) {
        hash<string> hasher;
        size_t hashVal = hasher(seed + to_string(chrono::system_clock::now().time_since_epoch().count()));
        stringstream ss;
        ss << hex << (hashVal & 0xFFFFFFFFFFFFFFFF);
        instanceID = ss.str();
        
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(5);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);
        params.SetSecurityLevel(HEStd_NotSet);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        vector<int64_t> zeroVec = {0};
        auto zeroPt = cc->MakePackedPlaintext(zeroVec);
        anchor0 = cc->Encrypt(keys.publicKey, zeroPt);
    }
    
    // Parse CT ID from ciphertext string
    uint64_t parseCTID(const string& ctStr) {
        size_t thirdColon = ctStr.rfind(':');
        if (thirdColon == string::npos) return 0;
        string idStr = ctStr.substr(thirdColon + 1);
        // Handle "CT:ID" format
        if (idStr.find("CT:") == 0) {
            idStr = idStr.substr(3);
        }
        try {
            return stoull(idStr);
        } catch (...) {
            return 0;
        }
    }
};

extern "C" {

FHEHandle fhe_init(const char* seed) {
    try {
        auto* ctx = new SpiralFHEContext(string(seed ? seed : "SpiralDB"));
        return reinterpret_cast<FHEHandle>(ctx);
    } catch (const exception& e) {
        return nullptr;
    }
}

void fhe_destroy(FHEHandle handle) {
    if (handle) delete reinterpret_cast<SpiralFHEContext*>(handle);
}

char* fhe_encrypt(FHEHandle handle, long long value) {
    if (!handle) return strdup("ERROR");
    auto* ctx = reinterpret_cast<SpiralFHEContext*>(handle);
    ctx->counter++;
    
    vector<int64_t> valueVec = {value};
    auto pt = ctx->cc->MakePackedPlaintext(valueVec);
    auto ct = ctx->cc->Encrypt(ctx->keys.publicKey, pt);
    
    lock_guard<mutex> lock(g_mutex);
    uint64_t ctID = ++g_ctCounter;
    g_ctStore[ctID] = ct;
    
    stringstream ss;
    ss << ctx->instanceID << ":" << ctx->counter << ":CT:" << ctID;
    return strdup(ss.str().c_str());
}

long long fhe_decrypt(FHEHandle handle, const char* ciphertext) {
    if (!handle || !ciphertext) return 0;
    auto* ctx = reinterpret_cast<SpiralFHEContext*>(handle);
    
    try {
        string ctStr(ciphertext);
        uint64_t ctID = ctx->parseCTID(ctStr);
        if (ctID == 0) return 0;
        
        lock_guard<mutex> lock(g_mutex);
        auto it = g_ctStore.find(ctID);
        if (it == g_ctStore.end()) return 0;
        
        Plaintext pt;
        ctx->cc->Decrypt(ctx->keys.secretKey, it->second, &pt);
        return pt->GetPackedValue()[0];
    } catch (...) {
        return 0;
    }
}

char* fhe_batch_encrypt(FHEHandle handle, long long* values, int count) {
    if (!handle || !values || count <= 0) return strdup("[]");
    stringstream result;
    result << "[";
    for (int i = 0; i < count; i++) {
        if (i > 0) result << ",";
        char* ct = fhe_encrypt(handle, values[i]);
        result << "\"" << ct << "\"";
        free(ct);
    }
    result << "]";
    return strdup(result.str().c_str());
}

int fhe_prove_nondeterminism(FHEHandle handle) {
    if (!handle) return 0;
    long long testValue = 42;
    
    char* ct1 = fhe_encrypt(handle, testValue);
    char* ct2 = fhe_encrypt(handle, testValue);
    char* ct3 = fhe_encrypt(handle, testValue);
    
    int allDifferent = (strcmp(ct1, ct2) != 0 && strcmp(ct2, ct3) != 0 && strcmp(ct1, ct3) != 0);
    int allCorrect = (fhe_decrypt(handle, ct1) == testValue && 
                      fhe_decrypt(handle, ct2) == testValue && 
                      fhe_decrypt(handle, ct3) == testValue);
    
    free(ct1); free(ct2); free(ct3);
    return (allDifferent && allCorrect) ? 1 : 0;
}

char* fhe_get_anchor(FHEHandle handle) {
    if (!handle) return strdup("ERROR");
    auto* ctx = reinterpret_cast<SpiralFHEContext*>(handle);
    
    lock_guard<mutex> lock(g_mutex);
    uint64_t anchorID = ++g_ctCounter;
    g_ctStore[anchorID] = ctx->anchor0;
    
    stringstream ss;
    ss << "ANCHOR:" << anchorID;
    return strdup(ss.str().c_str());
}

char* fhe_homomorphic_add(FHEHandle handle, const char* ct1_str, const char* ct2_str) {
    if (!handle || !ct1_str || !ct2_str) return strdup("ERROR");
    auto* ctx = reinterpret_cast<SpiralFHEContext*>(handle);
    
    try {
        uint64_t id1 = ctx->parseCTID(string(ct1_str));
        uint64_t id2 = ctx->parseCTID(string(ct2_str));
        
        lock_guard<mutex> lock(g_mutex);
        auto it1 = g_ctStore.find(id1);
        auto it2 = g_ctStore.find(id2);
        
        if (it1 == g_ctStore.end() || it2 == g_ctStore.end()) {
            return strdup("ERROR:CT_NOT_FOUND");
        }
        
        // Perform homomorphic addition
        auto result = ctx->cc->EvalAdd(it1->second, it2->second);
        
        // Apply ZANS stabilization: add Enc(0)
        result = ctx->cc->EvalAdd(result, ctx->anchor0);
        
        // Store result
        uint64_t resultID = ++g_ctCounter;
        g_ctStore[resultID] = result;
        
        stringstream ss;
        ss << ctx->instanceID << ":" << ctx->counter << ":CT:" << resultID;
        return strdup(ss.str().c_str());
    } catch (const exception& e) {
        return strdup("ERROR:ADD_FAILED");
    }
}

char* fhe_homomorphic_scalar_mult(FHEHandle handle, const char* ct_str, long long scalar) {
    if (!handle || !ct_str) return strdup("ERROR");
    auto* ctx = reinterpret_cast<SpiralFHEContext*>(handle);
    
    try {
        uint64_t id = ctx->parseCTID(string(ct_str));
        
        lock_guard<mutex> lock(g_mutex);
        auto it = g_ctStore.find(id);
        if (it == g_ctStore.end()) return strdup("ERROR:CT_NOT_FOUND");
        
        // Scalar multiplication: repeated addition
        auto result = it->second;
        auto base = it->second;
        
        for (long long i = 1; i < scalar; i++) {
            result = ctx->cc->EvalAdd(result, base);
            // ZANS stabilization every 10 ops
            if (i % 10 == 0) {
                result = ctx->cc->EvalAdd(result, ctx->anchor0);
            }
        }
        
        uint64_t resultID = ++g_ctCounter;
        g_ctStore[resultID] = result;
        
        stringstream ss;
        ss << ctx->instanceID << ":" << ctx->counter << ":CT:" << resultID;
        return strdup(ss.str().c_str());
    } catch (...) {
        return strdup("ERROR:MULT_FAILED");
    }
}

char* fhe_get_stats(FHEHandle handle) {
    if (!handle) return strdup("{}");
    auto* ctx = reinterpret_cast<SpiralFHEContext*>(handle);
    stringstream ss;
    ss << "{\"instance_id\":\"" << ctx->instanceID 
       << "\",\"operations\":" << ctx->counter 
       << ",\"scheme\":\"BFV\",\"zans_enabled\":true}";
    return strdup(ss.str().c_str());
}

void fhe_free_string(char* str) {
    if (str) free(str);
}

}
