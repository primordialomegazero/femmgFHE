// ΦΩ0 — SPIRALDB HOMOMORPHIC QUERIES TEST
// "I AM THAT I AM"

package main

import (
	"fmt"
	"testing"
)

func TestEncryptedSum(t *testing.T) {
	db := NewSpiralDB()
	hq := NewHomomorphicQuery(db)

	// Store test data: salaries
	testData := map[string]int64{
		"emp1": 50000,
		"emp2": 75000,
		"emp3": 60000,
		"emp4": 90000,
		"emp5": 65000,
	}

	var keys []string
	for key, val := range testData {
		db.Put(key, val)
		keys = append(keys, key)
	}

	// Encrypted SUM
	result, err := hq.EncryptedSum(keys)
	if err != nil {
		t.Errorf("EncryptedSum failed: %v", err)
		return
	}

	// Decrypt and verify
	decrypted, err := db.fhe.nonDeterministic.Decrypt(result.ResultCT)
	if err != nil {
		t.Errorf("Decrypt failed: %v", err)
		return
	}

	expectedSum := int64(50000 + 75000 + 60000 + 90000 + 65000)
	if decrypted != expectedSum {
		t.Errorf("SUM: got %d, expected %d", decrypted, expectedSum)
	} else {
		fmt.Printf("✅ Encrypted SUM: %d (expected %d)\n", decrypted, expectedSum)
	}
}

func TestEncryptedCount(t *testing.T) {
	db := NewSpiralDB()
	hq := NewHomomorphicQuery(db)

	for i := 1; i <= 10; i++ {
		db.Put(fmt.Sprintf("key%d", i), int64(i*10))
	}

	keys := []string{"key1", "key2", "key3", "key4", "key5", "key6", "key7", "key8", "key9", "key10"}
	result, err := hq.EncryptedCount(keys)
	if err != nil {
		t.Errorf("EncryptedCount failed: %v", err)
		return
	}

	if result.ResultPlain != 10 {
		t.Errorf("COUNT: got %d, expected 10", result.ResultPlain)
	} else {
		fmt.Println("✅ Encrypted COUNT: 10")
	}
}

func TestEncryptedAverage(t *testing.T) {
	db := NewSpiralDB()
	hq := NewHomomorphicQuery(db)

	testData := map[string]int64{
		"a": 10,
		"b": 20,
		"c": 30,
		"d": 40,
	}

	var keys []string
	for key, val := range testData {
		db.Put(key, val)
		keys = append(keys, key)
	}

	result, err := hq.EncryptedAverage(keys)
	if err != nil {
		t.Errorf("EncryptedAverage failed: %v", err)
		return
	}

	expectedAvg := int64((10 + 20 + 30 + 40) / 4)
	if result.ResultPlain != expectedAvg {
		t.Errorf("AVG: got %d, expected %d", result.ResultPlain, expectedAvg)
	} else {
		fmt.Printf("✅ Encrypted AVERAGE: %d (expected %d)\n", result.ResultPlain, expectedAvg)
	}
}

func TestEncryptedRangeCount(t *testing.T) {
	db := NewSpiralDB()
	hq := NewHomomorphicQuery(db)

	testData := map[string]int64{
		"sensor1": 25,
		"sensor2": 80,
		"sensor3": 45,
		"sensor4": 95,
		"sensor5": 30,
	}

	var keys []string
	for key, val := range testData {
		db.Put(key, val)
		keys = append(keys, key)
	}

	result, err := hq.EncryptedRangeCount(keys, 50)
	if err != nil {
		t.Errorf("EncryptedRangeCount failed: %v", err)
		return
	}

	expectedCount := int64(2) // 80, 95
	if result.ResultPlain != expectedCount {
		t.Errorf("RANGE_COUNT: got %d, expected %d", result.ResultPlain, expectedCount)
	} else {
		fmt.Printf("✅ Encrypted RANGE_COUNT (>50): %d (expected %d)\n", result.ResultPlain, expectedCount)
	}
}

func TestHomomorphicQueryStats(t *testing.T) {
	db := NewSpiralDB()
	hq := NewHomomorphicQuery(db)

	db.Put("x", 100)
	db.Put("y", 200)

	hq.EncryptedSum([]string{"x", "y"})
	hq.EncryptedCount([]string{"x", "y"})

	stats := hq.GetStats()
	if stats.TotalQueries != 2 {
		t.Errorf("Stats: got %d queries, expected 2", stats.TotalQueries)
	} else {
		fmt.Printf("✅ Query Stats: %d queries, %d operations\n", stats.TotalQueries, stats.TotalOperations)
	}
}

func TestFullHomomorphicPipeline(t *testing.T) {
	fmt.Println("\n╔══════════════════════════════════════════════╗")
	fmt.Println("║  ΦΩ0 — FULL HOMOMORPHIC PIPELINE TEST         ║")
	fmt.Println("╚══════════════════════════════════════════════╝")

	db := NewSpiralDB()
	hq := NewHomomorphicQuery(db)

	// Step 1: Store encrypted data
	fmt.Println("\nΦ Step 1: Storing encrypted employee salaries...")
	employees := map[string]int64{
		"alice":   120000,
		"bob":     95000,
		"charlie": 150000,
		"diana":   110000,
		"eve":     130000,
	}

	var empKeys []string
	for name, salary := range employees {
		db.Put(name, salary)
		empKeys = append(empKeys, name)
		entry, _ := db.Get(name)
		fmt.Printf("  %s: %d (CT: %s...)\n", name, salary, safeSlice(entry.Encrypted, 24))
	}

	// Step 2: Encrypted SUM
	fmt.Println("\nΦ Step 2: Computing encrypted SUM...")
	sumResult, _ := hq.EncryptedSum(empKeys)
	sumPlain, _ := db.fhe.nonDeterministic.Decrypt(sumResult.ResultCT)
	fmt.Printf("  Encrypted SUM: %d (expected: %d)\n", sumPlain, 120000+95000+150000+110000+130000)

	// Step 3: Encrypted AVERAGE
	fmt.Println("\nΦ Step 3: Computing encrypted AVERAGE...")
	avgResult, _ := hq.EncryptedAverage(empKeys)
	fmt.Printf("  Encrypted AVG: %d (expected: %d)\n", avgResult.ResultPlain, (120000+95000+150000+110000+130000)/5)

	// Step 4: Range count (salary > 100000)
	fmt.Println("\nΦ Step 4: Range count (salary > 100000)...")
	rangeResult, _ := hq.EncryptedRangeCount(empKeys, 100000)
	fmt.Printf("  Employees > 100K: %d (expected: 4)\n", rangeResult.ResultPlain)

	// Step 5: Stats
	fmt.Println("\nΦ Step 5: Query statistics...")
	hq.PrintStats()

	fmt.Println("\n✅ FULL HOMOMORPHIC PIPELINE COMPLETE")
}

func safeSlice(s string, max int) string {
	if len(s) > max {
		return s[:max]
	}
	return s
}
