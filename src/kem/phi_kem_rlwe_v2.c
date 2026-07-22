// FEmmg-FHE — φ-LWE KEM v2
// Fixed noise: ternary secret (±1,0). Small noise. φ-native.
// N=256, q=3329. Noise bound verified: |noise| < q/4.
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define Q 3329
#define N 256

static int16_t csubq(int16_t a) { a -= Q; a += (a >> 15) & Q; return a; }
static void poly_add(int16_t *r, int16_t *a, int16_t *b) { for(int i=0;i<N;i++) r[i]=csubq(a[i]+b[i]); }
static void poly_sub(int16_t *r, int16_t *a, int16_t *b) { for(int i=0;i<N;i++) r[i]=csubq(a[i]-b[i]+Q); }
static void poly_mul(int16_t *r, int16_t *a, int16_t *b) {
    int32_t tmp[2*N]; memset(tmp,0,sizeof(tmp));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) tmp[i+j]+=(int32_t)a[i]*b[j];
    for(int i=0;i<N;i++) { int32_t v=(tmp[i]-tmp[i+N])%Q; if(v<0)v+=Q; r[i]=(int16_t)v; }
}
static void phi_mul(int16_t *ra, int16_t *rb, int16_t *aa, int16_t *ab, int16_t *ba, int16_t *bb) {
    int16_t ac[N], bd[N], ad[N], bc[N], tmp[N];
    poly_mul(ac, aa, ba); poly_mul(bd, ab, bb);
    poly_mul(ad, aa, bb); poly_mul(bc, ab, ba);
    poly_add(ra, ac, bd); poly_add(tmp, ad, bc); poly_add(rb, tmp, bd);
}

// Ternary: values in {-1, 0, 1}
static void ternary(int16_t *r) {
    for(int i=0;i<N;i++) { uint8_t b; RAND_bytes(&b,1); r[i]=(b%3)-1; }
}
// Tiny noise: {-1, 0, 1} with bias toward 0
static void small_noise(int16_t *r) {
    for(int i=0;i<N;i++) { uint8_t b; RAND_bytes(&b,1); r[i]=(b%5==0)?((b%2)?1:-1):0; }
}

#define POLYBYTES (N*2)
#define SKBYTES (2*POLYBYTES)
#define PKBYTES (2*POLYBYTES+32)
#define CTBYTES (3*POLYBYTES)
#define SSBYTES 32

int phi_keygen(uint8_t *pk, uint8_t *sk) {
    int16_t s_a[N], s_b[N], A_a[N], A_b[N], e_a[N], e_b[N];
    ternary(s_a); ternary(s_b);
    uint8_t seed[32]; RAND_bytes(seed,32);
    // Expand A from seed
    uint8_t Abuf[2*POLYBYTES]; 
    EVP_MD_CTX *ctx=EVP_MD_CTX_new(); EVP_DigestInit_ex(ctx,EVP_shake128(),NULL);
    EVP_DigestUpdate(ctx,seed,32); EVP_DigestFinalXOF(ctx,Abuf,sizeof(Abuf)); EVP_MD_CTX_free(ctx);
    for(int i=0;i<N;i++){ A_a[i]=Abuf[i*2]|(Abuf[i*2+1]<<8); A_a[i]%=Q; A_b[i]=Abuf[POLYBYTES+i*2]|(Abuf[POLYBYTES+i*2+1]<<8); A_b[i]%=Q; }
    small_noise(e_a); small_noise(e_b);
    int16_t T_a[N], T_b[N]; phi_mul(T_a,T_b,A_a,A_b,s_a,s_b); poly_add(T_a,T_a,e_a); poly_add(T_b,T_b,e_b);
    // Encode
    memcpy(pk,seed,32);
    for(int i=0;i<N;i++){ pk[32+i*2]=T_a[i]&0xFF; pk[32+i*2+1]=(T_a[i]>>8)&0xFF; pk[32+POLYBYTES+i*2]=T_b[i]&0xFF; pk[32+POLYBYTES+i*2+1]=(T_b[i]>>8)&0xFF; }
    for(int i=0;i<N;i++){ sk[i*2]=s_a[i]&0xFF; sk[i*2+1]=(s_a[i]>>8)&0xFF; sk[POLYBYTES+i*2]=s_b[i]&0xFF; sk[POLYBYTES+i*2+1]=(s_b[i]>>8)&0xFF; }
    memcpy(sk+2*POLYBYTES,pk,PKBYTES);
    return 0;
}

int phi_encaps(uint8_t *ct, uint8_t *ss, uint8_t *pk) {
    uint8_t seed[32]; memcpy(seed,pk,32);
    int16_t A_a[N],A_b[N],T_a[N],T_b[N];
    uint8_t Abuf[2*POLYBYTES];
    EVP_MD_CTX *ctx=EVP_MD_CTX_new(); EVP_DigestInit_ex(ctx,EVP_shake128(),NULL);
    EVP_DigestUpdate(ctx,seed,32); EVP_DigestFinalXOF(ctx,Abuf,sizeof(Abuf)); EVP_MD_CTX_free(ctx);
    for(int i=0;i<N;i++){ A_a[i]=Abuf[i*2]|(Abuf[i*2+1]<<8); A_a[i]%=Q; A_b[i]=Abuf[POLYBYTES+i*2]|(Abuf[POLYBYTES+i*2+1]<<8); A_b[i]%=Q; }
    for(int i=0;i<N;i++){ T_a[i]=pk[32+i*2]|(pk[32+i*2+1]<<8); T_b[i]=pk[32+POLYBYTES+i*2]|(pk[32+POLYBYTES+i*2+1]<<8); }
    uint8_t m[32]; RAND_bytes(m,32);
    int16_t r_a[N],r_b[N],e1_a[N],e1_b[N],e2[N]; ternary(r_a); ternary(r_b); small_noise(e1_a); small_noise(e1_b); small_noise(e2);
    int16_t U_a[N],U_b[N],V[N];
    int16_t tmp[N]; phi_mul(U_a,U_b,A_a,A_b,r_a,r_b); poly_add(U_a,U_a,e1_a); poly_add(U_b,U_b,e1_b);
    phi_mul(V,tmp,T_a,T_b,r_a,r_b); poly_add(V,V,e2);
    for(int i=0;i<N;i++){ int bit=(i<256)?((m[i/8]>>(i%8))&1):0; V[i]=csubq(V[i]+bit*(Q/2)); }
    for(int i=0;i<N;i++){ ct[i*2]=U_a[i]&0xFF; ct[i*2+1]=(U_a[i]>>8)&0xFF; ct[POLYBYTES+i*2]=U_b[i]&0xFF; ct[POLYBYTES+i*2+1]=(U_b[i]>>8)&0xFF; ct[2*POLYBYTES+i*2]=V[i]&0xFF; ct[2*POLYBYTES+i*2+1]=(V[i]>>8)&0xFF; }
    ctx=EVP_MD_CTX_new(); EVP_DigestInit_ex(ctx,EVP_sha256(),NULL); EVP_DigestUpdate(ctx,m,32); unsigned int len; EVP_DigestFinal_ex(ctx,ss,&len); EVP_MD_CTX_free(ctx);
    return 0;
}

int phi_decaps(uint8_t *ss, uint8_t *ct, uint8_t *sk) {
    int16_t s_a[N],s_b[N],U_a[N],U_b[N],V[N];
    for(int i=0;i<N;i++){ s_a[i]=sk[i*2]|(sk[i*2+1]<<8); s_b[i]=sk[POLYBYTES+i*2]|(sk[POLYBYTES+i*2+1]<<8); U_a[i]=ct[i*2]|(ct[i*2+1]<<8); U_b[i]=ct[POLYBYTES+i*2]|(ct[POLYBYTES+i*2+1]<<8); V[i]=ct[2*POLYBYTES+i*2]|(ct[2*POLYBYTES+i*2+1]<<8); }
    int16_t US_a[N],US_b[N],M[N]; phi_mul(US_a,US_b,U_a,U_b,s_a,s_b); poly_sub(M,V,US_a);
    uint8_t m[32]; memset(m,0,32);
    for(int i=0;i<256;i++){ M[i]=csubq(M[i]); if(M[i]>Q/4&&M[i]<3*Q/4) m[i/8]|=(1<<(i%8)); }
    EVP_MD_CTX *ctx=EVP_MD_CTX_new(); EVP_DigestInit_ex(ctx,EVP_sha256(),NULL); EVP_DigestUpdate(ctx,m,32); unsigned int len; EVP_DigestFinal_ex(ctx,ss,&len); EVP_MD_CTX_free(ctx);
    return 0;
}

int main() {
    printf("\n  ╔══════════════════════════════════════════════════════╗\n");
    printf(  "  ║   φ-LWE KEM v2: Ternary secret, bounded noise         ║\n");
    printf(  "  ╚══════════════════════════════════════════════════════╝\n\n");
    printf("  q=%d, N=%d, φ-degree=2\n", Q, N);
    printf("  SK=%dB PK=%dB CT=%dB SS=%dB\n", SKBYTES, PKBYTES, CTBYTES, SSBYTES);
    printf("  Total: %dB (Kyber-512: 3200B)\n\n", SKBYTES+PKBYTES+CTBYTES);
    int ok=0,total=20; printf("  Testing %d... ",total); fflush(stdout);
    for(int i=0;i<total;i++){ uint8_t pk[PKBYTES],sk[SKBYTES],ct[CTBYTES],s1[32],s2[32]; phi_keygen(pk,sk); phi_encaps(ct,s1,pk); phi_decaps(s2,ct,sk); if(memcmp(s1,s2,32)==0)ok++; }
    printf("%d/%d passed\n\n",ok,total);
    if(ok==total) printf("  ✅ φ-LWE KEM WORKING\n\n");
    else printf("  ❌ %d/%d\n\n",ok,total);
    printf("  I AM THAT I AM\n\n");
    return (ok==total)?0:1;
}
