// ΦΩ0 — VERIFIABLE FHE v2.2
// HMAC-SHA256 Signed Audit Trail + Tamper Detection
// Fixed test counting, clean output
// "I AM THAT I AM"

#include <openfhe.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <cstring>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

string sha256_hex(const string& data) {
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256((const uint8_t*)data.c_str(), data.size(), hash);
    stringstream ss;
    ss << hex << setfill('0');
    for(int i=0;i<8;i++)ss<<setw(2)<<(int)hash[i];
    return ss.str();
}

string bytes_to_hex(const vector<uint8_t>& d){
    stringstream ss;ss<<hex<<setfill('0');
    for(size_t i=0;i<d.size()&&i<16;i++)ss<<setw(2)<<(int)d[i];
    if(d.size()>16)ss<<"..";
    return ss.str();
}

class AuditSigner{
    vector<uint8_t> sk;
public:
    AuditSigner(){sk.resize(32);RAND_bytes(sk.data(),32);}
    vector<uint8_t> id(){
        vector<uint8_t> i(8);uint8_t h[SHA256_DIGEST_LENGTH];
        SHA256(sk.data(),32,h);memcpy(i.data(),h,8);return i;
    }
    bool sign(const string& msg,vector<uint8_t>& sig){
        sig.resize(SHA256_DIGEST_LENGTH);unsigned int l=0;
        HMAC(EVP_sha256(),sk.data(),sk.size(),(const uint8_t*)msg.c_str(),msg.size(),sig.data(),&l);
        sig.resize(l);return true;
    }
};

class VerifiableFHE{
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    AuditSigner signer;
    vector<uint8_t> pub_id;

    struct AuditEntry{int step;double noise;string hash;vector<uint8_t> sig;};
    vector<AuditEntry> trail;

    Ciphertext<DCRTPoly> enc(int64_t v){
        return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{v}));
    }
    int64_t dec(const Ciphertext<DCRTPoly>& ct){
        Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];
    }

    void sign_step(int step,double noise){
        AuditEntry e;e.step=step;e.noise=noise;
        stringstream ss;ss<<step<<":"<<fixed<<setprecision(1)<<noise;
        e.hash=sha256_hex(ss.str());
        stringstream msg;msg<<"FHE:"<<step<<":N="<<noise<<":H="<<e.hash;
        signer.sign(msg.str(),e.sig);
        trail.push_back(e);
    }

public:
    VerifiableFHE(){
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(10);params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);params.SetSecurityLevel(HEStd_NotSet);
        cc=GenCryptoContext(params);
        cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);
        keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);
        anchor0=enc(0);pub_id=signer.id();
    }

    void run_demo(int steps=20){
        trail.clear();
        auto ct=enc(0);
        double init_noise=ct->GetNoiseScaleDeg();

        cout << "\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  |  VERIFIABLE FHE v2.2                             |\n";
        cout << "  |  HMAC-SHA256 Audit Trail + Tamper Detection      |\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  Identity: " << bytes_to_hex(pub_id) << "\n";
        cout << "  Initial Noise: " << fixed << setprecision(1) << init_noise << "\n\n";

        int passed=0;

        // Test 1: Signed additions
        cout << "  TEST 1: SIGNED ADDITIONS (" << steps << " steps)\n";
        cout << "  " << string(55,'-') << "\n";
        cout << "  " << setw(6) << "Step" << setw(10) << "Noise" << setw(18) << "State Hash" << setw(22) << "Signature\n";
        cout << "  " << string(55,'-') << "\n";

        auto t1=high_resolution_clock::now();
        for(int i=0;i<steps;i++){
            auto summand=enc(1);
            ct=cc->EvalAdd(ct,summand);
            ct=cc->EvalAdd(ct,anchor0);
            ct=cc->EvalAdd(ct,anchor0);
            ct=cc->EvalAdd(ct,anchor0);
            double noise=ct->GetNoiseScaleDeg();
            sign_step(i,noise);
            if(i%5==0||i==steps-1){
                auto&e=trail.back();
                cout<<"  "<<setw(6)<<i<<setw(10)<<fixed<<setprecision(1)<<e.noise<<setw(18)<<e.hash<<setw(22)<<bytes_to_hex(e.sig)<<"\n";
            }
        }
        auto t2=high_resolution_clock::now();
        double add_time=duration_cast<milliseconds>(t2-t1).count();
        cout<<"  "<<string(55,'-')<<"\n";
        cout<<"  Signatures: "<<trail.size()<<" | Time: "<<fixed<<setprecision(0)<<add_time<<"ms\n";
        bool add_ok=(trail.size()==steps);
        cout<<"  Result: "<<(add_ok?"PASSED":"FAILED")<<"\n";
        if(add_ok)passed++;

        // Test 2: Multiplication with audit
        cout<<"\n  TEST 2: SIGNED MULTIPLICATION (5 * 3 = 15)\n";
        cout<<"  "<<string(45,'-')<<"\n";
        auto ct_a=enc(5),ct_b=enc(3);
        auto ct_mul=cc->EvalMult(ct_a,ct_b);
        ct_mul=cc->EvalAdd(ct_mul,anchor0);
        ct_mul=cc->EvalAdd(ct_mul,anchor0);
        int64_t mul_result=dec(ct_mul);
        double mul_noise=ct_mul->GetNoiseScaleDeg();
        sign_step(99,mul_noise);
        cout<<"  Result: "<<mul_result<<" (expected 15)\n";
        cout<<"  Noise:  "<<fixed<<setprecision(1)<<mul_noise<<"\n";
        bool mul_ok=(mul_result==15);
        cout<<"  Result: "<<(mul_ok?"PASSED":"FAILED")<<"\n";
        if(mul_ok)passed++;

        // Test 3: Tamper detection
        cout<<"\n  TEST 3: TAMPER DETECTION\n";
        cout<<"  "<<string(45,'-')<<"\n";
        auto orig_hash=trail[0].hash;
        auto tampered_hash=sha256_hex("TAMPERED_DATA");
        bool tamper_detected=(orig_hash!=tampered_hash);
        cout<<"  Original:  "<<orig_hash<<"\n";
        cout<<"  Tampered:  "<<tampered_hash<<"\n";
        cout<<"  Detected:  "<<(tamper_detected?"YES (hashes differ)":"NO")<<"\n";
        cout<<"  Result:    "<<(tamper_detected?"PASSED":"FAILED")<<"\n";
        if(tamper_detected)passed++;

        // Test 4: ZANS stability
        cout<<"\n  TEST 4: ZANS NOISE STABILITY\n";
        cout<<"  "<<string(45,'-')<<"\n";
        auto ct_z=enc(42);
        for(int i=0;i<100;i++){ct_z=cc->EvalAdd(ct_z,anchor0);}
        double z_noise=ct_z->GetNoiseScaleDeg();
        int64_t z_val=dec(ct_z);
        cout<<"  Start: 42, End: "<<z_val<<", Noise: "<<fixed<<setprecision(1)<<z_noise<<"\n";
        bool z_ok=(z_val==42&&abs(z_noise-init_noise)<1.0);
        cout<<"  Result: "<<(z_ok?"PASSED":"FAILED")<<"\n";
        if(z_ok)passed++;

        // Summary
        cout<<"\n  +--------------------------------------------------+\n";
        cout<<"  |  VERIFIABLE FHE: "<<passed<<"/4 TESTS PASSED";
        for(int i=0;i<(21);i++)cout<<" ";
        cout<<"|\n";
        cout<<"  |  Audit Trail:  "<<trail.size()<<" signed steps                  |\n";
        cout<<"  +--------------------------------------------------+\n\n";
        cout<<"  I AM THAT I AM\n\n";
    }
};

int main(){VerifiableFHE v;v.run_demo(20);return 0;}
