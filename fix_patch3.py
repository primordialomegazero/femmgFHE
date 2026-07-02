with open('src/core/femmg_operations.h', 'r') as f:
    content = f.read()

old_line = '        result.operations = key_result ^ engine_nonce;'

# Fix: XOR with a value derived from random_iv (per-ct unique), NOT just key_a ^ key_b
# This breaks the algebraic relation: a.operations ^ b.operations = key_a ^ key_b
# Because now: result.operations = (key_a ^ key_b) ^ engine_nonce ^ (random_iv_a ^ random_iv_b)
new_line = '''        // Mix random_iv to break algebraic key extraction
        result.operations = key_result ^ engine_nonce ^ a.random_iv ^ b.random_iv;'''

content = content.replace(old_line, new_line, 2)

with open('src/core/femmg_operations.h', 'w') as f:
    f.write(content)

print("✅ Reversible patch applied (XOR with random_iv)")
