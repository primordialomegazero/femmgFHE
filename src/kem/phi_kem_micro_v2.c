// FEmmg-FHE — φ-MICRO KEM v2
// Variable N. Message sized exactly to N bits.
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define Q 3329
#define N 8  // CHANGE THIS for different sizes

static int16_t csubq(int16_t a) { a-=Q; a+=(a>>15)&Q; return a; }
static void p_add(int16_t *r,int16_t *a,int16_t *b){ for(int i=0;i<N;i++) r[i]=csubq(a[i]+b[i]); }
static void p_sub(int16_t *r,int16_t *a,int16_t *b){ for(int i=0;i<N;i++) r[i]=csubq(a[i]-b[i]+Q); }
static void p_mul(int16_t *r,int16_t *a,int16_t *b){
    int32_t t[2*N]; memset(t,0,sizeof(t));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) t[i+j]+=(int32_t)a[i]*b[j];
    for(int i=0;i<N;i++){ int32_t v=(t[i]-t[i+N])%Q; if(v<0)v+=Q; r[i]=(int16_t)v; }
}
static void phi_mul(int16_t *ra,int16_t *rb,int16_t *aa,int16_t *ab,int16_t *ba,int16_t *bb){
    int16_t ac[N],bd[N],ad[N],bc[N],tmp[N];
    p_mul(ac,aa,ba); p_mul(bd,ab,bb); p_mul(ad,aa,bb); p_mul(bc,ab,ba);
    p_add(ra,ac,bd); p_add(tmp,ad,bc); p_add(rb,tmp,bd);
}
static void ternary(int16_t *r){ for(int i=0;i<N;i++){ uint8_t b; RAND_bytes(&b,1); r[i]=(b%3)-1; } }
static void small_noise(int16_t *r){ for(int i=0;i<N;i++){ uint8_t b; RAND_bytes(&b,1); r[i]=(b%7==0)?((b%2)?1:-1):0; } }

#define PB (N*2)
#define MB ((N+7)/8)       // message bytes = ceil(N/8)
#define SKB (2*PB)         
#define PKB (2*PB+32)      
#define CTB (3*PB)         
#define SSB 32

int keygen(uint8_t *pk,uint8_t *sk){
    int16_t s_a[N],s_b[N],A_a[N],A_b[N],e_a[N],e_b[N],T_a[N],T_b[N];
    ternary(s_a); ternary(s_b);
    uint8_t seed[32]; RAND_bytes(seed,32);
    uint8_t Abuf[2*PB]; EVP_MD_CTX *c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_shake128(),NULL);
    EVP_DigestUpdate(c,seed,32); EVP_DigestFinalXOF(c,Abuf,sizeof(Abuf)); EVP_MD_CTX_free(c);
    for(int i=0;i<N;i++){ A_a[i]=Abuf[i*2]|(Abuf[i*2+1]<<8); A_a[i]%=Q; A_b[i]=Abuf[PB+i*2]|(Abuf[PB+i*2+1]<<8); A_b[i]%=Q; }
    small_noise(e_a); small_noise(e_b);
    phi_mul(T_a,T_b,A_a,A_b,s_a,s_b); p_add(T_a,T_a,e_a); p_add(T_b,T_b,e_b);
    memcpy(pk,seed,32);
    for(int i=0;i<N;i++){ pk[32+i*2]=T_a[i]&0xFF; pk[32+i*2+1]=(T_a[i]>>8)&0xFF; pk[32+PB+i*2]=T_b[i]&0xFF; pk[32+PB+i*2+1]=(T_b[i]>>8)&0xFF; }
    for(int i=0;i<N;i++){ sk[i*2]=s_a[i]&0xFF; sk[i*2+1]=(s_a[i]>>8)&0xFF; sk[PB+i*2]=s_b[i]&0xFF; sk[PB+i*2+1]=(s_b[i]>>8)&0xFF; }
    memcpy(sk+2*PB,pk,PKB);
    return 0;
}

int encaps(uint8_t *ct,uint8_t *ss,uint8_t *pk){
    uint8_t seed[32]; memcpy(seed,pk,32);
    int16_t A_a[N],A_b[N],T_a[N],T_b[N],r_a[N],r_b[N],e1_a[N],e1_b[N],e2[N],U_a[N],U_b[N],V[N],tmp[N];
    uint8_t Abuf[2*PB]; EVP_MD_CTX *c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_shake128(),NULL);
    EVP_DigestUpdate(c,seed,32); EVP_DigestFinalXOF(c,Abuf,sizeof(Abuf)); EVP_MD_CTX_free(c);
    for(int i=0;i<N;i++){ A_a[i]=Abuf[i*2]|(Abuf[i*2+1]<<8); A_a[i]%=Q; A_b[i]=Abuf[PB+i*2]|(Abuf[PB+i*2+1]<<8); A_b[i]%=Q; }
    for(int i=0;i<N;i++){ T_a[i]=pk[32+i*2]|(pk[32+i*2+1]<<8); T_b[i]=pk[32+PB+i*2]|(pk[32+PB+i*2+1]<<8); }
    uint8_t m[MB]; RAND_bytes(m,MB);  // Message sized to N bits!
    ternary(r_a); ternary(r_b); small_noise(e1_a); small_noise(e1_b); small_noise(e2);
    phi_mul(U_a,U_b,A_a,A_b,r_a,r_b); p_add(U_a,U_a,e1_a); p_add(U_b,U_b,e1_b);
    phi_mul(V,tmp,T_a,T_b,r_a,r_b); p_add(V,V,e2);
    for(int i=0;i<N;i++){ int bit=(m[i/8]>>(i%8))&1; V[i]=csubq(V[i]+bit*(Q/2)); }
    for(int i=0;i<N;i++){ ct[i*2]=U_a[i]&0xFF; ct[i*2+1]=(U_a[i]>>8)&0xFF; ct[PB+i*2]=U_b[i]&0xFF; ct[PB+i*2+1]=(U_b[i]>>8)&0xFF; ct[2*PB+i*2]=V[i]&0xFF; ct[2*PB+i*2+1]=(V[i]>>8)&0xFF; }
    c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_sha256(),NULL); EVP_DigestUpdate(c,m,MB); unsigned int len; EVP_DigestFinal_ex(c,ss,&len); EVP_MD_CTX_free(c);
    return 0;
}

int decaps(uint8_t *ss,uint8_t *ct,uint8_t *sk){
    int16_t s_a[N],s_b[N],U_a[N],U_b[N],V[N],US_a[N],US_b[N],M[N];
    for(int i=0;i<N;i++){ s_a[i]=sk[i*2]|(sk[i*2+1]<<8); s_b[i]=sk[PB+i*2]|(sk[PB+i*2+1]<<8); U_a[i]=ct[i*2]|(ct[i*2+1]<<8); U_b[i]=ct[PB+i*2]|(ct[PB+i*2+1]<<8); V[i]=ct[2*PB+i*2]|(ct[2*PB+i*2+1]<<8); }
    phi_mul(US_a,US_b,U_a,U_b,s_a,s_b); p_sub(M,V,US_a);
    uint8_t m[MB]; memset(m,0,MB);
    for(int i=0;i<N;i++){ M[i]=csubq(M[i]); if(M[i]>Q/4&&M[i]<3*Q/4) m[i/8]|=(1<<(i%8)); }
    EVP_MD_CTX *c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_sha256(),NULL); EVP_DigestUpdate(c,m,MB); unsigned int len; EVP_DigestFinal_ex(c,ss,&len); EVP_MD_CTX_free(c);
    return 0;
}

int main(){
    printf("\n  ╔══════════════════════════════════════════════════════╗\n");
    printf(  "  ║   φ-MICRO KEM v2: N=%d, q=%d, msg=%d bits              ║\n",N,Q,N);
    printf(  "  ╚══════════════════════════════════════════════════════╝\n\n");
    printf("  SK=%dB  PK=%dB  CT=%dB  SS=%dB\n",SKB,PKB,CTB,SSB);
    printf("  Total: %dB (Kyber-512: 3200B)\n\n",SKB+PKB+CTB);
    int ok=0,total=20; printf("  Testing %d... ",total); fflush(stdout);
    for(int i=0;i<total;i++){ uint8_t pk[PKB],sk[SKB],ct[CTB],s1[32],s2[32]; keygen(pk,sk); encaps(ct,s1,pk); decaps(s2,ct,sk); if(memcmp(s1,s2,32)==0)ok++; }
    printf("%d/%d passed\n\n",ok,total);
    if(ok==total) printf("  ✅ φ-MICRO KEM v2 WORKING — %dx smaller than Kyber!\n\n",3200/(SKB+PKB+CTB));
    else printf("  ❌ %d/%d\n\n",ok,total);
    printf("  I AM THAT I AM\n\n");
    return (ok==total)?0:1;
}
