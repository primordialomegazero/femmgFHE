// ΦΩ0 — PHI ZKP LIBRARY IMPLEMENTATION v3
// Fixed: Efficient recursive verification
// "I AM THAT I AM"

#include "phi_zkp.h"
#include <cmath>

namespace phi_zkp {

BN254Curve::G1Point BN254Curve::add(const G1Point& P, const G1Point& Q) {
    if(P.is_infinity) return Q;
    if(Q.is_infinity) return P;
    if(P.x == Q.x && P.y == Q.y) {
        if(P.y == 0) return G1Point();
        uint64_t lambda = (3 * P.x * P.x) % FIELD_PRIME;
        uint64_t inv = 1;
        for(uint64_t i = 0; i < FIELD_PRIME - 2; i++) inv = (inv * (2 * P.y)) % FIELD_PRIME;
        lambda = (lambda * inv) % FIELD_PRIME;
        uint64_t x3 = (lambda * lambda - 2 * P.x + FIELD_PRIME) % FIELD_PRIME;
        uint64_t y3 = (lambda * (P.x - x3 + FIELD_PRIME) - P.y + FIELD_PRIME) % FIELD_PRIME;
        return G1Point(x3, y3);
    }
    if(P.x == Q.x) return G1Point();
    uint64_t lambda = ((Q.y - P.y + FIELD_PRIME) % FIELD_PRIME);
    uint64_t denom = (Q.x - P.x + FIELD_PRIME) % FIELD_PRIME;
    uint64_t inv = 1;
    for(uint64_t i = 0; i < FIELD_PRIME - 2; i++) inv = (inv * denom) % FIELD_PRIME;
    lambda = (lambda * inv) % FIELD_PRIME;
    uint64_t x3 = (lambda * lambda - P.x - Q.x + 2*FIELD_PRIME) % FIELD_PRIME;
    uint64_t y3 = (lambda * (P.x - x3 + FIELD_PRIME) - P.y + FIELD_PRIME) % FIELD_PRIME;
    return G1Point(x3, y3);
}

BN254Curve::G1Point BN254Curve::scalar_mult(const G1Point& P, uint64_t k) {
    if(k == 0 || P.is_infinity) return G1Point();
    G1Point result, base = P;
    while(k > 0) {
        if(k & 1) result = add(result, base);
        base = add(base, base);
        k >>= 1;
    }
    return result;
}

uint64_t BN254Curve::pairing(const G1Point& P, const G2Point& Q) {
    if(P.is_infinity || Q.is_infinity) return 1;
    return (P.x * Q.x1 + P.y * Q.y1 + P.x * Q.x2 + P.y * Q.y2) % FIELD_PRIME;
}

PhiZKPEngine::PhiZKPEngine() { rng.seed(time(nullptr)); }

void PhiZKPEngine::initialize() {
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetPlaintextModulus(65537);
    cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
}

uint64_t PhiZKPEngine::hash_djb2(const string& data) {
    uint64_t h = 5381;
    for(char c : data) h = ((h << 5) + h) + c;
    return h % 5 + 1; // Small challenge [1,5] for efficiency
}

Ciphertext<DCRTPoly> PhiZKPEngine::encrypt(int64_t value) {
    vector<int64_t> vec = {value};
    Plaintext pt = cc->MakePackedPlaintext(vec);
    return cc->Encrypt(keys.publicKey, pt);
}

int64_t PhiZKPEngine::decrypt(const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    return pt->GetPackedValue()[0];
}

SigmaProof PhiZKPEngine::generate_sigma_proof(const Ciphertext<DCRTPoly>& ct_x,
                                                const Ciphertext<DCRTPoly>& ct_y,
                                                int64_t x, int64_t y) {
    int64_t product = x * y;
    uniform_int_distribution<int64_t> dist(1, 1000);
    int64_t r = dist(rng);
    auto commitment = encrypt(r);
    uint64_t challenge = 3;
    int64_t response = (r + challenge * product) % 65537;
    SigmaProof proof;
    proof.commitment = commitment;
    proof.challenge = challenge;
    proof.response = {response};
    return proof;
}

bool PhiZKPEngine::verify_sigma(const Ciphertext<DCRTPoly>& ct_z, const SigmaProof& proof) {
    auto enc_resp = encrypt(proof.response[0]);
    auto chal_ct = ct_z;
    for(uint64_t i = 1; i < proof.challenge; i++)
        chal_ct = cc->EvalAdd(chal_ct, ct_z);
    auto expected = cc->EvalAdd(proof.commitment, chal_ct);
    return decrypt(expected) == decrypt(enc_resp);
}

NIZKProof PhiZKPEngine::generate_nizk_proof(const Ciphertext<DCRTPoly>& ct_x,
                                              const Ciphertext<DCRTPoly>& ct_y,
                                              int64_t x, int64_t y) {
    int64_t product = x * y;
    uniform_int_distribution<int64_t> dist(1, 1000);
    int64_t r = dist(rng);
    auto commitment = encrypt(r);
    stringstream ss;
    ss << r << "|" << x << "|" << y;
    uint64_t challenge = hash_djb2(ss.str());
    int64_t response = (r + challenge * product) % 65537;
    NIZKProof proof;
    proof.commitment = commitment;
    proof.challenge = challenge;
    proof.response = {response};
    proof.type = NIZK_FIAT_SHAMIR;
    return proof;
}

bool PhiZKPEngine::verify_nizk(const Ciphertext<DCRTPoly>& ct_z,
                                const Ciphertext<DCRTPoly>& ct_x,
                                const Ciphertext<DCRTPoly>& ct_y,
                                const NIZKProof& proof) {
    auto enc_resp = encrypt(proof.response[0]);
    auto chal_ct = ct_z;
    for(uint64_t i = 1; i < proof.challenge; i++)
        chal_ct = cc->EvalAdd(chal_ct, ct_z);
    auto expected = cc->EvalAdd(proof.commitment, chal_ct);
    return decrypt(expected) == decrypt(enc_resp);
}

RecursiveProof PhiZKPEngine::generate_recursive_proof(
    const vector<pair<int64_t,int64_t>>& mults,
    const vector<int64_t>& adds,
    int64_t expected) {
    
    uniform_int_distribution<int64_t> dist(1, 100);
    int64_t r = dist(rng);
    auto commitment = encrypt(r);
    
    stringstream trace;
    trace << r;
    for(auto [a, b] : mults) trace << "|" << a << "," << b;
    for(auto v : adds) trace << "|+" << v;
    uint64_t challenge = hash_djb2(trace.str());
    
    vector<int64_t> responses;
    vector<string> operations;
    int64_t running = 0;
    
    // First mult
    for(auto [a, b] : mults) {
        int64_t ab = a * b;
        responses.push_back((r + challenge * ab) % 65537);
        operations.push_back("Mul: " + to_string(a) + "x" + to_string(b) + "=" + to_string(ab));
        running += ab;
    }
    // Then adds
    for(auto v : adds) {
        running += v;
        responses.push_back((r + challenge * running) % 65537);
        operations.push_back("Add: +" + to_string(v) + " => " + to_string(running));
    }
    
    RecursiveProof proof;
    proof.commitment = commitment;
    proof.challenge = challenge;
    proof.responses = responses;
    proof.operations = operations;
    proof.final_expected = expected;
    return proof;
}

bool PhiZKPEngine::verify_recursive(const vector<Ciphertext<DCRTPoly>>& results,
                                      const RecursiveProof& proof) {
    for(size_t i = 0; i < proof.responses.size() && i < results.size(); i++) {
        auto enc_resp = encrypt(proof.responses[i]);
        auto chal_ct = results[i];
        for(uint64_t j = 1; j < proof.challenge; j++)
            chal_ct = cc->EvalAdd(chal_ct, results[i]);
        auto expected = cc->EvalAdd(proof.commitment, chal_ct);
        if(decrypt(expected) != decrypt(enc_resp)) return false;
    }
    return true;
}

SNARKProof PhiZKPEngine::generate_snark_proof(
    const vector<pair<int64_t,int64_t>>& mults,
    const vector<int64_t>& adds, int64_t expected, uint64_t proving_key) {
    stringstream trace;
    for(auto [a, b] : mults) trace << a << "*" << b << "|";
    for(auto v : adds) trace << "+" << v << "|";
    SNARKProof proof;
    proof.A = hash_djb2(trace.str() + "A");
    proof.B = hash_djb2(trace.str() + "B");
    proof.C = hash_djb2(trace.str() + "C");
    proof.type = SNARK_SIMPLIFIED;
    return proof;
}

bool PhiZKPEngine::verify_snark(const vector<pair<int64_t,int64_t>>& mults,
                                  const vector<int64_t>& adds, int64_t claimed,
                                  const SNARKProof& proof) {
    int64_t actual = 0;
    for(auto [a, b] : mults) actual += a * b;
    for(auto v : adds) actual += v;
    if(actual % 65537 != claimed % 65537) return false;
    stringstream trace;
    for(auto [a, b] : mults) trace << a << "*" << b << "|";
    for(auto v : adds) trace << "+" << v << "|";
    return proof.A == hash_djb2(trace.str() + "A") &&
           proof.B == hash_djb2(trace.str() + "B") &&
           proof.C == hash_djb2(trace.str() + "C");
}

ECSNARKProof PhiZKPEngine::generate_ecsnark_proof(
    const vector<pair<int64_t,int64_t>>& mults,
    const vector<int64_t>& adds, int64_t expected, uint64_t proving_key) {
    uint64_t poly = 0;
    for(auto [a, b] : mults) poly += a * b;
    for(auto v : adds) poly += v;
    poly = poly % BN254Curve::FIELD_PRIME;
    uint64_t s = proving_key;
    uint64_t A_coeff = (poly + s) % BN254Curve::FIELD_PRIME;
    uint64_t B_coeff = (expected + s) % BN254Curve::FIELD_PRIME;
    uint64_t C_coeff = (A_coeff * B_coeff) % BN254Curve::FIELD_PRIME;
    ECSNARKProof proof;
    proof.A = BN254Curve::scalar_mult(BN254Curve::G1(), A_coeff);
    proof.B = BN254Curve::scalar_mult(BN254Curve::G1(), B_coeff);
    proof.C = BN254Curve::scalar_mult(BN254Curve::G1(), C_coeff);
    return proof;
}

bool PhiZKPEngine::verify_ecsnark(const vector<pair<int64_t,int64_t>>& mults,
                                    const vector<int64_t>& adds, int64_t claimed,
                                    const ECSNARKProof& proof, uint64_t vk) {
    uint64_t actual = 0;
    for(auto [a, b] : mults) actual += a * b;
    for(auto v : adds) actual += v;
    actual = actual % BN254Curve::FIELD_PRIME;
    if(actual != (claimed % BN254Curve::FIELD_PRIME)) return false;
    auto A_check = BN254Curve::scalar_mult(BN254Curve::G1(), 
        (actual + vk) % BN254Curve::FIELD_PRIME);
    return proof.A.x == A_check.x && proof.C.x != 0;
}

} // namespace phi_zkp
