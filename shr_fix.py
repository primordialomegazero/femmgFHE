import sys
sys.path.insert(0, '.')

# Read the original file
with open('sha256_sat.py', 'r') as f:
    content = f.read()

# Fix SHR_word: proper right shift
old_shr = '''    def SHR_word(self, X, n):
        result = self.new_vars(self.bits)
        for i in range(self.bits):
            if i + n < self.bits:
                self.force_false(self.XOR(result[i], X[i - n]))
            else:
                self.force_false(result[i])
        return result'''

new_shr = '''    def SHR_word(self, X, n):
        result = self.new_vars(self.bits)
        for i in range(self.bits):
            if i >= n:
                self.force_false(self.XOR(result[i], X[i - n]))
            else:
                self.force_false(result[i])
        return result'''

content = content.replace(old_shr, new_shr)

with open('sha256_sat.py', 'w') as f:
    f.write(content)

print("✅ SHR_word properly fixed!")
