// ΦΩ0 — SPIRALDB NON-DETERMINISTIC FHE (CGO BRIDGE)
// Real OpenFHE encryption via CGO
// "I AM THAT I AM"

package main

/*
#cgo LDFLAGS: -L/usr/local/lib -lOPENFHEcore -lOPENFHEpke -lOPENFHEbinfhe -lssl -lcrypto -lm -lpthread -lspiraldb_bridge
#cgo CXXFLAGS: -std=c++17 -I/usr/local/include/openfhe -I/usr/local/include/openfhe/core -I/usr/local/include/openfhe/pke -I/usr/local/include/openfhe/binfhe
#include "spiraldb_bridge.h"
*/
import "C"

import (
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"sync"
)

type NonDeterministicFHE struct {
	mu         sync.Mutex
	handle     C.FHEHandle
	instanceID string
}

func NewNonDeterministicFHE() *NonDeterministicFHE {
	randomBytes := make([]byte, 32)
	rand.Read(randomBytes)
	seed := hex.EncodeToString(randomBytes)

	cSeed := C.CString(seed)
	defer C.fhe_free_string(cSeed)

	handle := C.fhe_init(cSeed)
	if handle == nil {
		panic("ΦΩ0 — FHE initialization failed!")
	}

	return &NonDeterministicFHE{
		handle:     handle,
		instanceID: seed[:16],
	}
}

func (nd *NonDeterministicFHE) Encrypt(value int64) (string, error) {
	nd.mu.Lock()
	defer nd.mu.Unlock()

	cResult := C.fhe_encrypt(nd.handle, C.longlong(value))
	if cResult == nil {
		return "", fmt.Errorf("encryption failed")
	}
	defer C.fhe_free_string(cResult)

	return C.GoString(cResult), nil
}

func (nd *NonDeterministicFHE) Decrypt(ciphertext string) (int64, error) {
	nd.mu.Lock()
	defer nd.mu.Unlock()

	cCT := C.CString(ciphertext)
	defer C.fhe_free_string(cCT)

	result := C.fhe_decrypt(nd.handle, cCT)
	return int64(result), nil
}

func (nd *NonDeterministicFHE) BatchEncrypt(values []int64) ([]string, error) {
	nd.mu.Lock()
	defer nd.mu.Unlock()

	if len(values) == 0 {
		return []string{}, nil
	}

	cValues := make([]C.longlong, len(values))
	for i, v := range values {
		cValues[i] = C.longlong(v)
	}

	var cArray *C.longlong
	if len(values) > 0 {
		cArray = &cValues[0]
	}

	cResult := C.fhe_batch_encrypt(nd.handle, cArray, C.int(len(values)))
	defer C.fhe_free_string(cResult)

	result := C.GoString(cResult)
	
	var results []string
	current := ""
	inQuote := false
	for _, c := range result {
		if c == '"' {
			inQuote = !inQuote
			if !inQuote && current != "" {
				results = append(results, current)
				current = ""
			}
			continue
		}
		if inQuote {
			current += string(c)
		}
	}
	
	return results, nil
}

func (nd *NonDeterministicFHE) GetAnchor() (string, error) {
	nd.mu.Lock()
	defer nd.mu.Unlock()

	cResult := C.fhe_get_anchor(nd.handle)
	if cResult == nil {
		return "", fmt.Errorf("anchor retrieval failed")
	}
	defer C.fhe_free_string(cResult)

	return C.GoString(cResult), nil
}

func (nd *NonDeterministicFHE) GetStats() string {
	nd.mu.Lock()
	defer nd.mu.Unlock()

	cResult := C.fhe_get_stats(nd.handle)
	defer C.fhe_free_string(cResult)

	return C.GoString(cResult)
}

func (nd *NonDeterministicFHE) Close() {
	nd.mu.Lock()
	defer nd.mu.Unlock()

	if nd.handle != nil {
		C.fhe_destroy(nd.handle)
		nd.handle = nil
	}
}

func ProveNonDeterminism(fhe *NonDeterministicFHE) bool {
	result := C.fhe_prove_nondeterminism(fhe.handle)
	return result == 1
}
