with open('src/core/lyapunov_fhe.h', 'r') as f:
    content = f.read()

# Old: uint64_t nonce = enc_counter_.fetch_add(1) ^ (seed & 0xFFFFFFFF);
# New: uint64_t nonce = enc_counter_.fetch_add(1) ^ splitmix64_hash(seed);

old_line = '        uint64_t nonce = enc_counter_.fetch_add(1) ^ (seed & 0xFFFFFFFF);'
new_line = '''        // Splitmix64 hash of full seed for nonce uniqueness
        uint64_t hashed_seed = seed;
        hashed_seed = (hashed_seed ^ (hashed_seed >> 30)) * 0xBF58476D1CE4E5B9ULL;
        hashed_seed = (hashed_seed ^ (hashed_seed >> 27)) * 0x94D049BB133111EBULL;
        hashed_seed = hashed_seed ^ (hashed_seed >> 31);
        uint64_t nonce = enc_counter_.fetch_add(1) ^ hashed_seed;'''

count = content.count(old_line)
content = content.replace(old_line, new_line)
print(f"Replaced {count} occurrences")

with open('src/core/lyapunov_fhe.h', 'w') as f:
    f.write(content)
print("✅ Nonce fix applied")
