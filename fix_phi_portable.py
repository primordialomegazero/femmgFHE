with open('src/security/memory_guard.h', 'r') as f:
    content = f.read()

# Replace: uint64_t phi_bits; memcpy(&phi_bits, &PHI, sizeof(double));
# With portable: union of double/uint64_t
old = '''        uint64_t phi_bits;
        std::memcpy(&phi_bits, &PHI, sizeof(double));'''

new = '''        // Portable φ bits — IEEE 754 double: 0x3FF9E3779B97F4A7
        // Uses union to avoid architecture-dependent memcpy
        union { double d; uint64_t u; } phi_union;
        phi_union.d = PHI;
        uint64_t phi_bits = phi_union.u;'''

content = content.replace(old, new)
print(f"Replaced: {old in content} → {new in content}")

with open('src/security/memory_guard.h', 'w') as f:
    f.write(content)
print("✅ memory_guard.h — Portable φ bits via union")
