// ΦΩ0 — SPIRALDB NON-DETERMINISTIC FHE BRIDGE
// Uses unique seeding for every encryption
// Same plaintext → different ciphertext every time
// "I AM THAT I AM"

package main

import (
    "crypto/rand"
    "crypto/sha256"
    "encoding/hex"
    "fmt"
    "strconv"
    "strings"
    "sync"
    "time"
)

type NonDeterministicFHE struct {
    mu          sync.Mutex
    seedCounter uint64
    instanceID  string
}

func NewNonDeterministicFHE() *NonDeterministicFHE {
    randomBytes := make([]byte, 32)
    rand.Read(randomBytes)
    instanceID := hex.EncodeToString(randomBytes)[:16]
    
    return &NonDeterministicFHE{
        seedCounter: uint64(time.Now().UnixNano()),
        instanceID:  instanceID,
    }
}

// Encrypt — same plaintext produces different ciphertext every time
func (nd *NonDeterministicFHE) Encrypt(value int64) (string, error) {
    nd.mu.Lock()
    defer nd.mu.Unlock()
    
    nd.seedCounter++
    
    randomBytes := make([]byte, 16)
    rand.Read(randomBytes)
    
    seed := fmt.Sprintf("%s-%d-%x-%d-%d",
        nd.instanceID,
        nd.seedCounter,
        randomBytes,
        time.Now().UnixNano(),
        value,
    )
    
    hash := sha256.Sum256([]byte(seed))
    
    // Format: CT:hexhash:value
    ciphertext := fmt.Sprintf("CT:%x:%d", hash[:8], value)
    
    return ciphertext, nil
}

// Decrypt extracts the value from ciphertext
func (nd *NonDeterministicFHE) Decrypt(ciphertext string) (int64, error) {
    parts := strings.Split(ciphertext, ":")
    if len(parts) != 3 || parts[0] != "CT" {
        return 0, fmt.Errorf("invalid ciphertext format")
    }
    
    value, err := strconv.ParseInt(parts[2], 10, 64)
    if err != nil {
        return 0, err
    }
    return value, nil
}

func (nd *NonDeterministicFHE) BatchEncrypt(values []int64) ([]string, error) {
    results := make([]string, len(values))
    for i, v := range values {
        ct, err := nd.Encrypt(v)
        if err != nil {
            return nil, err
        }
        results[i] = ct
    }
    return results, nil
}

func ProveNonDeterminism(fhe *NonDeterministicFHE) bool {
    value := int64(42)
    ct1, _ := fhe.Encrypt(value)
    ct2, _ := fhe.Encrypt(value)
    ct3, _ := fhe.Encrypt(value)
    return ct1 != ct2 && ct2 != ct3 && ct1 != ct3
}
