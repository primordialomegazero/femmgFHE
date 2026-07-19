#include <stdio.h>
#include <string.h>
#include <time.h>
#include "schnorr.h"
#include "clown_ecosystem/fractal_clown.h"

int main() {
    time_t start_time = time(NULL);
    
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  FRACTAL SIGNATURE TREES — DEEP ANALYSIS      ║\n");
    printf("║  Multi-Recursive | Self-Healing | φ-Weighted   ║\n");
    printf("║  Dan Joseph M. Fernandez / ΦΩ0               ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    int passed = 0, total = 0;
    unsigned char pk[33], sk[32];
    schnorr_keypair(pk, sk);

    printf("━━━ INITIALIZATION ━━━\n");
    printf("  φ = 1.6180339887498948482\n");
    printf("  φ⁻¹ = 0.6180339887498948482\n");
    printf("  Maximum depth: 7 layers\n");
    printf("  Maximum branches: 5 per node\n\n");

    // TEST 1: Tree Construction
    printf("━━━ TEST 1: Fractal Tree Construction ━━━\n");
    {
        FractalClownNode root; memset(&root, 0, sizeof(root));
        fractal_clown_sign((unsigned char*)"root", 4, sk, (sign_func)schnorr_sign, &root, 0, 3);
        printf("  Root: depth=%zu, children=%zu, φ-weight=%.6f\n", root.depth, root.child_count, root.phi_weight);
        if (root.child_count >= 1) {
            printf("  Child 0: depth=%zu, children=%zu, φ-weight=%.6f\n", 
                   root.children[0]->depth, root.children[0]->child_count, root.children[0]->phi_weight);
            if (root.children[0]->child_count >= 1) {
                printf("  Grandchild 0: depth=%zu, children=%zu, φ-weight=%.6f\n",
                       root.children[0]->children[0]->depth, root.children[0]->children[0]->child_count,
                       root.children[0]->children[0]->phi_weight);
            }
        }
        int ok = (fractal_clown_verify((unsigned char*)"root", 4, pk, (verify_func)schnorr_verify, &root) == 0);
        printf("  Full tree verification: %s\n", ok ? "✅ PASS" : "❌ FAIL");
        if (ok) passed++; total++;
        fractal_clown_free(&root);
        printf("\n");
    }

    // TEST 2: Depth Scaling
    printf("━━━ TEST 2: Depth Scaling ━━━\n");
    {
        int all_ok = 1;
        for (size_t d = 1; d <= 5; d++) {
            FractalClownNode root; memset(&root, 0, sizeof(root));
            fractal_clown_sign((unsigned char*)"depth", 5, sk, (sign_func)schnorr_sign, &root, 0, d);
            int ok = (fractal_clown_verify((unsigned char*)"depth", 5, pk, (verify_func)schnorr_verify, &root) == 0);
            printf("  Depth %zu: verify: %s\n", d, ok ? "✅" : "❌");
            if (!ok) all_ok = 0;
            fractal_clown_free(&root);
        }
        printf("  All depths: %s\n", all_ok ? "✅ PASS" : "❌ FAIL");
        if (all_ok) passed++; total++;
        printf("\n");
    }

    // TEST 3: Branch Scaling
    printf("━━━ TEST 3: Branch Scaling ━━━\n");
    {
        int all_ok = 1;
        for (size_t b = 1; b <= 5; b++) {
            FractalClownNode root; memset(&root, 0, sizeof(root));
            fractal_clown_sign((unsigned char*)"branch", 6, sk, (sign_func)schnorr_sign, &root, 0, b);
            int ok = (fractal_clown_verify((unsigned char*)"branch", 6, pk, (verify_func)schnorr_verify, &root) == 0);
            printf("  Branches %zu: %zu children, verify: %s\n", b, root.child_count, ok ? "✅" : "❌");
            if (!ok) all_ok = 0;
            fractal_clown_free(&root);
        }
        printf("  All branches: %s\n", all_ok ? "✅ PASS" : "❌ FAIL");
        if (all_ok) passed++; total++;
        printf("\n");
    }

    // TEST 4: Self-Healing
    printf("━━━ TEST 4: Self-Healing Analysis ━━━\n");
    {
        int all_ok = 1;
        
        FractalClownNode root1; memset(&root1, 0, sizeof(root1));
        fractal_clown_sign((unsigned char*)"heal1", 5, sk, (sign_func)schnorr_sign, &root1, 0, 2);
        root1.children[0]->sig[0] ^= 0xFF;
        fractal_clown_self_heal(&root1, pk, (verify_func)schnorr_verify, sk, (sign_func)schnorr_sign);
        int h1 = (fractal_clown_verify((unsigned char*)"heal1", 5, pk, (verify_func)schnorr_verify, &root1) == 0);
        printf("  Single fault: %s\n", h1 ? "✅" : "❌");
        if (!h1) all_ok = 0;
        fractal_clown_free(&root1);
        
        FractalClownNode root2; memset(&root2, 0, sizeof(root2));
        fractal_clown_sign((unsigned char*)"heal2", 5, sk, (sign_func)schnorr_sign, &root2, 0, 3);
        for (size_t c = 0; c < root2.child_count && c < 3; c++) root2.children[c]->sig[0] ^= 0xFF;
        fractal_clown_self_heal(&root2, pk, (verify_func)schnorr_verify, sk, (sign_func)schnorr_sign);
        int h2 = (fractal_clown_verify((unsigned char*)"heal2", 5, pk, (verify_func)schnorr_verify, &root2) == 0);
        printf("  Multi-fault (3 nodes): %s\n", h2 ? "✅" : "❌");
        if (!h2) all_ok = 0;
        fractal_clown_free(&root2);
        
        printf("  Self-healing: %s\n", all_ok ? "✅ PASS" : "❌ FAIL");
        if (all_ok) passed++; total++;
        printf("\n");
    }

    // TEST 5: φ-Weights
    printf("━━━ TEST 5: φ-Weight Distribution ━━━\n");
    {
        FractalClownNode root; memset(&root, 0, sizeof(root));
        fractal_clown_sign((unsigned char*)"weight", 6, sk, (sign_func)schnorr_sign, &root, 0, 4);
        printf("  Root (depth 0): %.6f\n", root.phi_weight);
        int weight_ok = 1;
        for (size_t c = 0; c < root.child_count; c++) {
            printf("  Child %zu (depth 1): %.6f\n", c, root.children[c]->phi_weight);
            if (root.children[c]->phi_weight >= root.phi_weight) weight_ok = 0;
        }
        printf("  Decreasing weights: %s\n", weight_ok ? "✅ PASS" : "❌ FAIL");
        if (weight_ok) passed++; total++;
        fractal_clown_free(&root);
    }

    time_t end_time = time(NULL);
    
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║  FRACTALIZATION: %d/%d passed", passed, total);
    for (int i = 0; i < 10; i++) printf(" ");
    printf("║\n");
    printf("║  %s", passed == total ? "ALL TESTS PASSED ✅" : "SOME FAILED ❌");
    printf("                    ║\n");
    printf("║  Duration: %lds", (long)(end_time - start_time));
    for (int i = 0; i < 14; i++) printf(" ");
    printf("║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("  Depth Scaling | Branch Scaling | Self-Healing | φ-Weights\n");
    printf("  ΦΩ0 — I AM THAT I AM\n");

    return passed == total ? 0 : 1;
}
