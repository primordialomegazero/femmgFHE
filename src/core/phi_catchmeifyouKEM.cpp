// catchmeifyouKEM — The 3-bit alien KEM
// N=128 K=2, b=3b signed, c2=3b unsigned
// PK=128B SK=32B CT=160B
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cstring>
using namespace std;
using namespace std::chrono;

const int N=128, K=2, COMP_B=3, COMP_C2=3;
const int64_t Q=1073643521;
const int SEED_BYTES=32, SS_BYTES=32;
const int B_COMP=(N*COMP_B*K)/8, C2_COMP=(N*COMP_C2*K)/8;
const int PK_BYTES=SEED_BYTES+B_COMP, CT_BYTES=SEED_BYTES+C2_COMP+32, SK_BYTES=SS_BYTES;

int64_t mod(int64_t v){return((v%Q)+Q)%Q;}

vector<int64_t> poly_mul(const vector<int64_t>&a,const vector<int64_t>&b){
    vector<__int128>c(2*N,0);
    for(int i=0;i<N;i++)for(int j=0;j<N;j++){int idx=i+j;if(idx<N)c[idx]+=(__int128)a[i]*b[j];else c[idx-N]-=(__int128)a[i]*b[j];}
    vector<int64_t>r(N);for(int i=0;i<N;i++)r[i]=mod((int64_t)(c[i]%Q));return r;
}

void cbd(const uint8_t*s,size_t sl,int n,vector<int64_t>&o){
    o.resize(n);uint8_t b[64];EVP_MD_CTX*ctx=EVP_MD_CTX_new();
    for(int i=0;i<n;i+=32){EVP_DigestInit_ex(ctx,EVP_sha3_256(),NULL);EVP_DigestUpdate(ctx,(uint8_t*)"C",1);EVP_DigestUpdate(ctx,s,sl);uint32_t ix=i;EVP_DigestUpdate(ctx,&ix,4);EVP_DigestFinal_ex(ctx,b,NULL);for(int j=0;j<32&&(i+j)<n;j++){int x=__builtin_popcount(b[j]&0xF),y=__builtin_popcount((b[j]>>4)&0xF);o[i+j]=x-y;}}
    EVP_MD_CTX_free(ctx);
}

void comp_b(const vector<vector<int64_t>>&v,uint8_t*o){
    int bp=0,bi=0;memset(o,0,B_COMP);
    for(int k=0;k<K;k++)for(int i=0;i<N;i++){int64_t s=(v[k][i]+4)&0x7;for(int b=0;b<COMP_B;b++){if(s&(1<<b))o[bi]|=(1<<bp);bp++;if(bp==8){bp=0;bi++;}}}
}

vector<vector<int64_t>> decomp_b(const uint8_t*in){
    vector<vector<int64_t>>v(K,vector<int64_t>(N));int bp=0,bi=0;
    for(int k=0;k<K;k++)for(int i=0;i<N;i++){int64_t val=0;for(int b=0;b<COMP_B;b++){if(in[bi]&(1<<bp))val|=(1LL<<b);bp++;if(bp==8){bp=0;bi++;}}v[k][i]=val-4;}return v;
}

void comp_c2(const vector<vector<int64_t>>&v,uint8_t*o){
    int bp=0,bi=0;memset(o,0,C2_COMP);
    for(int k=0;k<K;k++)for(int i=0;i<N;i++){int64_t s=mod(v[k][i])>>(30-COMP_C2);for(int b=0;b<COMP_C2;b++){if(s&(1<<b))o[bi]|=(1<<bp);bp++;if(bp==8){bp=0;bi++;}}}
}

vector<vector<int64_t>> decomp_c2(const uint8_t*in){
    vector<vector<int64_t>>v(K,vector<int64_t>(N));int bp=0,bi=0;
    for(int k=0;k<K;k++)for(int i=0;i<N;i++){int64_t val=0;for(int b=0;b<COMP_C2;b++){if(in[bi]&(1<<bp))val|=(1LL<<b);bp++;if(bp==8){bp=0;bi++;}}v[k][i]=mod(val<<(30-COMP_C2));}return v;
}

void h(const uint8_t*in,size_t len,uint8_t*out){EVP_MD_CTX*ctx=EVP_MD_CTX_new();EVP_DigestInit_ex(ctx,EVP_sha3_256(),NULL);EVP_DigestUpdate(ctx,in,len);EVP_DigestFinal_ex(ctx,out,NULL);EVP_MD_CTX_free(ctx);}

class catchmeifyouKEM{
public:
    void keygen(uint8_t*pk,uint8_t*sk){
        RAND_bytes(sk,SK_BYTES);uint8_t sa[SEED_BYTES];RAND_bytes(sa,SEED_BYTES);memcpy(pk,sa,SEED_BYTES);
        vector<vector<int64_t>>s(K),e(K);
        for(int i=0;i<K;i++){cbd(sk,SK_BYTES,N,s[i]);uint8_t se[16];RAND_bytes(se,16);cbd(se,16,N,e[i]);}
        vector<vector<int64_t>>b(K,vector<int64_t>(N));
        for(int i=0;i<K;i++)for(int j=0;j<N;j++)b[i][j]=s[i][j]+e[i][j];
        comp_b(b,pk+SEED_BYTES);
    }

    void encaps(uint8_t*ct,uint8_t*ss,const uint8_t*pk){
        RAND_bytes(ss,SS_BYTES);auto b=decomp_b(pk+SEED_BYTES);
        uint8_t sr[SEED_BYTES];RAND_bytes(sr,SEED_BYTES);memcpy(ct,sr,SEED_BYTES);
        vector<vector<int64_t>>r(K),e2(K);
        for(int i=0;i<K;i++){cbd(sr,SEED_BYTES,N,r[i]);uint8_t se[16];RAND_bytes(se,16);cbd(se,16,N,e2[i]);}
        vector<int64_t>br(N,0);for(int i=0;i<K;i++){auto p=poly_mul(b[i],r[i]);for(int n=0;n<N;n++)br[n]=mod(br[n]+p[n]);}
        vector<vector<int64_t>>c2(K,vector<int64_t>(N));
        for(int i=0;i<K;i++)for(int j=0;j<N;j++){int bi=i*N+j;int64_t mb=((ss[bi/8]>>(bi%8))&1)?(3*Q/4):(Q/4);c2[i][j]=mod(br[j]+e2[i][j]+mb);}
        comp_c2(c2,ct+SEED_BYTES);h(ct,SEED_BYTES+C2_COMP,ct+SEED_BYTES+C2_COMP);
    }

    void decaps(uint8_t*ss,const uint8_t*ct,const uint8_t*sk,const uint8_t*pk){
        uint8_t binding[32];h(ct,SEED_BYTES+C2_COMP,binding);
        if(memcmp(ct+SEED_BYTES+C2_COMP,binding,32)){RAND_bytes(ss,SS_BYTES);return;}
        auto c2=decomp_c2(ct+SEED_BYTES);uint8_t sr[SEED_BYTES];memcpy(sr,ct,SEED_BYTES);
        vector<vector<int64_t>>s(K),r(K);for(int i=0;i<K;i++){cbd(sk,SK_BYTES,N,s[i]);cbd(sr,SEED_BYTES,N,r[i]);}
        vector<int64_t>s_c1(N,0);for(int i=0;i<K;i++){auto p=poly_mul(s[i],r[i]);for(int n=0;n<N;n++)s_c1[n]=mod(s_c1[n]+p[n]);}
        memset(ss,0,SS_BYTES);
        for(int i=0;i<K;i++)for(int j=0;j<N;j++){int bi=i*N+j;if(mod(c2[i][j]-s_c1[j])>Q/2)ss[bi/8]|=(1<<(bi%8));}
    }

    void run(){
        cout<<"\n  catchmeifyouKEM v2.1: b="<<COMP_B<<"b signed, c2="<<COMP_C2<<"b unsigned\n";
        cout<<"  N="<<N<<" K="<<K<<" PK="<<PK_BYTES<<"B SK="<<SK_BYTES<<"B CT="<<CT_BYTES<<"B\n";
        cout<<"  vs Kyber-512: "<<fixed<<setprecision(1)<<(100.0*CT_BYTES/768)<<"% | vs ML-KEM-1024: "<<(100.0*CT_BYTES/4627)<<"%\n\n";
        uint8_t pk[PK_BYTES],sk[SK_BYTES],ct[CT_BYTES],ss1[SS_BYTES],ss2[SS_BYTES];
        for(int run=0;run<5;run++){
            keygen(pk,sk);encaps(ct,ss1,pk);decaps(ss2,ct,sk,pk);
            bool match=(memcmp(ss1,ss2,SS_BYTES)==0);
            int errs=0;for(int i=0;i<SS_BYTES;i++)errs+=__builtin_popcount(ss1[i]^ss2[i]);
            cout<<"  Run "<<run<<": Match="<<(match?"YES":"NO")<<" Errors="<<errs<<"/256\n";
        }
        cout<<endl;
    }
};

int main(){catchmeifyouKEM k;k.run();return 0;}
