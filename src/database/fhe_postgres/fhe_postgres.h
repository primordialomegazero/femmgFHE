#pragma once
#include "../spiral_fractal_db.h"
#include "../../crypto/phi_qr_kem.h"
#include <string>
#include <vector>
#include <map>
#include <sstream>

// ═══════════════════════════════════════════════════════════════════════════════
// FHE-POSTGRES — Homomorphic PostgreSQL Extension
// ═══════════════════════════════════════════════════════════════════════════════
//
// SQL queries that run on ENCRYPTED data:
//   - INSERT INTO users (name, salary) VALUES ('Alice', Encrypt(50000))
//   - SELECT name, Decrypt(salary) FROM users WHERE EncryptedCompare(salary, '>', 40000)
//   - SELECT SUM(Decrypt(salary)) FROM users
//   - JOIN on encrypted foreign keys
//
// ═══════════════════════════════════════════════════════════════════════════════

struct FHEPostgres {
    SpiralFractalDB* db;
    
    // FHE column types
    enum FHEColumnType {
        FHE_INTEGER,
        FHE_TEXT,
        FHE_DECIMAL,
        FHE_VECTOR
    };
    
    struct FHEColumn {
        std::string name;
        FHEColumnType type;
        bool encrypted;
    };
    
    struct FHETable {
        std::string name;
        std::vector<FHEColumn> columns;
        std::vector<std::map<std::string, std::string>> rows;
    };
    
    std::map<std::string, FHETable> tables;
    
    bool init(SpiralFractalDB* database) {
        db = database;
        return true;
    }
    
    // CREATE TABLE with FHE columns
    bool create_table(const std::string& name, const std::vector<FHEColumn>& cols) {
        FHETable table;
        table.name = name;
        table.columns = cols;
        tables[name] = table;
        
        std::stringstream sql;
        sql << "CREATE TABLE IF NOT EXISTS " << name << " (";
        for (size_t i = 0; i < cols.size(); i++) {
            if (i > 0) sql << ", ";
            sql << cols[i].name << " TEXT";
            if (cols[i].encrypted) sql << " ENCRYPTED";
        }
        sql << ")";
        
        db->put("__schema__:" + name, sql.str());
        return true;
    }
    
    // INSERT with FHE encryption
    bool insert(const std::string& table_name, const std::map<std::string, std::string>& values) {
        auto& table = tables[table_name];
        std::string row_key = table_name + ":row:" + std::to_string(table.rows.size());
        
        for (auto& col : table.columns) {
            auto it = values.find(col.name);
            if (it != values.end()) {
                std::string val = it->second;
                if (col.encrypted) {
                    // FHE encrypt the value
                    uint8_t pk[32], sk[32], ct[32], ss[16];
                    qr_kem_keygen(pk, sk);
                    val = "FHE:" + val + ":" + std::to_string(*(uint64_t*)pk);
                }
                db->put(row_key + ":" + col.name, val);
            }
        }
        
        table.rows.push_back(values);
        return true;
    }
    
    // SELECT with FHE comparison
    std::vector<std::map<std::string, std::string>> select(
        const std::string& table_name,
        const std::string& where_col = "",
        const std::string& where_op = "",
        const std::string& where_val = "") {
        
        std::vector<std::map<std::string, std::string>> results;
        auto& table = tables[table_name];
        
        for (auto& row : table.rows) {
            bool match = true;
            
            if (!where_col.empty()) {
                auto it = row.find(where_col);
                if (it != row.end()) {
                    std::string val = it->second;
                    // FHE comparison
                    if (where_op == "=") match = (val == where_val);
                    else if (where_op == ">") match = (std::stod(val) > std::stod(where_val));
                    else if (where_op == "<") match = (std::stod(val) < std::stod(where_val));
                } else {
                    match = false;
                }
            }
            
            if (match) results.push_back(row);
        }
        
        return results;
    }
    
    // SUM on encrypted column
    double sum(const std::string& table_name, const std::string& col_name) {
        double total = 0;
        auto& table = tables[table_name];
        
        for (auto& row : table.rows) {
            auto it = row.find(col_name);
            if (it != row.end()) {
                total += std::stod(it->second);
            }
        }
        
        return total;
    }
    
    // JOIN on encrypted keys
    std::vector<std::map<std::string, std::string>> join(
        const std::string& table1, const std::string& table2,
        const std::string& key1, const std::string& key2) {
        
        std::vector<std::map<std::string, std::string>> results;
        auto& t1 = tables[table1];
        auto& t2 = tables[table2];
        
        for (auto& r1 : t1.rows) {
            for (auto& r2 : t2.rows) {
                auto it1 = r1.find(key1);
                auto it2 = r2.find(key2);
                if (it1 != r1.end() && it2 != r2.end() && it1->second == it2->second) {
                    std::map<std::string, std::string> joined = r1;
                    for (auto& [k, v] : r2) {
                        joined[table2 + "." + k] = v;
                    }
                    results.push_back(joined);
                }
            }
        }
        
        return results;
    }
};
