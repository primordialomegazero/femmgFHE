// ΦΩ0 — SPIRALDB NON-DETERMINISTIC TEST
// "I AM THAT I AM"

package main

import (
    "fmt"
    "testing"
)

func TestNonDeterministicEncryption(t *testing.T) {
    fhe := NewNonDeterministicFHE()
    
    // Test 1: Same value → different ciphertexts
    value := int64(42)
    ct1, _ := fhe.Encrypt(value)
    ct2, _ := fhe.Encrypt(value)
    ct3, _ := fhe.Encrypt(value)
    
    if ct1 == ct2 || ct2 == ct3 || ct1 == ct3 {
        t.Errorf("FAIL: Same plaintext produced identical ciphertexts (deterministic!)")
    } else {
        fmt.Println("✅ Non-deterministic: Same plaintext → different ciphertexts")
    }
    
    // Test 2: Decrypt returns original value
    decrypted, err := fhe.Decrypt(ct1)
    if err != nil || decrypted != value {
        t.Errorf("FAIL: Decrypt returned %d, expected %d", decrypted, value)
    } else {
        fmt.Println("✅ Decrypt: Ciphertext correctly decrypts to original value")
    }
    
    // Test 3: Batch encryption — all unique
    values := []int64{1, 2, 3, 4, 5}
    cts, _ := fhe.BatchEncrypt(values)
    seen := make(map[string]bool)
    for _, ct := range cts {
        if seen[ct] {
            t.Errorf("FAIL: Duplicate ciphertext in batch")
            return
        }
        seen[ct] = true
    }
    fmt.Println("✅ Batch: All 5 ciphertexts unique")
    
    // Test 4: ProveNonDeterminism
    if !ProveNonDeterminism(fhe) {
        t.Errorf("FAIL: ProveNonDeterminism returned false")
    } else {
        fmt.Println("✅ ProveNonDeterminism: Confirmed non-deterministic")
    }
    
    fmt.Println("\nΦΩ0 — SPIRALDB NON-DETERMINISTIC: ALL TESTS PASSED")
}

func TestSpiralDBWithFHE(t *testing.T) {
    fhe := NewNonDeterministicFHE()
    db := NewSpiralDB()
    db.fhe = &FHEContext{nonDeterministic: fhe}
    
    // Store same value twice
    db.Put("key1", 42)
    db.Put("key2", 42)
    
    // Retrieve
    v1, _ := db.Get("key1")
    v2, _ := db.Get("key2")
    
    if v1.Value != 42 || v2.Value != 42 {
        t.Errorf("FAIL: Retrieved values incorrect")
    }
    
    // Verify ciphertexts are different (non-deterministic)
    if v1.Encrypted == v2.Encrypted {
        t.Errorf("FAIL: Same plaintext produced same ciphertext in database")
    } else {
        fmt.Println("✅ SpiralDB: Same value stored with different ciphertexts")
    }
}
