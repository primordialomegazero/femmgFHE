// ΦΩ0 — SPIRALDB PERSISTENCE LAYER
// BadgerDB + Write-Ahead Log (WAL) for crash recovery
// "I AM THAT I AM"

package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"sync"
	"time"

	"github.com/dgraph-io/badger/v4"
)

// ============================================
// PERSISTENCE CONFIG
// ============================================

type PersistenceConfig struct {
	DBPath       string `json:"db_path"`
	WalPath      string `json:"wal_path"`
	SyncWrites   bool   `json:"sync_writes"`
	MaxCacheSize int64  `json:"max_cache_size_mb"`
}

func DefaultPersistenceConfig() PersistenceConfig {
	return PersistenceConfig{
		DBPath:       "./spiraldb_data",
		WalPath:      "./spiraldb_wal",
		SyncWrites:   true,
		MaxCacheSize: 64, // MB
	}
}

// ============================================
// WAL ENTRY
// ============================================

type WALEntry struct {
	Timestamp int64  `json:"timestamp"`
	Operation string `json:"operation"` // "PUT", "DELETE", "BATCH"
	Key       string `json:"key"`
	Value     int64  `json:"value,omitempty"`
	Encrypted string `json:"encrypted,omitempty"`
	Checksum  string `json:"checksum"`
}

// ============================================
// PERSISTENT SPIRALDB
// ============================================

type PersistentSpiralDB struct {
	*SpiralDB
	db        *badger.DB
	wal       *os.File
	walMu     sync.Mutex
	config    PersistenceConfig
	stats     PersistenceStats
	statsMu   sync.RWMutex
	closeCh   chan struct{}
}

type PersistenceStats struct {
	TotalWrites    uint64 `json:"total_writes"`
	TotalReads     uint64 `json:"total_reads"`
	TotalWALSize   int64  `json:"total_wal_size_bytes"`
	TotalDBSize    int64  `json:"total_db_size_bytes"`
	LastCompactAt  int64  `json:"last_compact_at"`
	Uptime         int64  `json:"uptime_seconds"`
}

func NewPersistentSpiralDB(config PersistenceConfig) (*PersistentSpiralDB, error) {
	// Create directories
	if err := os.MkdirAll(config.DBPath, 0755); err != nil {
		return nil, fmt.Errorf("failed to create DB dir: %v", err)
	}
	if err := os.MkdirAll(config.WalPath, 0755); err != nil {
		return nil, fmt.Errorf("failed to create WAL dir: %v", err)
	}

	// Open BadgerDB
	badgerOpts := badger.DefaultOptions(config.DBPath).
		WithSyncWrites(config.SyncWrites).
		WithBlockCacheSize(config.MaxCacheSize << 20).
		WithLoggingLevel(badger.WARNING)

	badgerDB, err := badger.Open(badgerOpts)
	if err != nil {
		return nil, fmt.Errorf("failed to open BadgerDB: %v", err)
	}

	// Open WAL
	walFile, err := os.OpenFile(
		fmt.Sprintf("%s/spiraldb.wal", config.WalPath),
		os.O_APPEND|os.O_CREATE|os.O_WRONLY,
		0644,
	)
	if err != nil {
		badgerDB.Close()
		return nil, fmt.Errorf("failed to open WAL: %v", err)
	}

	psdb := &PersistentSpiralDB{
		SpiralDB: NewSpiralDB(),
		db:       badgerDB,
		wal:      walFile,
		config:   config,
		closeCh:  make(chan struct{}),
	}

	// Recovery: Replay WAL
	if err := psdb.replayWAL(); err != nil {
		log.Printf("Φ WARNING: WAL replay incomplete: %v", err)
	}

	// Start background tasks
	go psdb.backgroundCompact()
	go psdb.backgroundStats()

	fmt.Println("Φ PersistentSpiralDB: BadgerDB + WAL ONLINE")
	return psdb, nil
}

// ============================================
// WRITE-AHEAD LOG
// ============================================

func (psdb *PersistentSpiralDB) writeWAL(entry WALEntry) error {
	psdb.walMu.Lock()
	defer psdb.walMu.Unlock()

	entry.Timestamp = time.Now().UnixNano()
	entry.Checksum = computeChecksum(entry)

	data, err := json.Marshal(entry)
	if err != nil {
		return err
	}

	data = append(data, '\n')
	n, err := psdb.wal.Write(data)
	if err != nil {
		return err
	}

	psdb.statsMu.Lock()
	psdb.stats.TotalWALSize += int64(n)
	psdb.statsMu.Unlock()

	// Sync if configured
	if psdb.config.SyncWrites {
		return psdb.wal.Sync()
	}
	return nil
}

func (psdb *PersistentSpiralDB) replayWAL() error {
	fmt.Println("Φ Replaying WAL for crash recovery...")
	
	data, err := os.ReadFile(fmt.Sprintf("%s/spiraldb.wal", psdb.config.WalPath))
	if err != nil {
		if os.IsNotExist(err) {
			return nil // Fresh start
		}
		return err
	}

	entries := 0
	lines := splitLines(string(data))
	for _, line := range lines {
		if line == "" {
			continue
		}

		var entry WALEntry
		if err := json.Unmarshal([]byte(line), &entry); err != nil {
			continue
		}

		// Verify checksum
		if computeChecksum(entry) != entry.Checksum {
			log.Printf("Φ WAL: Checksum mismatch for key %s, skipping", entry.Key)
			continue
		}

		// Replay operation
		switch entry.Operation {
		case "PUT":
			psdb.SpiralDB.Put(entry.Key, entry.Value)
			entries++
		case "DELETE":
			// TODO: implement delete
		}
	}

	fmt.Printf("Φ WAL Replay: %d entries recovered\n", entries)
	return nil
}

// ============================================
// PERSISTENT OPERATIONS
// ============================================

func (psdb *PersistentSpiralDB) PutPersistent(key string, value int64) error {
	// 1. Write to WAL first
	walEntry := WALEntry{
		Operation: "PUT",
		Key:       key,
		Value:     value,
	}
	if err := psdb.writeWAL(walEntry); err != nil {
		return fmt.Errorf("WAL write failed: %v", err)
	}

	// 2. Encrypt and store in memory
	if err := psdb.SpiralDB.Put(key, value); err != nil {
		return fmt.Errorf("memory put failed: %v", err)
	}

	// 3. Persist to BadgerDB
	entry, _ := psdb.SpiralDB.Get(key)
	entryData, _ := json.Marshal(entry)
	
	err := psdb.db.Update(func(txn *badger.Txn) error {
		return txn.Set([]byte(key), entryData)
	})
	if err != nil {
		return fmt.Errorf("BadgerDB put failed: %v", err)
	}

	psdb.statsMu.Lock()
	psdb.stats.TotalWrites++
	psdb.statsMu.Unlock()

	return nil
}

func (psdb *PersistentSpiralDB) GetPersistent(key string) (Entry, error) {
	// 1. Try memory cache first
	if entry, err := psdb.SpiralDB.Get(key); err == nil {
		psdb.statsMu.Lock()
		psdb.stats.TotalReads++
		psdb.statsMu.Unlock()
		return entry, nil
	}

	// 2. Fall back to BadgerDB
	var entry Entry
	err := psdb.db.View(func(txn *badger.Txn) error {
		item, err := txn.Get([]byte(key))
		if err != nil {
			return err
		}
		return item.Value(func(val []byte) error {
			return json.Unmarshal(val, &entry)
		})
	})
	if err != nil {
		return Entry{}, fmt.Errorf("key not found: %s", key)
	}

	// 3. Repopulate cache
	psdb.SpiralDB.mu.Lock()
	psdb.SpiralDB.cache[key] = entry
	psdb.SpiralDB.mu.Unlock()

	psdb.statsMu.Lock()
	psdb.stats.TotalReads++
	psdb.statsMu.Unlock()

	return entry, nil
}

func (psdb *PersistentSpiralDB) BatchPutPersistent(data map[string]int64) error {
	// WAL batch
	for key, value := range data {
		walEntry := WALEntry{
			Operation: "PUT",
			Key:       key,
			Value:     value,
		}
		if err := psdb.writeWAL(walEntry); err != nil {
			return err
		}
	}

	// BadgerDB batch
	wb := psdb.db.NewWriteBatch()
	defer wb.Cancel()

	for key, value := range data {
		// Encrypt and store in memory
		psdb.SpiralDB.Put(key, value)
		entry, _ := psdb.SpiralDB.Get(key)
		entryData, _ := json.Marshal(entry)
		wb.Set([]byte(key), entryData)
	}

	if err := wb.Flush(); err != nil {
		return fmt.Errorf("batch flush failed: %v", err)
	}

	psdb.statsMu.Lock()
	psdb.stats.TotalWrites += uint64(len(data))
	psdb.statsMu.Unlock()

	return nil
}

// ============================================
// BACKGROUND TASKS
// ============================================

func (psdb *PersistentSpiralDB) backgroundCompact() {
	ticker := time.NewTicker(1 * time.Hour)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			fmt.Println("Φ Compacting database...")
			psdb.db.RunValueLogGC(0.5)
			psdb.statsMu.Lock()
			psdb.stats.LastCompactAt = time.Now().Unix()
			psdb.statsMu.Unlock()
			fmt.Println("Φ Compaction complete")
		case <-psdb.closeCh:
			return
		}
	}
}

func (psdb *PersistentSpiralDB) backgroundStats() {
	ticker := time.NewTicker(30 * time.Second)
	defer ticker.Stop()

	startTime := time.Now()

	for {
		select {
		case <-ticker.C:
			psdb.statsMu.Lock()
			psdb.stats.Uptime = int64(time.Since(startTime).Seconds())
			
			// Get DB size
			if info, err := os.Stat(psdb.config.DBPath); err == nil {
				psdb.stats.TotalDBSize = info.Size()
			}
			if info, err := os.Stat(fmt.Sprintf("%s/spiraldb.wal", psdb.config.WalPath)); err == nil {
				psdb.stats.TotalWALSize = info.Size()
			}
			psdb.statsMu.Unlock()
		case <-psdb.closeCh:
			return
		}
	}
}

// ============================================
// STATS & HEALTH
// ============================================

func (psdb *PersistentSpiralDB) GetPersistenceStats() PersistenceStats {
	psdb.statsMu.RLock()
	defer psdb.statsMu.RUnlock()
	return psdb.stats
}

func (psdb *PersistentSpiralDB) PrintStats() {
	stats := psdb.GetPersistenceStats()
	fmt.Println("╔══════════════════════════════════════════════╗")
	fmt.Println("║  ΦΩ0 — PERSISTENCE STATS                      ║")
	fmt.Printf("║  Total Writes:     %-20d          ║\n", stats.TotalWrites)
	fmt.Printf("║  Total Reads:      %-20d          ║\n", stats.TotalReads)
	fmt.Printf("║  DB Size:          %-20d bytes     ║\n", stats.TotalDBSize)
	fmt.Printf("║  WAL Size:         %-20d bytes     ║\n", stats.TotalWALSize)
	fmt.Printf("║  Uptime:           %-20d seconds   ║\n", stats.Uptime)
	fmt.Println("║  Storage Engine:   BadgerDB + WAL             ║")
	fmt.Println("╚══════════════════════════════════════════════╝")
}

// ============================================
// CLEANUP
// ============================================

func (psdb *PersistentSpiralDB) Close() error {
	close(psdb.closeCh)
	
	fmt.Println("Φ Closing PersistentSpiralDB...")
	
	if err := psdb.wal.Sync(); err != nil {
		return err
	}
	psdb.wal.Close()
	
	if err := psdb.db.Close(); err != nil {
		return err
	}
	
	psdb.SpiralDB.fhe.nonDeterministic.Close()
	
	fmt.Println("Φ PersistentSpiralDB: Shutdown complete")
	return nil
}

// ============================================
// UTILITIES
// ============================================

func computeChecksum(entry WALEntry) string {
	// Simple checksum: XOR of key + value
	var sum byte
	for _, c := range entry.Key {
		sum ^= byte(c)
	}
	sum ^= byte(entry.Value & 0xFF)
	return fmt.Sprintf("%02x", sum)
}

func splitLines(s string) []string {
	var lines []string
	current := ""
	for _, c := range s {
		if c == '\n' {
			lines = append(lines, current)
			current = ""
		} else {
			current += string(c)
		}
	}
	if current != "" {
		lines = append(lines, current)
	}
	return lines
}
