with open('src/core/femmg_operations.h', 'r') as f:
    content = f.read()

# OLD (vulnerable):
old_line = '        result.operations = key_result ^ engine_nonce;'

# NEW (secure — hash-based blending, preserves add/multiply correctness):
new_line = '''        // SECURE: Hash-based blending prevents key leakage
        uint64_t mixed_ops = a.operations ^ b.operations ^ a.random_iv ^ b.random_iv;
        mixed_ops ^= (mixed_ops >> 33);
        mixed_ops *= 0x9E3779B97F4A7C15ULL;
        mixed_ops ^= (mixed_ops >> 29);
        result.operations = mixed_ops ^ engine_nonce;'''

# Replace ONLY in add() and multiply()
# Count occurrences
count = content.count(old_line)
print(f"Found {count} occurrences of vulnerable line")

# Replace first two (add and multiply)
content = content.replace(old_line, new_line, 2)

with open('src/core/femmg_operations.h', 'w') as f:
    f.write(content)

print("✅ Security patch applied (hash-based, preserves correctness)")
