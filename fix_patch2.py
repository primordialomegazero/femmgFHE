with open('src/core/femmg_operations.h', 'r') as f:
    content = f.read()

old_line = '        result.operations = key_result ^ engine_nonce;'

# Minimal fix: hash the result so engine_nonce can't be extracted algebraically
new_line = '''        // Hash-blend to prevent algebraic key extraction
        result.operations = key_result ^ engine_nonce;
        result.operations ^= (result.operations >> 33);
        result.operations *= 0x9E3779B97F4A7C15ULL;
        result.operations ^= (result.operations >> 29);
        result.operations ^= engine_nonce;'''

content = content.replace(old_line, new_line, 2)

with open('src/core/femmg_operations.h', 'w') as f:
    f.write(content)

print("✅ Minimal hash patch applied")
