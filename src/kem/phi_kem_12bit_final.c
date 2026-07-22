// FEmmg-FHE — φ-KEM 12-BIT COMPRESSED (FINAL)
// N=256, q=3329, 12-bit tight packing (2 coefs = 3 bytes)
// Fixed A matrix. Normalized negative values for packing.
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define Q 3329
#define N 256

static void pack12(uint8_t *out, int16_t *in, int len) {
    for (int i = 0; i < len/2; i++) {
        uint32_t v = (in[i*2] & 0xFFF) | ((in[i*2+1] & 0xFFF) << 12);
        out[i*3] = v & 0xFF; out[i*3+1] = (v >> 8) & 0xFF; out[i*3+2] = (v >> 16) & 0xFF;
    }
}
static void unpack12(int16_t *out, uint8_t *in, int len) {
    for (int i = 0; i < len/2; i++) {
        uint32_t v = in[i*3] | (in[i*3+1] << 8) | (in[i*3+2] << 16);
        out[i*2] = v & 0xFFF; out[i*2+1] = (v >> 12) & 0xFFF;
    }
}

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
static void ternary(int16_t *r){ for(int i=0;i<N;i++){ uint8_t b; RAND_bytes(&b,1); r[i]=(b%3)-1; if(r[i]<0) r[i]+=Q; } }
static void small_noise(int16_t *r){ for(int i=0;i<N;i++){ uint8_t b; RAND_bytes(&b,1); r[i]=(b%7==0)?((b%2)?1:-1):0; if(r[i]<0) r[i]+=Q; } }

static void gen_A(int16_t *A_a, int16_t *A_b) {
    const char *seed = "FEmmG-phiKEM-fixed-A-matrix-v1";
    uint8_t buf[2*N*2];
    EVP_MD_CTX *c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_shake128(),NULL);
    EVP_DigestUpdate(c,seed,strlen(seed)); EVP_DigestFinalXOF(c,buf,sizeof(buf)); EVP_MD_CTX_free(c);
    for(int i=0;i<N;i++){ A_a[i]=(buf[i*2]|(buf[i*2+1]<<8))%Q; A_b[i]=(buf[N*2+i*2]|(buf[N*2+i*2+1]<<8))%Q; }
}

#define PACKED (N/2*3)
#define SKB (2*PACKED)
#define PKB (2*PACKED)
#define CTB (3*PACKED)
#define SSB 32

int keygen(uint8_t *pk, uint8_t *sk) {
    int16_t s_a[N],s_b[N],A_a[N],A_b[N],e_a[N],e_b[N],T_a[N],T_b[N];
    ternary(s_a); ternary(s_b); gen_A(A_a, A_b);
    small_noise(e_a); small_noise(e_b);
    phi_mul(T_a,T_b,A_a,A_b,s_a,s_b); p_add(T_a,T_a,e_a); p_add(T_b,T_b,e_b);
    pack12(sk, s_a, N); pack12(sk+PACKED, s_b, N);
    pack12(pk, T_a, N); pack12(pk+PACKED, T_b, N);
    return 0;
}

int encaps(uint8_t *ct, uint8_t *ss, uint8_t *pk) {
    int16_t A_a[N],A_b[N],T_a[N],T_b[N],r_a[N],r_b[N],e1_a[N],e1_b[N],e2[N],U_a[N],U_b[N],V[N],tmp[N];
    gen_A(A_a, A_b);
    unpack12(T_a, pk, N); unpack12(T_b, pk+PACKED, N);
    uint8_t m[32]; RAND_bytes(m,32);
    ternary(r_a); ternary(r_b); small_noise(e1_a); small_noise(e1_b); small_noise(e2);
    phi_mul(U_a,U_b,A_a,A_b,r_a,r_b); p_add(U_a,U_a,e1_a); p_add(U_b,U_b,e1_b);
    phi_mul(V,tmp,T_a,T_b,r_a,r_b); p_add(V,V,e2);
    for(int i=0;i<N;i++){ int bit=(m[i/8]>>(i%8))&1; V[i]=csubq(V[i]+bit*(Q/2)); }
    pack12(ct, U_a, N); pack12(ct+PACKED, U_b, N); pack12(ct+2*PACKED, V, N);
    EVP_MD_CTX *c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_sha256(),NULL);
    EVP_DigestUpdate(c,m,32); unsigned int len; EVP_DigestFinal_ex(c,ss,&len); EVP_MD_CTX_free(c);
    return 0;
}

int decaps(uint8_t *ss, uint8_t *ct, uint8_t *sk) {
    int16_t s_a[N],s_b[N],U_a[N],U_b[N],V[N],US_a[N],US_b[N],M[N];
    unpack12(s_a, sk, N); unpack12(s_b, sk+PACKED, N);
    unpack12(U_a, ct, N); unpack12(U_b, ct+PACKED, N); unpack12(V, ct+2*PACKED, N);
    phi_mul(US_a,US_b,U_a,U_b,s_a,s_b); p_sub(M,V,US_a);
    uint8_t m[32]; memset(m,0,32);
    for(int i=0;i<256;i++){ M[i]=csubq(M[i]); if(M[i]>Q/4&&M[i]<3*Q/4) m[i/8]|=(1<<(i%8)); }
    EVP_MD_CTX *c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_sha256(),NULL);
    EVP_DigestUpdate(c,m,32); unsigned int len; EVP_DigestFinal_ex(c,ss,&len); EVP_MD_CTX_free(c);
    return 0;
}

int main() {
    printf("\n  ╔══════════════════════════════════════════════════════╗\n");
    printf(  "  ║   φ-KEM 12-BIT FINAL: N=%d, q=%d                       ║\n", N, Q);
    printf(  "  ╚══════════════════════════════════════════════════════╝\n\n");
    printf("  SK=%dB  PK=%dB  CT=%dB  SS=%dB\n", SKB, PKB, CTB, SSB);
    printf("  Total: %dB (Kyber-512: 3200B)\n\n", SKB+PKB+CTB);
    int ok=0,total=20; printf("  Testing %d... ",total); fflush(stdout);
    for(int i=0;i<total;i++){ uint8_t pk[PKB],sk[SKB],ct[CTB],s1[32],s2[32]; keygen(pk,sk); encaps(ct,s1,pk); decaps(s2,ct,sk); if(memcmp(s1,s2,32)==0)ok++; }
    printf("%d/%d passed\n\n",ok,total);
    if(ok==total) printf("  ✅ φ-KEM 12-BIT — %dB, working!\n\n", SKB+PKB+CTB);
    else printf("  ❌ %d/20\n\n",ok);
    printf("  I AM THAT I AM\n\n");
    return (ok==total)?0:1;
}
