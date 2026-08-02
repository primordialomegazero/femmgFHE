// KEM 12-bit debug: Find exactly where compressed KEM fails
// Test each step: keygen → pack → unpack → encaps → decaps

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define Q 3329
#define N 256

// 12-bit packing (proven lossless)
static void pack12(uint8_t *out, int16_t *in, int len) {
    for (int i = 0; i < len/2; i++) {
        uint32_t v = (in[i*2] & 0xFFF) | ((in[i*2+1] & 0xFFF) << 12);
        out[i*3] = v & 0xFF;
        out[i*3+1] = (v >> 8) & 0xFF;
        out[i*3+2] = (v >> 16) & 0xFF;
    }
}
static void unpack12(int16_t *out, uint8_t *in, int len) {
    for (int i = 0; i < len/2; i++) {
        uint32_t v = in[i*3] | (in[i*3+1] << 8) | (in[i*3+2] << 16);
        out[i*2] = v & 0xFFF;
        out[i*2+1] = (v >> 12) & 0xFFF;
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
static void small_noise(int16_t *r){ for(int i=0;i<N;i++){ uint8_t b; RAND_bytes(&b,1); r[i]=(b%7==0)?((b%2)?1:-1):0; } }

#define PACKED (N/2*3)  // 384

// Fixed A generation
static void gen_A(int16_t *A_a, int16_t *A_b) {
    const char *seed = "FEmmG-phiKEM-fixed-A-matrix-v1";
    uint8_t buf[2*N*2];
    EVP_MD_CTX *c=EVP_MD_CTX_new(); EVP_DigestInit_ex(c,EVP_shake128(),NULL);
    EVP_DigestUpdate(c,seed,strlen(seed)); EVP_DigestFinalXOF(c,buf,sizeof(buf)); EVP_MD_CTX_free(c);
    for(int i=0;i<N;i++){ A_a[i]=(buf[i*2]|(buf[i*2+1]<<8))%Q; A_b[i]=(buf[N*2+i*2]|(buf[N*2+i*2+1]<<8))%Q; }
}

int main() {
    printf("\n  12-bit KEM Debug\n\n");

    // Test 1: Keygen → pack sk → unpack → verify
    printf("  [1] Keygen + pack/unpack SK: ");
    int16_t s_a[N], s_b[N], A_a[N], A_b[N], e_a[N], e_b[N], T_a[N], T_b[N];
    ternary(s_a); ternary(s_b);
    gen_A(A_a, A_b);
    small_noise(e_a); small_noise(e_b);
    phi_mul(T_a,T_b,A_a,A_b,s_a,s_b); p_add(T_a,T_a,e_a); p_add(T_b,T_b,e_b);
    
    uint8_t sk_packed[2*PACKED], pk_packed[2*PACKED];
    pack12(sk_packed, s_a, N);
    pack12(sk_packed+PACKED, s_b, N);
    pack12(pk_packed, T_a, N);
    pack12(pk_packed+PACKED, T_b, N);
    
    int16_t s_a2[N], s_b2[N], T_a2[N], T_b2[N];
    unpack12(s_a2, sk_packed, N);
    unpack12(s_b2, sk_packed+PACKED, N);
    unpack12(T_a2, pk_packed, N);
    unpack12(T_b2, pk_packed+PACKED, N);
    
    int ok=1;
    for(int i=0;i<N;i++) if(s_a[i]!=s_a2[i]||s_b[i]!=s_b2[i]||T_a[i]!=T_a2[i]||T_b[i]!=T_b2[i]) { ok=0; break; }
    printf("%s\n", ok?"PASS":"FAIL");

    // Test 2: Encaps with packed PK
    printf("  [2] Encaps with packed PK: ");
    int16_t r_a[N], r_b[N], e1_a[N], e1_b[N], e2[N], U_a[N], U_b[N], V[N], tmp[N];
    uint8_t m[32]; RAND_bytes(m,32);
    ternary(r_a); ternary(r_b); small_noise(e1_a); small_noise(e1_b); small_noise(e2);
    
    // Reconstruct A and T from packed
    int16_t A_a2[N], A_b2[N];
    gen_A(A_a2, A_b2);
    unpack12(T_a2, pk_packed, N);
    unpack12(T_b2, pk_packed+PACKED, N);
    
    phi_mul(U_a,U_b,A_a2,A_b2,r_a,r_b); p_add(U_a,U_a,e1_a); p_add(U_b,U_b,e1_b);
    phi_mul(V,tmp,T_a2,T_b2,r_a,r_b); p_add(V,V,e2);
    for(int i=0;i<N;i++){ int bit=(m[i/8]>>(i%8))&1; V[i]=csubq(V[i]+bit*(Q/2)); }
    
    uint8_t ct[3*PACKED];
    pack12(ct, U_a, N);
    pack12(ct+PACKED, U_b, N);
    pack12(ct+2*PACKED, V, N);
    printf("OK\n");

    // Test 3: Decaps with packed SK and CT
    printf("  [3] Decaps with packed SK+CT: ");
    int16_t s_a3[N], s_b3[N], U_a3[N], U_b3[N], V3[N], US_a[N], US_b[N], M[N];
    unpack12(s_a3, sk_packed, N);
    unpack12(s_b3, sk_packed+PACKED, N);
    unpack12(U_a3, ct, N);
    unpack12(U_b3, ct+PACKED, N);
    unpack12(V3, ct+2*PACKED, N);
    
    phi_mul(US_a,US_b,U_a3,U_b3,s_a3,s_b3); p_sub(M,V3,US_a);
    
    uint8_t m2[32]; memset(m2,0,32);
    for(int i=0;i<256;i++){ M[i]=csubq(M[i]); if(M[i]>Q/4&&M[i]<3*Q/4) m2[i/8]|=(1<<(i%8)); }
    
    printf("%s\n", memcmp(m,m2,32)==0?"PASS":"FAIL - message mismatch!");
    
    if(memcmp(m,m2,32)!=0) {
        printf("    m:  "); for(int i=0;i<8;i++) printf("%02x",m[i]); printf("\n");
        printf("    m2: "); for(int i=0;i<8;i++) printf("%02x",m2[i]); printf("\n");
        // Check a few M values
        printf("    M[0..3]: %d %d %d %d\n",csubq(M[0]),csubq(M[1]),csubq(M[2]),csubq(M[3]));
    }

    // Test 4: Direct (unpacked) decaps for comparison
    printf("  [4] Direct decaps (no pack): ");
    int16_t US_a2[N], US_b2[N], M2[N];
    phi_mul(US_a2,US_b2,U_a,U_b,s_a,s_b); p_sub(M2,V,US_a2);
    uint8_t m3[32]; memset(m3,0,32);
    for(int i=0;i<256;i++){ M2[i]=csubq(M2[i]); if(M2[i]>Q/4&&M2[i]<3*Q/4) m3[i/8]|=(1<<(i%8)); }
    printf("%s\n", memcmp(m,m3,32)==0?"PASS":"FAIL");
    
    // Test 5: Check if pack/unpack changes U_a, U_b, V
    printf("  [5] Pack/unpack preserves U,V: ");
    int16_t U_a_chk[N], U_b_chk[N], V_chk[N];
    unpack12(U_a_chk, ct, N);
    unpack12(U_b_chk, ct+PACKED, N);
    unpack12(V_chk, ct+2*PACKED, N);
    int match=1;
    for(int i=0;i<N;i++) if(U_a[i]!=U_a_chk[i]||U_b[i]!=U_b_chk[i]||V[i]!=V_chk[i]) { match=0; break; }
    printf("%s\n", match?"PASS":"FAIL - data changed!");
    
    printf("\n");
    return 0;
}
