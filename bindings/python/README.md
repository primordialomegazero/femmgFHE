# FEmmg-FHE Python Bindings

Python wrapper for the FEmmg-FHE Fully Homomorphic Encryption library.

## Installation

```bash
cd bindings/python
pip install -e .
```

## Quick Start

```python
from femmgfhe import encrypt, decrypt, add, multiply

# Encrypt a number
ct = encrypt(42)

# Add while encrypted
ct2 = add(ct, 10)

# Decrypt the result
result = decrypt(ct2)  # 52
```

## Status

- **v0.1.0 (Alpha):** API design and placeholder implementation
- **Next:** CFFI bridge to C++ OpenFHE backend
- **Goal:** `pip install femmgfhe` with full FHE support

## Development

```bash
# Build C++ library first
cd ~/femmgFHE
make lib  # Creates libfemmgfhe.so

# Install Python package
cd bindings/python
pip install -e .
python -c "from femmgfhe.core import demo; demo()"
```
