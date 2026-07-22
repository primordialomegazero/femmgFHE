// Minimal φ-LWE debug: test φ-mul + basic encrypt/decrypt roundtrip
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <openssl/rand.h>

#define Q 3329
#define N 4  // Tiny for debugging

static int16_t csubq(int16_t a) { a -= Q; a += (a >> 15) & Q; return a; }
static void poly_add(int16_t *r, int16_t *a, int16_t *b) { for(int i=0;i<N;i++) r[i]=csubq(a[i]+b[i]); }
static void poly_sub(int16_t *r, int16_t *a, int16_t *b) { for(int i=0;i<N;i++) r[i]=csubq(a[i]-b[i]+Q); }
static void poly_mul(int16_t *r, int16_t *a, int16_t *b) {
    int32_t tmp[2*N]; memset(tmp,0,sizeof(tmp));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) tmp[i+j]+=(int32_t)a[i]*b[j];
    for(int i=0;i<N;i++) { int32_t v=(tmp[i]-tmp[i+N])%Q; if(v<0)v+=Q; r[i]=(int16_t)v; }
}

// φ-mul: (a+bφ)(c+dφ) = (ac+bd) + (ad+bc+bd)φ
static void phi_mul(int16_t *ra, int16_t *rb, int16_t *aa, int16_t *ab, int16_t *ba, int16_t *bb) {
    int16_t ac[N], bd[N], ad[N], bc[N], tmp[N];
    poly_mul(ac, aa, ba); poly_mul(bd, ab, bb);
    poly_mul(ad, aa, bb); poly_mul(bc, ab, ba);
    poly_add(ra, ac, bd);
    poly_add(tmp, ad, bc);
    poly_add(rb, tmp, bd);
}

int main() {
    // Simple test: s = (1, 0), A = (2, 0), e = (0, 0)
    int16_t s_a[N]={1,0,0,0}, s_b[N]={0}, A_a[N]={2,0,0,0}, A_b[N]={0}, e_a[N]={0}, e_b[N]={0};
    int16_t T_a[N], T_b[N];
    
    printf("φ-mul test: A=(2+0φ), s=(1+0φ)\n");
    phi_mul(T_a, T_b, A_a, A_b, s_a, s_b);
    printf("A·s = (%d+%dφ) + e(0)\n", T_a[0], T_b[0]);
    printf("Expected: (2+0φ). Got: T_a[0]=%d T_b[0]=%d\n\n", T_a[0], T_b[0]);

    // Encrypt: r=(3,0), e1=(0,0), e2=(0,0), m=encoded as bit in V
    int16_t r_a[N]={3,0,0,0}, r_b[N]={0}, e1_a[N]={0}, e1_b[N]={0}, e2_a[N]={0}, e2_b[N]={0};
    int16_t U_a[N], U_b[N], V_a[N], V_b[N];
    
    // U = A·r + e1
    phi_mul(U_a, U_b, A_a, A_b, r_a, r_b);
    poly_add(U_a, U_a, e1_a); poly_add(U_b, U_b, e1_b);
    printf("U = A·r = (%d+%dφ)\n", U_a[0], U_b[0]);
    
    // V = T·r + e2 + encode(m=1)
    int16_t TR_a[N], TR_b[N];
    phi_mul(TR_a, TR_b, T_a, T_b, r_a, r_b);
    poly_add(TR_a, TR_a, e2_a); poly_add(TR_b, TR_b, e2_b);
    int msg_bit = 1;
    V_a[0] = csubq(TR_a[0] + msg_bit * (Q/2));
    for(int i=0;i<N;i++) { V_a[i]=TR_a[i]; V_b[i]=TR_b[i]; }
    V_a[0] = csubq(TR_a[0] + msg_bit * (Q/2));
    printf("V = T·r + (Q/2)*1 = %d\n", V_a[0]);
    
    // Decrypt: V - U·s
    int16_t US_a[N], US_b[N], M[N];
    phi_mul(US_a, US_b, U_a, U_b, s_a, s_b);
    poly_sub(M, V_a, US_a);
    M[0] = csubq(M[0]);
    int recovered = (M[0] > Q/4 && M[0] < 3*Q/4) ? 1 : 0;
    printf("Decrypt: M[0]=%d, recovered bit=%d (expected %d)\n\n", M[0], recovered, msg_bit);
    
    // Check: V - U·s should be encode(m) + noise
    // U·s = (A·r)·s = A·(r·s) 
    // T·r = (A·s)·r = A·(s·r)
    // Since φ-mul is commutative, T·r = U·s!
    // So V - U·s = encode(m) + e2 - e1·s ≈ encode(m)
    printf("Verify: T·r[0]=%d, U·s[0]=%d (should be equal)\n", TR_a[0], US_a[0]);
    
    return (recovered == msg_bit) ? 0 : 1;
}
