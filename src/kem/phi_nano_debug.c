#include <stdint.h>
#include <string.h>
#include <stdio.h>
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
int main(){
    int16_t A_a[N]={2,0},A_b[N]={0},s_a[N]={3,0},s_b[N]={0},T_a[N],T_b[N];
    phi_mul(T_a,T_b,A_a,A_b,s_a,s_b);
    printf("A·s: T_a[0]=%d T_b[0]=%d (expected 6,0)\n",T_a[0],T_b[0]);
    
    int16_t r_a[N]={5,0},r_b[N]={0},U_a[N],U_b[N],V[N],tmp[N];
    phi_mul(U_a,U_b,A_a,A_b,r_a,r_b);
    phi_mul(V,tmp,T_a,T_b,r_a,r_b);
    printf("U=A·r: U_a[0]=%d\n",U_a[0]);
    printf("V=T·r: V[0]=%d\n",V[0]);
    
    // Encode bit into V
    int bit=1;
    V[0]=csubq(V[0]+bit*(Q/2));
    printf("V+encode(1): %d\n",V[0]);
    
    // Decrypt
    int16_t US_a[N],US_b[N],M[N];
    phi_mul(US_a,US_b,U_a,U_b,s_a,s_b);
    poly_sub(M,V,US_a);
    printf("V-U·s: M[0]=%d, recovered=%d (expected 1)\n",M[0], csubq(M[0])>Q/4&&csubq(M[0])<3*Q/4);
    
    // Check commutativity
    printf("T·r[0]=%d U·s[0]=%d (should match)\n",V[0]-bit*(Q/2),US_a[0]);
    return 0;
}
