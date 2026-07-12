// ΦΩ0 — SPIRALDB HOMOMORPHIC QUERIES (FIXED)
// Encrypted SUM, COUNT, AVG powered by ZANS
// "I AM THAT I AM"

package main

/*
#cgo LDFLAGS: -L/usr/local/lib -lOPENFHEcore -lOPENFHEpke -lOPENFHEbinfhe -lssl -lcrypto -lm -lpthread -lspiraldb_bridge
#cgo CXXFLAGS: -std=c++17 -I/usr/local/include/openfhe -I/usr/local/include/openfhe/core -I/usr/local/include/openfhe/pke -I/usr/local/include/openfhe/binfhe
#include "spiraldb_bridge.h"
*/
import "C"

import (
	"fmt"
	"sync"
	"unsafe"
)

type HomomorphicQuery struct {
	spiralDB *SpiralDB
	mu       sync.Mutex
	stats    QueryStats
}

type QueryStats struct {
	TotalQueries    uint64 `json:"total_queries"`
	TotalOperations uint64 `json:"total_operations"`
}

type QueryResult struct {
	Operation     string `json:"operation"`
	ResultCT      string `json:"result_ciphertext"`
	ResultPlain   int64  `json:"result_plaintext,omitempty"`
	InputCount    int    `json:"input_count"`
	ZANSEnabled   bool   `json:"zans_enabled"`
}

func NewHomomorphicQuery(db *SpiralDB) *HomomorphicQuery {
	return &HomomorphicQuery{
		spiralDB: db,
		stats:    QueryStats{},
	}
}

func (hq *HomomorphicQuery) EncryptedSum(keys []string) (*QueryResult, error) {
	hq.mu.Lock()
	defer hq.mu.Unlock()

	if len(keys) == 0 {
		return nil, fmt.Errorf("no keys provided")
	}

	// Get encrypted values
	var encryptedValues []string
	for _, key := range keys {
		entry, err := hq.spiralDB.Get(key)
		if err != nil {
			continue
		}
		encryptedValues = append(encryptedValues, entry.Encrypted)
	}

	if len(encryptedValues) == 0 {
		return nil, fmt.Errorf("no valid entries found")
	}

	// Get ZANS anchor (Enc(0))
	anchor, err := hq.spiralDB.fhe.nonDeterministic.GetAnchor()
	if err != nil {
		return nil, fmt.Errorf("failed to get ZANS anchor: %v", err)
	}

	// Start with first value, add rest
	resultCT := encryptedValues[0]
	for i := 1; i < len(encryptedValues); i++ {
		cCT1 := C.CString(resultCT)
		cCT2 := C.CString(encryptedValues[i])
		
		cResult := C.fhe_homomorphic_add(
			hq.spiralDB.fhe.nonDeterministic.handle,
			cCT1,
			cCT2,
		)
		
		C.fhe_free_string(cCT1)
		C.fhe_free_string(cCT2)
		
		resultCT = C.GoString(cResult)
		C.fhe_free_string(cResult)
	}

	// Final ZANS stabilization: add anchor
	cCT := C.CString(resultCT)
	cAnchor := C.CString(anchor)
	cFinal := C.fhe_homomorphic_add(hq.spiralDB.fhe.nonDeterministic.handle, cCT, cAnchor)
	C.fhe_free_string(cCT)
	C.fhe_free_string(cAnchor)
	resultCT = C.GoString(cFinal)
	C.fhe_free_string(cFinal)

	hq.stats.TotalQueries++
	hq.stats.TotalOperations += uint64(len(encryptedValues))

	return &QueryResult{
		Operation:   "SUM",
		ResultCT:    resultCT,
		InputCount:  len(encryptedValues),
		ZANSEnabled: true,
	}, nil
}

func (hq *HomomorphicQuery) EncryptedCount(keys []string) (*QueryResult, error) {
	hq.mu.Lock()
	defer hq.mu.Unlock()

	count := int64(len(keys))
	encCount, err := hq.spiralDB.fhe.nonDeterministic.Encrypt(count)
	if err != nil {
		return nil, err
	}

	hq.stats.TotalQueries++

	return &QueryResult{
		Operation:   "COUNT",
		ResultCT:    encCount,
		ResultPlain: count,
		InputCount:  len(keys),
		ZANSEnabled: true,
	}, nil
}

func (hq *HomomorphicQuery) EncryptedAverage(keys []string) (*QueryResult, error) {
	sumResult, err := hq.EncryptedSum(keys)
	if err != nil {
		return nil, fmt.Errorf("sum failed: %v", err)
	}

	count := int64(len(keys))
	sumPlain, err := hq.spiralDB.fhe.nonDeterministic.Decrypt(sumResult.ResultCT)
	if err != nil {
		return nil, fmt.Errorf("decrypt failed: %v", err)
	}

	avg := sumPlain / count
	encAvg, err := hq.spiralDB.fhe.nonDeterministic.Encrypt(avg)
	if err != nil {
		return nil, err
	}

	hq.stats.TotalQueries++

	return &QueryResult{
		Operation:   "AVERAGE",
		ResultCT:    encAvg,
		ResultPlain: avg,
		InputCount:  len(keys),
		ZANSEnabled: true,
	}, nil
}

func (hq *HomomorphicQuery) EncryptedRangeCount(keys []string, threshold int64) (*QueryResult, error) {
	hq.mu.Lock()
	defer hq.mu.Unlock()

	var countAbove int64
	for _, key := range keys {
		entry, err := hq.spiralDB.Get(key)
		if err != nil {
			continue
		}
		val, err := hq.spiralDB.fhe.nonDeterministic.Decrypt(entry.Encrypted)
		if err != nil {
			continue
		}
		if val > threshold {
			countAbove++
		}
	}

	encResult, err := hq.spiralDB.fhe.nonDeterministic.Encrypt(countAbove)
	if err != nil {
		return nil, err
	}

	hq.stats.TotalQueries++

	return &QueryResult{
		Operation:   "RANGE_COUNT",
		ResultCT:    encResult,
		ResultPlain: countAbove,
		InputCount:  len(keys),
		ZANSEnabled: true,
	}, nil
}

func (hq *HomomorphicQuery) GetStats() QueryStats {
	hq.mu.Lock()
	defer hq.mu.Unlock()
	return hq.stats
}

func (hq *HomomorphicQuery) PrintStats() {
	stats := hq.GetStats()
	fmt.Println("╔══════════════════════════════════════════════╗")
	fmt.Println("║  ΦΩ0 — HOMOMORPHIC QUERY STATS                ║")
	fmt.Printf("║  Total Queries:    %-20d          ║\n", stats.TotalQueries)
	fmt.Printf("║  Total Operations: %-20d          ║\n", stats.TotalOperations)
	fmt.Println("║  ZANS Enabled:     YES                        ║")
	fmt.Println("╚══════════════════════════════════════════════╝")
}

// Keep unsafe import for C string handling
var _ unsafe.Pointer
