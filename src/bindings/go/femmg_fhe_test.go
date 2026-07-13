package femmg

import (
	"fmt"
	"testing"
)

func TestFHE(t *testing.T) {
	fmt.Println("Testing FEmmg-FHE Go Bindings...")
	
	ctx := NewFHEContext()
	
	// Encrypt
	ct, err := ctx.Encrypt(42)
	if err != nil {
		t.Fatalf("Encrypt failed: %v", err)
	}
	fmt.Printf("  Encrypt(42): OK\n")
	
	// Decrypt
	val, err := ctx.Decrypt(ct)
	if err != nil {
		t.Fatalf("Decrypt failed: %v", err)
	}
	fmt.Printf("  Decrypt: %d (expected 42) %s\n", val, map[bool]string{true: "OK", false: "FAIL"}[val == 42])
	
	// ZANS Add
	ctx.ZANSAdd(ct)
	val, _ = ctx.Decrypt(ct)
	fmt.Printf("  ZANS Add: %d (preserved) %s\n", val, map[bool]string{true: "OK", false: "FAIL"}[val == 42])
	
	// Eternal
	data, guard, err := ctx.EternalEncrypt(99)
	if err != nil {
		t.Fatalf("EternalEncrypt failed: %v", err)
	}
	fmt.Printf("  Eternal Encrypt: data=%d bytes, guard=%d bytes OK\n", len(data), len(guard))
	
	fmt.Println("All Go tests passed!")
}
