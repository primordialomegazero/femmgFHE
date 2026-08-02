// Fix 3×3 NOT gate for Barrington iO
// Find a matrix that swaps identity <-> cycle
#include <stdio.h>

int main() {
    // Identity matrix I
    int I[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
    // 3-cycle C
    int C[3][3] = {{0,1,0},{0,0,1},{1,0,0}};
    
    printf("Finding NOT gate: NOT(I)=C, NOT(C)=I\n\n");
    
    // Try: find matrix N such that N*I*N^-1 = C
    // Since N*I*N^-1 = I always, this is impossible via conjugation.
    printf("NOT via conjugation is impossible (conjugation preserves identity).\n");
    printf("Need a different approach.\n\n");
    
    // Approach 1: Use different encoding
    // Encode 0 as matrix A, 1 as matrix B where A and B are NOT conjugates of each other
    // Then NOT simply swaps A and B
    
    // Approach 2: Use affine transformation
    // NOT(x) = M * x * N where M,N are fixed matrices
    // For x=I: M*I*N = M*N = C
    // For x=C: M*C*N = I
    // So M*N = C and M*C*N = I
    // Substitute N = M^-1 * C into second: M*C*M^-1*C = I
    // → M*C*M^-1 = C^-1
    // C^-1 = C^2 = {{0,0,1},{1,0,0},{0,1,0}}
    // So we need M such that M*C = C^2*M
    
    printf("Approach 2: Affine NOT(x) = M * x * N\n");
    printf("Need M*C = C^2*M (M commutes C to C^2)\n");
    
    // Solve M*C = C^2*M
    // This means M maps eigenvectors of C to eigenvectors of C^2
    // C has eigenvalues 1, ω, ω^2 where ω=e^(2πi/3)
    // C^2 has same eigenvalues but ω <-> ω^2 swapped
    // So M should swap the ω and ω^2 eigenspaces
    
    // The eigenvectors are:
    // v1 = (1,1,1) for λ=1
    // v2 = (1,ω,ω^2) for λ=ω  
    // v3 = (1,ω^2,ω) for λ=ω^2
    
    // Over integers mod Q: we need a matrix that swaps v2 and v3 while fixing v1
    // This is the permutation (2 3) in the eigenbasis = reflection
    
    // In standard basis, this reflection matrix can be computed
    // For characteristic 0, it's:
    int M_try[3][3] = {
        {0, 0, 1},
        {0, 1, 0},
        {1, 0, 0}
    };
    
    printf("\nTesting M = [[0,0,1],[0,1,0],[1,0,0]]:\n");
    
    // Compute M*C
    int MC[3][3] = {0};
    for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++) MC[i][j] += M_try[i][k]*C[k][j];
    printf("M*C = [[%d,%d,%d],[%d,%d,%d],[%d,%d,%d]]\n",MC[0][0],MC[0][1],MC[0][2],MC[1][0],MC[1][1],MC[1][2],MC[2][0],MC[2][1],MC[2][2]);
    
    // Compute C^2
    int C2[3][3] = {0};
    for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++) C2[i][j] += C[i][k]*C[k][j];
    printf("C^2 = [[%d,%d,%d],[%d,%d,%d],[%d,%d,%d]]\n",C2[0][0],C2[0][1],C2[0][2],C2[1][0],C2[1][1],C2[1][2],C2[2][0],C2[2][1],C2[2][2]);
    
    // Compute C^2*M
    int C2M[3][3] = {0};
    for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++) C2M[i][j] += C2[i][k]*M_try[k][j];
    printf("C^2*M = [[%d,%d,%d],[%d,%d,%d],[%d,%d,%d]]\n",C2M[0][0],C2M[0][1],C2M[0][2],C2M[1][0],C2M[1][1],C2M[1][2],C2M[2][0],C2M[2][1],C2M[2][2]);
    
    int match = 1;
    for(int i=0;i<3;i++) for(int j=0;j<3;j++) if(MC[i][j]!=C2M[i][j]) match=0;
    printf("M*C == C^2*M? %s\n", match?"YES!":"No");
    
    if(match) {
        // Then N = M^-1 * C
        // M is its own inverse (swap rows 0 and 2)
        int N_mat[3][3] = {0};
        for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++) N_mat[i][j] += M_try[i][k]*C[k][j];
        printf("N = M*C = [[%d,%d,%d],[%d,%d,%d],[%d,%d,%d]]\n",N_mat[0][0],N_mat[0][1],N_mat[0][2],N_mat[1][0],N_mat[1][1],N_mat[1][2],N_mat[2][0],N_mat[2][1],N_mat[2][2]);
        
        printf("\nNOT gate matrix: NOT(x) = M * x * N\n");
        printf("For x=I: M*I*N = M*N = C ✓\n");
        printf("For x=C: M*C*N = M*C*M*C = C^2*M*M*C = C^2*C = C^3 = I ✓\n");
    }
    
    return 0;
}
