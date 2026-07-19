# Installation

## Prerequisites

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y build-essential libssl-dev
```

### Verify
```bash
pkg-config --libs openssl
# Output: -lssl -lcrypto
```

## Build

```bash
git clone https://github.com/primordialomegazero/libsodium-schnorr.git
cd libsodium-schnorr
make all
./test_quick
```
