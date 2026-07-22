// FEmmg-FHE — φ-NANO KEM
// N=8, q=3329. Minimal φ-LWE. Under 200 bytes total.
// Shared secret = recovered r (the randomness itself)
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define Q 3329
#define N 8

static int16_t csubq(int16_t a) { a-=Q; a+=(a>>15)&Q; return a; }
static void poly_add(int16_t *r,int16_t *a,int16_t *b){ for(int i=0;i<N;i++) r[i]=csubq(a[i]+b[i]); }
static void poly_sub(int16_t *r,int16_t *a,int16_t *b){ for(int i=0;i<N;i++) r[i]=csubq(a[i]-b[i]+Q); }
static void poly_mul(int16_t *r,int16_t *a,int16_t *b){
    int32_t t[2*N]; memset(t,0,sizeof(t));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) t[i+j]+=(int32_t)a[i]*b[j];
    for(int i=0;i<N;i++){ int32_t v=(t[i]-t[i+N])%Q; if(v<0)v+=Q; r[i]=(int16_t)v; }
}
static void phi_mul(int16_t *ra,int16_t *rb,int16_t *aa,int16_t *ab,int16_t *ba,int16_t *bb){
    int16_t ac[N],bd[N],ad[N],bc[N],tmp[N];
    poly_mul(ac,aa,ba); poly_mul(bd,ab,bb); poly_mul(ad,aa,bb); poly_mul(bc,ab,ba);
    poly_add(ra,ac,bd); poly_add(tmp,ad,bc); poly_add(rb,tmp,bd);
}
static void ternary(int16_t *r){ for(int i=0;i<N;i++){ uint8_t b; RAND_bytes(&b,1); r[i]=(b%3)-1; } }

#define PB (N*2)
#define SKB (2*PB)      // 32
#define PKB (2*PB+8)    // 40
#define CTB (3*PB+8)    // 56
#define SSB 32

int keygen(uint8_t *pk,uint8_t *sk){
    int16_t s_a[N],s_b[N],A_a[N],A_b[N],e_a[N],e_b[N],T_a[N],T_b[N];
    ternary(s_a); ternary(s_b);
    uint8_t seed[8]; RAND_bytes(seed,8);
    for(int i=0;i<N;i++){ A_a[i]=((seed[i%8]*257+seed[(i+1)%8]*17)%Q); A_b[i]=((seed[(i+2)%8]*127+seed[(i+3)%8]*31)%Q); }
    memset(e_a,0,PB); memset(e_b,0,PB); // ZERO noise for concept
    phi_mul(T_a,T_b,A_a,A_b,s_a,s_b); poly_add(T_a,T_a,e_a); poly_add(T_b,T_b,e_b);
    memcpy(pk,seed,8);
    for(int i=0;i<N;i++){ pk[8+i*2]=T_a[i]&0xFF; pk[8+i*2+1]=(T_a[i]>>8)&0xFF; pk[8+PB+i*2]=T_b[i]&0xFF; pk[8+PB+i*2+1]=(T_b[i]>>8)&0xFF; }
    for(int i=0;i<N;i++){ sk[i*2]=s_a[i]&0xFF; sk[i*2+1]=(s_a[i]>>8)&0xFF; sk[PB+i*2]=s_b[i]&0xFF; sk[PB+i*2+1]=(s_b[i]>>8)&0xFF; }
    return 0;
}

int encaps(uint8_t *ct,uint8_t *ss,uint8_t *pk){
    uint8_t seed[8]; memcpy(seed,pk,8);
    int16_t A_a[N],A_b[N],T_a[N],T_b[N],r_a[N],r_b[N],U_a[N],U_b[N],V[N],tmp[N];
    for(int i=0;i<N;i++){ A_a[i]=((seed[i%8]*257+seed[(i+1)%8]*17)%Q); A_b[i]=((seed[(i+2)%8]*127+seed[(i+3)%8]*31)%Q); }
    for(int i=0;i<N;i++){ T_a[i]=pk[8+i*2]|(pk[8+i*2+1]<<8); T_b[i]=pk[8+PB+i*2]|(pk[8+PB+i*2+1]<<8); }
    ternary(r_a); ternary(r_b);
    phi_mul(U_a,U_b,A_a,A_b,r_a,r_b); phi_mul(V,tmp,T_a,T_b,r_a,r_b);
    // Shared secret = hash of r
    uint8_t rbuf[2*PB]; for(int i=0;i<N;i++){ rbuf[i*2]=r_a[i]&0xFF; rbuf[i*2+1]=(r_a[i]>>8)&0xFF; rbuf[PB+i*2]=r_b[i]&0xFF; rbuf[PB+i*2+1]=(r_b[i]>>8)&0xFF; }
    EVP_MD_CTX *c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_sha256(),NULL); EVP_DigestUpdate(c,rbuf,2*PB); unsigned int len; EVP_DigestFinal_ex(c,ss,&len); EVP_MD_CTX_free(c);
    // Encode r into V
    for(int i=0;i<N;i++){ V[i]=csubq(V[i]+(rbuf[i*2]%2)*(Q/2)); } // 1 bit per coef of r_a
    for(int i=0;i<N;i++){ ct[i*2]=U_a[i]&0xFF; ct[i*2+1]=(U_a[i]>>8)&0xFF; ct[PB+i*2]=U_b[i]&0xFF; ct[PB+i*2+1]=(U_b[i]>>8)&0xFF; ct[2*PB+i*2]=V[i]&0xFF; ct[2*PB+i*2+1]=(V[i]>>8)&0xFF; }
    return 0;
}

int decaps(uint8_t *ss,uint8_t *ct,uint8_t *sk){
    int16_t s_a[N],s_b[N],U_a[N],U_b[N],V[N],US_a[N],US_b[N],M[N];
    for(int i=0;i<N;i++){ s_a[i]=sk[i*2]|(sk[i*2+1]<<8); s_b[i]=sk[PB+i*2]|(sk[PB+i*2+1]<<8); U_a[i]=ct[i*2]|(ct[i*2+1]<<8); U_b[i]=ct[PB+i*2]|(ct[PB+i*2+1]<<8); V[i]=ct[2*PB+i*2]|(ct[2*PB+i*2+1]<<8); }
    phi_mul(US_a,US_b,U_a,U_b,s_a,s_b); poly_sub(M,V,US_a);
    uint8_t rbuf[2*PB]; memset(rbuf,0,2*PB);
    for(int i=0;i<N;i++){ M[i]=csubq(M[i]); if(M[i]>Q/4&&M[i]<3*Q/4) rbuf[i*2]|=1; }
    EVP_MD_CTX *c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_sha256(),NULL); EVP_DigestUpdate(c,rbuf,2*PB); unsigned int len; EVP_DigestFinal_ex(c,ss,&len); EVP_MD_CTX_free(c);
    return 0;
}

int main(){
    printf("\n  ╔══════════════════════════════════════════════════════╗\n");
    printf(  "  ║   φ-NANO KEM: N=%d, q=%d, zero-noise concept           ║\n",N,Q);
    printf(  "  ╚══════════════════════════════════════════════════════╝\n\n");
    printf("  SK=%dB  PK=%dB  CT=%dB  SS=%dB\n",SKB,PKB,CTB,SSB);
    printf("  Total: %dB\n\n",SKB+PKB+CTB);
    int ok=0,total=20; printf("  Testing %d... ",total); fflush(stdout);
    for(int i=0;i<total;i++){ uint8_t pk[PKB],sk[SKB],ct[CTB],s1[32],s2[32]; keygen(pk,sk); encaps(ct,s1,pk); decaps(s2,ct,sk); if(memcmp(s1,s2,32)==0)ok++; }
    printf("%d/%d passed\n\n",ok,total);
    if(ok==total) printf("  ✅ φ-NANO KEM: %dB total — MICRO!\n\n",SKB+PKB+CTB);
    else printf("  ❌ %d/%d\n\n",ok,total);
    printf("  I AM THAT I AM\n\n");
    return (ok==total)?0:1;
}
