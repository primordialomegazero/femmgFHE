// ΦΩ0 — ZANS ON LATTIGO (Go)
// Cross-library validation #5: Go-based FHE library
// "I AM THAT I AM"

package main

import (
    "fmt"
    "github.com/tuneinsight/lattigo/v5/core/rlwe"
    "github.com/tuneinsight/lattigo/v5/schemes/bgv"
)

func main() {
    fmt.Println("\n╔══════════════════════════════════════════════╗")
    fmt.Println("║  ΦΩ0 — ZANS ON LATTIGO (Go)                   ║")
    fmt.Println("║  Cross-Library Validation #5                  ║")
    fmt.Println("║  I AM THAT I AM                              ║")
    fmt.Println("╚══════════════════════════════════════════════╝")
    fmt.Println()

    // Setup BGV scheme with default parameters
    params, err := bgv.NewParametersFromLiteral(bgv.ParametersLiteral{
        LogN: 12,        // 4096
        LogQ: []int{40, 40},
        P:    []int{0x7fffffff},
    })
    if err != nil {
        fmt.Println("❌ Failed to create parameters:", err)
        // Try simpler params
        params, _ = bgv.NewParametersFromLiteral(bgv.ParametersLiteral{
            LogN: 11,     // 2048
            LogQ: []int{35},
            P:    []int{0x3fffff},
        })
    }

    fmt.Printf("Φ Library: Lattigo (Go)\n")
    fmt.Printf("Φ Scheme: BGV (LogN=%d)\n\n", params.LogN())

    // Key generation
    kgen := rlwe.NewKeyGenerator(params)
    sk := kgen.GenSecretKeyNew()
    pk := kgen.GenPublicKeyNew(sk)

    encoder := bgv.NewEncoder(params)
    encryptor := rlwe.NewEncryptor(params, pk)
    decryptor := rlwe.NewDecryptor(params, sk)
    evaluator := bgv.NewEvaluator(params, nil)

    // Create Enc(0) anchor
    zero_plaintext := bgv.NewPlaintext(params, params.MaxLevel())
    encoder.Encode([]int64{0}, zero_plaintext)
    anchor, _ := encryptor.EncryptNew(zero_plaintext)

    // Create test value: 42
    truth_plaintext := bgv.NewPlaintext(params, params.MaxLevel())
    encoder.Encode([]int64{42}, truth_plaintext)
    ct, _ := encryptor.EncryptNew(truth_plaintext)

    fmt.Println("=== ZANS: Enc(0) ADDITIONS ===")

    working := ct.CopyNew()
    checkpoints := []int{1, 10, 100, 500, 1000}
    prev := 0

    for _, target := range checkpoints {
        for j := prev; j < target; j++ {
            evaluator.Add(working, anchor, working)
        }
        
        result_plaintext := bgv.NewPlaintext(params, params.MaxLevel())
        decryptor.Decrypt(working, result_plaintext)
        result := make([]int64, 1)
        encoder.Decode(result_plaintext, result)

        fmt.Printf("  Op %d: value=%d", target, result[0])
        fmt.Printf(" (expected 42)")
        if result[0] == 42 {
            fmt.Println(" ✅")
        } else {
            fmt.Println(" ❌ CORRUPTED")
        }
        prev = target
    }

    // Enc(1) comparison
    fmt.Println("\n=== Enc(1) ADDITIONS ===")

    one_plaintext := bgv.NewPlaintext(params, params.MaxLevel())
    encoder.Encode([]int64{1}, one_plaintext)
    one_ct, _ := encryptor.EncryptNew(one_plaintext)

    std_working := ct.CopyNew()
    std_checkpoints := []int{1, 10, 50, 100}
    prev = 0

    for _, target := range std_checkpoints {
        for j := prev; j < target; j++ {
            evaluator.Add(std_working, one_ct, std_working)
        }
        
        result_plaintext := bgv.NewPlaintext(params, params.MaxLevel())
        decryptor.Decrypt(std_working, result_plaintext)
        result := make([]int64, 1)
        encoder.Decode(result_plaintext, result)

        expected := int64(42 + target)
        fmt.Printf("  Op %d: value=%d", target, result[0])
        fmt.Printf(" (expected %d)", expected)
        if result[0] == expected {
            fmt.Println(" ✅")
        } else {
            fmt.Println(" ❌ CORRUPTED")
        }
        prev = target
    }

    fmt.Println("\n╔══════════════════════════════════════════════╗")
    fmt.Println("║  LATTIGO CROSS-VALIDATION: COMPLETE           ║")
    fmt.Println("╚══════════════════════════════════════════════╝")
    fmt.Println()
    fmt.Println("╔══════════════════════════════════════════════╗")
    fmt.Println("║  ΦΩ0 — I AM THAT I AM                        ║")
    fmt.Println("╚══════════════════════════════════════════════╝")
}
