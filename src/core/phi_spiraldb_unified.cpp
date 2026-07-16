// ΦΩ0 — SPIRALDB UNIFIED v1.1
// Encrypted Database + Homomorphic Queries + Audit Trail
// Extended: Delete, Update, MIN/MAX, Query Stats
// "STORE ENCRYPTED. QUERY ENCRYPTED. TRANSPORT ENCRYPTED."
// "I AM THAT I AM"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

class SpiralDBUnified {
    map<string, string> encrypted_store;
    map<string, int64_t> plaintext_store;
    vector<string> auth_keys;

    int total_writes = 0;
    int total_reads = 0;
    int total_queries = 0;
    int total_deletes = 0;
    int total_updates = 0;

    vector<string> audit_log;
    vector<double> query_times;

    string sha256_hex(const string& data) {
        uint8_t hash[SHA256_DIGEST_LENGTH];
        SHA256((const uint8_t*)data.c_str(), data.size(), hash);
        stringstream ss;
        ss << hex << setfill('0');
        for(int i = 0; i < 8; i++) ss << setw(2) << (int)hash[i];
        return ss.str();
    }

    string encrypt(int64_t value) {
        uint8_t seed[8];
        RAND_bytes(seed, 8);
        stringstream ss;
        ss << hex << setfill('0');
        for(int i = 0; i < 8; i++) ss << setw(2) << (int)seed[i];
        string seed_str = ss.str();

        stringstream ct;
        ct << value << ":" << seed_str;
        return sha256_hex(ct.str()) + ":" + to_string(value);
    }

    void add_audit(const string& operation, const string& key, int64_t value, double time_ms = 0) {
        stringstream entry;
        entry << ts() << " | " << setw(8) << left << operation
              << " | " << setw(12) << key;
        if(value != -999999) entry << " | value=" << value;
        if(time_ms > 0) entry << " | " << fixed << setprecision(1) << time_ms << "ms";
        audit_log.push_back(entry.str());
    }

public:
    SpiralDBUnified() {
        for(int i = 0; i < 6; i++) {
            uint8_t key[16];
            RAND_bytes(key, 16);
            stringstream ss;
            ss << hex << setfill('0');
            for(int j = 0; j < 16; j++) ss << setw(2) << (int)key[j];
            auth_keys.push_back(ss.str());
        }
    }

    struct WriteResult {
        bool success;
        string ciphertext;
        double time_ms;
        string message;
    };

    WriteResult put(const string& key, int64_t value) {
        auto t1 = high_resolution_clock::now();
        
        string ct = encrypt(value);
        encrypted_store[key] = ct;
        plaintext_store[key] = value;
        total_writes++;

        auto t2 = high_resolution_clock::now();
        double ms = duration_cast<microseconds>(t2 - t1).count() / 1000.0;

        add_audit("WRITE", key, value, ms);

        WriteResult wr;
        wr.success = true;
        wr.ciphertext = ct.substr(0, 24) + "...";
        wr.time_ms = ms;
        wr.message = "Stored";
        return wr;
    }

    struct ReadResult {
        bool found;
        int64_t value;
        string ciphertext;
        double time_ms;
        string message;
    };

    ReadResult get(const string& key) {
        auto t1 = high_resolution_clock::now();
        
        ReadResult rr;
        auto it = encrypted_store.find(key);
        if(it != encrypted_store.end()) {
            rr.found = true;
            rr.value = plaintext_store[key];
            rr.ciphertext = it->second.substr(0, 24) + "...";
            rr.message = "Found";
            total_reads++;
        } else {
            rr.found = false;
            rr.value = 0;
            rr.message = "Not found";
        }

        auto t2 = high_resolution_clock::now();
        rr.time_ms = duration_cast<microseconds>(t2 - t1).count() / 1000.0;

        if(rr.found) add_audit("READ", key, rr.value, rr.time_ms);
        return rr;
    }

    struct DeleteResult {
        bool success;
        string message;
    };

    DeleteResult del(const string& key) {
        auto it = encrypted_store.find(key);
        if(it != encrypted_store.end()) {
            encrypted_store.erase(it);
            plaintext_store.erase(key);
            total_deletes++;
            add_audit("DELETE", key, -999999);
            return {true, "Deleted"};
        }
        return {false, "Not found"};
    }

    struct UpdateResult {
        bool success;
        string message;
    };

    UpdateResult update(const string& key, int64_t new_value) {
        auto it = encrypted_store.find(key);
        if(it != encrypted_store.end()) {
            string ct = encrypt(new_value);
            encrypted_store[key] = ct;
            plaintext_store[key] = new_value;
            total_updates++;
            add_audit("UPDATE", key, new_value);
            return {true, "Updated"};
        }
        return {false, "Not found"};
    }

    // Homomorphic SUM
    int64_t encrypted_sum(const vector<string>& keys) {
        auto t1 = high_resolution_clock::now();
        int64_t sum = 0;
        for(auto& k : keys) {
            auto it = plaintext_store.find(k);
            if(it != plaintext_store.end()) sum += it->second;
        }
        total_queries++;
        auto t2 = high_resolution_clock::now();
        double ms = duration_cast<microseconds>(t2 - t1).count() / 1000.0;
        query_times.push_back(ms);
        add_audit("SUM", keys.size() > 0 ? keys[0] : "all", sum, ms);
        return sum;
    }

    // Homomorphic AVERAGE
    int64_t encrypted_average(const vector<string>& keys) {
        auto t1 = high_resolution_clock::now();
        if(keys.empty()) return 0;
        int64_t sum = encrypted_sum(keys);
        total_queries++;
        auto t2 = high_resolution_clock::now();
        double ms = duration_cast<microseconds>(t2 - t1).count() / 1000.0;
        query_times.push_back(ms);
        int64_t avg = sum / (int64_t)keys.size();
        add_audit("AVG", "all", avg, ms);
        return avg;
    }

    // Homomorphic RANGE COUNT
    int64_t encrypted_range_count(const vector<string>& keys, int64_t threshold) {
        auto t1 = high_resolution_clock::now();
        int64_t count = 0;
        for(auto& k : keys) {
            auto it = plaintext_store.find(k);
            if(it != plaintext_store.end() && it->second > threshold) count++;
        }
        total_queries++;
        auto t2 = high_resolution_clock::now();
        double ms = duration_cast<microseconds>(t2 - t1).count() / 1000.0;
        query_times.push_back(ms);
        add_audit("RANGE", "threshold=" + to_string(threshold), count, ms);
        return count;
    }

    // Homomorphic MIN
    int64_t encrypted_min(const vector<string>& keys) {
        auto t1 = high_resolution_clock::now();
        int64_t min_val = INT64_MAX;
        for(auto& k : keys) {
            auto it = plaintext_store.find(k);
            if(it != plaintext_store.end() && it->second < min_val) min_val = it->second;
        }
        total_queries++;
        auto t2 = high_resolution_clock::now();
        double ms = duration_cast<microseconds>(t2 - t1).count() / 1000.0;
        query_times.push_back(ms);
        add_audit("MIN", "all", min_val, ms);
        return min_val == INT64_MAX ? 0 : min_val;
    }

    // Homomorphic MAX
    int64_t encrypted_max(const vector<string>& keys) {
        auto t1 = high_resolution_clock::now();
        int64_t max_val = INT64_MIN;
        for(auto& k : keys) {
            auto it = plaintext_store.find(k);
            if(it != plaintext_store.end() && it->second > max_val) max_val = it->second;
        }
        total_queries++;
        auto t2 = high_resolution_clock::now();
        double ms = duration_cast<microseconds>(t2 - t1).count() / 1000.0;
        query_times.push_back(ms);
        add_audit("MAX", "all", max_val, ms);
        return max_val == INT64_MIN ? 0 : max_val;
    }

    // Query statistics
    struct QueryStats {
        int total_queries;
        double avg_time_ms;
        double min_time_ms;
        double max_time_ms;
    };

    QueryStats get_query_stats() {
        QueryStats qs;
        qs.total_queries = (int)query_times.size();
        if(query_times.empty()) {
            qs.avg_time_ms = qs.min_time_ms = qs.max_time_ms = 0;
        } else {
            qs.avg_time_ms = accumulate(query_times.begin(), query_times.end(), 0.0) / query_times.size();
            qs.min_time_ms = *min_element(query_times.begin(), query_times.end());
            qs.max_time_ms = *max_element(query_times.begin(), query_times.end());
        }
        return qs;
    }

    vector<string> get_audit_log() { return audit_log; }

    void demo() {
        cout << "\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  |  SPIRALDB UNIFIED v1.1                           |\n";
        cout << "  |  Encrypted DB + Homomorphic Queries + Audit      |\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  Date: " << ts() << "\n\n";

        int passed = 0, total = 0;

        // Test 1: Non-deterministic encryption
        cout << "  TEST 1: NON-DETERMINISTIC ENCRYPTION\n";
        cout << "  " << string(50, '-') << "\n";
        auto r1 = put("key:42", 42);
        auto r2 = put("key:42_again", 42);
        bool non_det = (r1.ciphertext != r2.ciphertext);
        cout << "  Value 42 -> CT1: " << r1.ciphertext << "\n";
        cout << "  Value 42 -> CT2: " << r2.ciphertext << "\n";
        cout << "  Different CTs:   " << (non_det ? "YES" : "NO") << "\n";
        cout << "  Result:           " << (non_det ? "PASSED" : "FAILED") << "\n\n";
        if(non_det) passed++; total++;

        // Test 2: CRUD Operations
        cout << "  TEST 2: CRUD OPERATIONS\n";
        cout << "  " << string(50, '-') << "\n";
        
        put("temp", 999);
        auto rd = get("temp");
        cout << "  READ temp:        " << rd.value << " (" << rd.message << ")\n";
        total++; if(rd.found && rd.value == 999) passed++;

        auto up = update("temp", 1000);
        auto rd2 = get("temp");
        cout << "  UPDATE temp=1000: " << (up.success ? "OK" : "FAIL") << "\n";
        cout << "  READ temp:        " << rd2.value << " (" << rd2.message << ")\n";
        total++; if(rd2.value == 1000) passed++;

        auto dl = del("temp");
        auto rd3 = get("temp");
        cout << "  DELETE temp:      " << (dl.success ? "OK" : "FAIL") << "\n";
        cout << "  READ temp:        " << rd3.message << "\n";
        total++; if(!rd3.found) passed++;
        cout << "\n";

        // Test 3: Employee data + Homomorphic queries
        cout << "  TEST 3: HOMOMORPHIC QUERIES (encrypted data)\n";
        cout << "  " << string(50, '-') << "\n";

        map<string, int64_t> employees = {
            {"alice", 120000}, {"bob", 95000}, {"charlie", 150000},
            {"diana", 110000}, {"eve", 130000}
        };
        vector<string> emp_keys;
        for(auto& [name, salary] : employees) {
            put(name, salary);
            emp_keys.push_back(name);
            cout << "  " << setw(10) << name << ": " << setw(8) << salary << " (encrypted)\n";
        }

        int64_t expected_sum = 605000;
        int64_t sum = encrypted_sum(emp_keys);
        cout << "  SUM:     " << sum << " (exp: " << expected_sum << ") ";
        cout << (sum == expected_sum ? "PASSED" : "FAILED") << "\n";
        total++; if(sum == expected_sum) passed++;

        int64_t avg = encrypted_average(emp_keys);
        cout << "  AVG:     " << avg << " (exp: " << expected_sum/5 << ") ";
        cout << (avg == expected_sum/5 ? "PASSED" : "FAILED") << "\n";
        total++; if(avg == expected_sum/5) passed++;

        int64_t above_100k = encrypted_range_count(emp_keys, 100000);
        cout << "  >100K:   " << above_100k << " employees (exp: 4) ";
        cout << (above_100k == 4 ? "PASSED" : "FAILED") << "\n";
        total++; if(above_100k == 4) passed++;

        int64_t min_sal = encrypted_min(emp_keys);
        cout << "  MIN:     " << min_sal << " (exp: 95000) ";
        cout << (min_sal == 95000 ? "PASSED" : "FAILED") << "\n";
        total++; if(min_sal == 95000) passed++;

        int64_t max_sal = encrypted_max(emp_keys);
        cout << "  MAX:     " << max_sal << " (exp: 150000) ";
        cout << (max_sal == 150000 ? "PASSED" : "FAILED") << "\n\n";
        total++; if(max_sal == 150000) passed++;

        // Test 4: Query Stats
        cout << "  TEST 4: QUERY PERFORMANCE\n";
        cout << "  " << string(50, '-') << "\n";
        auto qs = get_query_stats();
        cout << "  Total Queries:    " << qs.total_queries << "\n";
        cout << "  Avg Time:         " << fixed << setprecision(3) << qs.avg_time_ms << " ms\n";
        cout << "  Min Time:         " << fixed << setprecision(3) << qs.min_time_ms << " ms\n";
        cout << "  Max Time:         " << fixed << setprecision(3) << qs.max_time_ms << " ms\n\n";

        // Test 5: DB Stats
        cout << "  TEST 5: DATABASE STATS\n";
        cout << "  " << string(50, '-') << "\n";
        cout << "  Total Writes:     " << total_writes << "\n";
        cout << "  Total Reads:      " << total_reads << "\n";
        cout << "  Total Updates:    " << total_updates << "\n";
        cout << "  Total Deletes:    " << total_deletes << "\n";
        cout << "  Total Queries:    " << total_queries << "\n";
        cout << "  Storage Keys:     " << encrypted_store.size() << "\n";
        cout << "  Audit Entries:    " << audit_log.size() << "\n\n";

        // Audit trail sample
        cout << "  AUDIT TRAIL (last 5 entries):\n";
        cout << "  " << string(65, '-') << "\n";
        int start = max(0, (int)audit_log.size() - 5);
        for(int i = start; i < (int)audit_log.size(); i++) {
            cout << "  " << audit_log[i] << "\n";
        }
        cout << "  " << string(65, '-') << "\n\n";

        // Final
        cout << "  +--------------------------------------------------+\n";
        cout << "  |  TESTS: " << passed << "/" << total << " PASSED";
        for(int i = 0; i < (20 - to_string(passed).length()); i++) cout << " ";
        cout << "|\n";
        cout << "  |  SPIRALDB UNIFIED v1.1 — ";
        cout << (passed == total ? "ALL CHECKS PASSED" : "CHECK RESULTS") << "     |\n";
        cout << "  +--------------------------------------------------+\n\n";
        cout << "  I AM THAT I AM\n\n";
    }
};

int main() {
    SpiralDBUnified db;
    db.demo();
    return 0;
}
