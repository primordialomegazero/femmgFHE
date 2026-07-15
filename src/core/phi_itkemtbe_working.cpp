// itKEMtbe — Ring-LWE KEM with fixed a=1
// Single polynomial (no module), a=1, small s/r/e
// c1 = r + e1, c2 = b*r + e2 + msg
// Decrypt: c2 - s*c1 = b*r + msg - s*(r+e1) = (s+e)*r + msg - s*r - s*e1
// = e*r + e2 - s*e1 + msg ≈ msg (all products are small*small)
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cstring>
using namespace std;
using namespace std::chrono;

const int N=256, COMP_BITS=8;
const int64_t Q=1073643521;
const int SEED_BYTES=32, SS_BYTES=32;
const int POLY_COMP=(N*COMP_BITS)/8;
const int PK_BYTES=SEED_BYTES+N*4; // seed + uncompressed b
const int CT_BYTES=SEED_BYTES+POLY_COMP+32; // seed_r + compressed c2 + binding
const int SK_BYTES=SS_BYTES;

int64_t mod(int64_t v){return((v%Q)+Q)%Q;}

vector<int64_t> poly_mul(const vector<int64_t>&a,const vector<int64_t>&b){
    vector<__int128> c(2*N,0);
    for(int i=0;i<N;i++)for(int j=0;j<N;j++){int idx=i+j;if(idx<N)c[idx]+=(__int128)a[i]*b[j];else c[idx-N]-=(__int128)a[i]*b[j];}
    vector<int64_t> r(N);
    for(int i=0;i<N;i++)r[i]=mod((int64_t)(c[i]%Q));
    return r;
}

void cbd(const uint8_t*s,size_t sl,int n,vector<int64_t>&o){
    o.resize(n);uint8_t b[64];EVP_MD_CTX*ctx=EVP_MD_CTX_new();
    for(int i=0;i<n;i+=32){EVP_DigestInit_ex(ctx,EVP_sha3_256(),NULL);EVP_DigestUpdate(ctx,(uint8_t*)"C",1);EVP_DigestUpdate(ctx,s,sl);uint32_t ix=i;EVP_DigestUpdate(ctx,&ix,4);EVP_DigestFinal_ex(ctx,b,NULL);for(int j=0;j<32&&(i+j)<n;j++){int x=__builtin_popcount(b[j]&0xF),y=__builtin_popcount((b[j]>>4)&0xF);o[i+j]=x-y;}} // SIGNED, not mod
    EVP_MD_CTX_free(ctx);
}

void compress(const vector<int64_t>&v,uint8_t*o){
    int bp=0,bi=0;memset(o,0,POLY_COMP);
    for(int i=0;i<N;i++){int64_t s=mod(v[i])>>(30-COMP_BITS);for(int b=0;b<COMP_BITS;b++){if(s&(1<<b))o[bi]|=(1<<bp);bp++;if(bp==8){bp=0;bi++;}}}
}

vector<int64_t> decompress(const uint8_t*in){
    vector<int64_t>v(N);int bp=0,bi=0;
    for(int i=0;i<N;i++){int64_t val=0;for(int b=0;b<COMP_BITS;b++){if(in[bi]&(1<<bp))val|=(1LL<<b);bp++;if(bp==8){bp=0;bi++;}}v[i]=mod(val<<(30-COMP_BITS));}
    return v;
}

class itKEMtbe{
public:
    void keygen(uint8_t*pk,uint8_t*sk){
        RAND_bytes(sk,SK_BYTES);
        uint8_t sa[SEED_BYTES];RAND_bytes(sa,SEED_BYTES);memcpy(pk,sa,SEED_BYTES);
        vector<int64_t>s,e;cbd(sk,SK_BYTES,N,s);uint8_t se[16];RAND_bytes(se,16);cbd(se,16,N,e);
        vector<int64_t>b(N); // b = s + e (a=1), small signed values
        for(int i=0;i<N;i++)b[i]=s[i]+e[i]; // Keep signed! Range ±8
        for(int i=0;i<N;i++){int64_t v=b[i];memcpy(pk+SEED_BYTES+i*4,&v,4);}
    }

    void encaps(uint8_t*ct,uint8_t*ss,const uint8_t*pk){
        RAND_bytes(ss,SS_BYTES);
        vector<int64_t>b(N);for(int i=0;i<N;i++){int64_t v;memcpy(&v,pk+SEED_BYTES+i*4,4);b[i]=v;} // Signed, don't mod!
        uint8_t sr[SEED_BYTES];RAND_bytes(sr,SEED_BYTES);memcpy(ct,sr,SEED_BYTES);
        vector<int64_t>r,e2;cbd(sr,SEED_BYTES,N,r);uint8_t se[16];RAND_bytes(se,16);cbd(se,16,N,e2);
        auto br=poly_mul(b,r);
        vector<int64_t>c2(N);
        for(int i=0;i<N;i++){
            int bit_idx=i%256;
            int64_t mb=((ss[bit_idx/8]>>(bit_idx%8))&1)?(3*Q/4):(Q/4);
            c2[i]=mod(br[i]+e2[i]+mb);
        }
        compress(c2,ct+SEED_BYTES);
    }

    void decaps(uint8_t*ss,const uint8_t*ct,const uint8_t*sk,const uint8_t*pk){
        uint8_t sr[SEED_BYTES];memcpy(sr,ct,SEED_BYTES);
        auto c2=decompress(ct+SEED_BYTES);
        vector<int64_t>s,r;cbd(sk,SK_BYTES,N,s);cbd(sr,SEED_BYTES,N,r);
        // c1 = r (a=1, no e1 needed since r is already short)
        auto&c1=r;
        auto s_c1=poly_mul(s,c1);
        memset(ss,0,SS_BYTES);
        cout<<"  [DBG] s_c1[0]="<<s_c1[0]<<" c2[0]="<<c2[0]<<" rec[0]="<<mod(c2[0]-s_c1[0])<<endl;
        cout<<"  [DBG] s_c1[1]="<<s_c1[1]<<" c2[1]="<<c2[1]<<" rec[1]="<<mod(c2[1]-s_c1[1])<<endl;
        for(int i=0;i<N;i++){
            int64_t rec=mod(c2[i]-s_c1[i]);
            int bit_idx=i%256;
            if(rec>Q/2)ss[bit_idx/8]|=(1<<(bit_idx%8));
        }
    }

    void run(){
        cout<<"\n  itKEMtbe Ring-LWE a=1 N="<<N<<" comp="<<COMP_BITS<<"b PK="<<PK_BYTES<<"B CT="<<CT_BYTES<<"B\n";
        uint8_t pk[PK_BYTES],sk[SK_BYTES],ct[CT_BYTES],ss1[SS_BYTES],ss2[SS_BYTES];
        for(int run=0;run<3;run++){
            keygen(pk,sk);encaps(ct,ss1,pk);decaps(ss2,ct,sk,pk);
            bool match=(memcmp(ss1,ss2,SS_BYTES)==0);
            int errs=0;for(int i=0;i<SS_BYTES;i++)errs+=__builtin_popcount(ss1[i]^ss2[i]);
            cout<<"  Run "<<run<<": Match="<<(match?"YES":"NO")<<" Errors="<<errs<<"/256\n";
        }
        cout<<endl;
    }
};

int main(){itKEMtbe k;k.run();return 0;}
