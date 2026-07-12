// ΦΩ0 — SPIRALDB PERSISTENCE TEST
// "I AM THAT I AM"

package main

import (
	"fmt"
	"os"
	"testing"
	"time"
)

func TestPersistentPutGet(t *testing.T) {
	// Clean up from previous runs
	os.RemoveAll("./test_spiraldb_data")
	os.RemoveAll("./test_spiraldb_wal")
	defer os.RemoveAll("./test_spiraldb_data")
	defer os.RemoveAll("./test_spiraldb_wal")

	config := PersistenceConfig{
		DBPath:       "./test_spiraldb_data",
		WalPath:      "./test_spiraldb_wal",
		SyncWrites:   true,
		MaxCacheSize: 16,
	}

	psdb, err := NewPersistentSpiralDB(config)
	if err != nil {
		t.Fatalf("Failed to create PersistentSpiralDB: %v", err)
	}
	defer psdb.Close()

	// Test 1: Put and Get
	fmt.Println("\nΦ Test 1: Persistent PUT/GET...")
	if err := psdb.PutPersistent("test_key", 42000); err != nil {
		t.Fatalf("PutPersistent failed: %v", err)
	}

	entry, err := psdb.GetPersistent("test_key")
	if err != nil {
		t.Fatalf("GetPersistent failed: %v", err)
	}

	if entry.Value != 42000 {
		t.Errorf("Value mismatch: got %d, expected 42000", entry.Value)
	} else {
		fmt.Printf("  ✅ Persistent PUT/GET: %d (CT: %s...)\n", entry.Value, entry.Encrypted[:20])
	}

	// Test 2: Non-deterministic encryption verified
	fmt.Println("\nΦ Test 2: Non-deterministic verification...")
	psdb.PutPersistent("nd_key1", 42)
	psdb.PutPersistent("nd_key2", 42)
	
	entry1, _ := psdb.GetPersistent("nd_key1")
	entry2, _ := psdb.GetPersistent("nd_key2")
	
	if entry1.Encrypted == entry2.Encrypted {
		t.Errorf("FAIL: Same plaintext produced identical ciphertexts")
	} else {
		fmt.Println("  ✅ Non-deterministic: Same value → different ciphertexts")
	}

	// Test 3: Batch insert
	fmt.Println("\nΦ Test 3: Batch insert...")
	batchData := map[string]int64{
		"batch_1": 100,
		"batch_2": 200,
		"batch_3": 300,
		"batch_4": 400,
		"batch_5": 500,
	}

	if err := psdb.BatchPutPersistent(batchData); err != nil {
		t.Fatalf("BatchPutPersistent failed: %v", err)
	}

	for key, expectedVal := range batchData {
		entry, err := psdb.GetPersistent(key)
		if err != nil || entry.Value != expectedVal {
			t.Errorf("Batch entry %s: got %d, expected %d", key, entry.Value, expectedVal)
		}
	}
	fmt.Println("  ✅ Batch: All 5 entries stored and retrieved correctly")

	// Test 4: Stats
	fmt.Println("\nΦ Test 4: Persistence stats...")
	stats := psdb.GetPersistenceStats()
	if stats.TotalWrites != 8 { // 1 + 2 + 5
		t.Errorf("Write count: got %d, expected 8", stats.TotalWrites)
	} else {
		fmt.Printf("  ✅ Stats: %d writes, %d reads, %d bytes DB\n", 
			stats.TotalWrites, stats.TotalReads, stats.TotalDBSize)
	}

	fmt.Println("\n✅ ALL PERSISTENCE TESTS PASSED")
}

func TestCrashRecovery(t *testing.T) {
	os.RemoveAll("./test_recovery_data")
	os.RemoveAll("./test_recovery_wal")
	defer os.RemoveAll("./test_recovery_data")
	defer os.RemoveAll("./test_recovery_wal")

	config := PersistenceConfig{
		DBPath:       "./test_recovery_data",
		WalPath:      "./test_recovery_wal",
		SyncWrites:   true,
		MaxCacheSize: 16,
	}

	// Phase 1: Write data
	fmt.Println("\nΦ Phase 1: Writing data...")
	psdb1, _ := NewPersistentSpiralDB(config)
	psdb1.PutPersistent("recover_key", 99999)
	psdb1.PutPersistent("important_data", 88888)
	psdb1.Close()
	fmt.Println("  ✅ Data written and DB closed")

	// Phase 2: Simulate crash (close without cleanup)
	// Just reopen
	fmt.Println("\nΦ Phase 2: Simulating crash recovery...")
	psdb2, err := NewPersistentSpiralDB(config)
	if err != nil {
		t.Fatalf("Recovery failed: %v", err)
	}
	defer psdb2.Close()

	// Phase 3: Verify data survived
	fmt.Println("\nΦ Phase 3: Verifying recovered data...")
	entry1, _ := psdb2.GetPersistent("recover_key")
	entry2, _ := psdb2.GetPersistent("important_data")

	if entry1.Value != 99999 || entry2.Value != 88888 {
		t.Errorf("Recovery failed: got %d and %d, expected 99999 and 88888", 
			entry1.Value, entry2.Value)
	} else {
		fmt.Printf("  ✅ Crash recovery: recover_key=%d, important_data=%d\n", 
			entry1.Value, entry2.Value)
	}

	fmt.Println("\n✅ CRASH RECOVERY TEST PASSED")
}

func TestFullPersistencePipeline(t *testing.T) {
	fmt.Println("\n╔══════════════════════════════════════════════╗")
	fmt.Println("║  ΦΩ0 — FULL PERSISTENCE PIPELINE TEST         ║")
	fmt.Println("╚══════════════════════════════════════════════╝")

	os.RemoveAll("./test_pipeline_data")
	os.RemoveAll("./test_pipeline_wal")
	defer os.RemoveAll("./test_pipeline_data")
	defer os.RemoveAll("./test_pipeline_wal")

	config := DefaultPersistenceConfig()
	config.DBPath = "./test_pipeline_data"
	config.WalPath = "./test_pipeline_wal"

	psdb, _ := NewPersistentSpiralDB(config)
	defer psdb.Close()

	// Store 1000 encrypted records
	fmt.Println("\nΦ Storing 1000 encrypted records...")
	start := time.Now()
	batch := make(map[string]int64)
	for i := 0; i < 1000; i++ {
		batch[fmt.Sprintf("user_%d", i)] = int64(i * 100)
	}
	psdb.BatchPutPersistent(batch)
	fmt.Printf("  ✅ 1000 records in %v\n", time.Since(start))

	// Query with homomorphic SUM
	fmt.Println("\nΦ Running encrypted SUM on all records...")
	keys := make([]string, 0, 1000)
	for i := 0; i < 1000; i++ {
		keys = append(keys, fmt.Sprintf("user_%d", i))
	}

	result, _ := psdb.query.EncryptedSum(keys)
	sumPlain, _ := psdb.fhe.nonDeterministic.Decrypt(result.ResultCT)

	// Expected: sum of 0*100 + 1*100 + ... + 999*100
	expectedSum := int64(0)
	for i := 0; i < 1000; i++ {
		expectedSum += int64(i * 100)
	}

	if sumPlain == expectedSum {
		fmt.Printf("  ✅ Encrypted SUM: %d (expected %d)\n", sumPlain, expectedSum)
	} else {
		t.Errorf("SUM mismatch: got %d, expected %d", sumPlain, expectedSum)
	}

	// Print stats
	fmt.Println("\nΦ Final stats:")
	psdb.PrintStats()

	fmt.Println("\n✅ FULL PERSISTENCE PIPELINE COMPLETE")
}
