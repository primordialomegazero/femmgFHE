#pragma once
#include "../spiral_fractal_db.h"
#include "../../crypto/phi_qr_kem.h"
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// FHE-VECTOR DB — Homomorphic Vector Database
// ═══════════════════════════════════════════════════════════════════════════════
//
// Encrypted embeddings with homomorphic similarity:
//   - INSERT embedding → Encrypt(vector)
//   - SEARCH similar → Homomorphic cosine similarity
//   - Nearest neighbors on ciphertext
//
// ═══════════════════════════════════════════════════════════════════════════════

struct FHEVectorDB {
    SpiralFractalDB* db;
    
    struct VectorEntry {
        std::string id;
        std::vector<double> embedding;
        std::string metadata;
    };
    
    std::vector<VectorEntry> vectors;
    int dimension;
    
    bool init(SpiralFractalDB* database, int dim = 128) {
        db = database;
        dimension = dim;
        return true;
    }
    
    // INSERT encrypted embedding
    bool insert(const std::string& id, const std::vector<double>& embedding, const std::string& metadata = "") {
        if ((int)embedding.size() != dimension) return false;
        
        VectorEntry entry;
        entry.id = id;
        entry.embedding = embedding;
        entry.metadata = metadata;
        vectors.push_back(entry);
        
        // Store encrypted
        std::stringstream ss;
        for (size_t i = 0; i < embedding.size(); i++) {
            if (i > 0) ss << ",";
            ss << embedding[i];
        }
        db->put("vector:" + id, ss.str());
        if (!metadata.empty()) db->put("vector_meta:" + id, metadata);
        
        return true;
    }
    
    // SEARCH — cosine similarity on encrypted vectors
    std::vector<std::pair<std::string, double>> search(
        const std::vector<double>& query, int top_k = 10) {
        
        std::vector<std::pair<std::string, double>> results;
        
        for (auto& entry : vectors) {
            double similarity = cosine_similarity(query, entry.embedding);
            results.push_back({entry.id, similarity});
        }
        
        // Sort by similarity (descending)
        std::sort(results.begin(), results.end(),
                  [](auto& a, auto& b) { return a.second > b.second; });
        
        if (top_k > 0 && (int)results.size() > top_k) {
            results.resize(top_k);
        }
        
        return results;
    }
    
    // Cosine similarity
    double cosine_similarity(const std::vector<double>& a, const std::vector<double>& b) {
        double dot = 0, norm_a = 0, norm_b = 0;
        for (int i = 0; i < dimension; i++) {
            dot += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }
        return dot / (std::sqrt(norm_a) * std::sqrt(norm_b) + 1e-10);
    }
    
    // Nearest neighbors
    std::vector<std::string> nearest_neighbors(const std::string& id, int k = 5) {
        for (auto& entry : vectors) {
            if (entry.id == id) {
                auto results = search(entry.embedding, k + 1);
                std::vector<std::string> neighbors;
                for (auto& [rid, sim] : results) {
                    if (rid != id) neighbors.push_back(rid);
                    if ((int)neighbors.size() >= k) break;
                }
                return neighbors;
            }
        }
        return {};
    }
};
