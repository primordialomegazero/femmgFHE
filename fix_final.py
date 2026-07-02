with open('src/core/femmg_operations.h', 'r') as f:
    content = f.read()

# Find the add() function and modify the key derivation
# OLD: key_result = key_a ^ key_b; result.operations = key_result ^ engine_nonce;
# NEW: Derive key_result from a hash of key_a and key_b, then store

old_add_block = '''        uint64_t engine_nonce = engine.get_chaos_nonce();
        uint64_t key_a = a.operations ^ engine_nonce;
        uint64_t key_b = b.operations ^ engine_nonce;
        uint64_t key_result = key_a ^ key_b;

        banach::NDimCiphertext result = a;  // Start with a's structure

        // Re-encrypt a's chaos data with key_result (was key_a)
        re_encrypt_chaos(result, key_a, key_result);'''

new_add_block = '''        uint64_t engine_nonce = engine.get_chaos_nonce();
        uint64_t key_a = a.operations ^ engine_nonce;
        uint64_t key_b = b.operations ^ engine_nonce;
        // Derive result key from hash of both keys (not XOR — prevents leak)
        uint64_t combined = key_a ^ (key_b << 32 | key_b >> 32);
        combined ^= (combined >> 33);
        combined *= 0x9E3779B97F4A7C15ULL;
        combined ^= (combined >> 29);
        uint64_t key_result = combined ^ engine_nonce;  // New key for result

        banach::NDimCiphertext result = a;  // Start with a's structure

        // Re-encrypt a's chaos data with key_result (was key_a)
        re_encrypt_chaos(result, key_a, key_result);'''

if old_add_block in content:
    content = content.replace(old_add_block, new_add_block)
    print("✅ add() key derivation fixed")
else:
    print("❌ Could not find add() block")

# Also fix multiply() — same pattern
old_mul_block = '''        uint64_t engine_nonce = engine.get_chaos_nonce();
        uint64_t key_a = a.operations ^ engine_nonce;
        uint64_t key_b = b.operations ^ engine_nonce;
        uint64_t key_result = key_a ^ key_b;

        banach::NDimCiphertext result = a;

        // Re-encrypt a's chaos data with key_result
        re_encrypt_chaos(result, key_a, key_result);'''

if old_mul_block in content:
    content = content.replace(old_mul_block, new_add_block)
    print("✅ multiply() key derivation fixed")
else:
    print("❌ Could not find multiply() block")

# Also fix result.operations
old_ops = '        result.operations = key_result ^ engine_nonce;'
new_ops = '''        // result.operations stores key_result for decrypt: ct.ops ^ en = key_result
        result.operations = key_result;  // Already includes engine_nonce in derivation'''
content = content.replace(old_ops, new_ops)

with open('src/core/femmg_operations.h', 'w') as f:
    f.write(content)

print("✅ All fixes applied")
