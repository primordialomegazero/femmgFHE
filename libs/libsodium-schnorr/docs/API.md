# Schnorr API Reference

## Core Functions

### schnorr_keypair
```c
int schnorr_keypair(unsigned char *pk, unsigned char *sk);
```
- **pk:** Output buffer (33 bytes). Caller must allocate.
- **sk:** Output buffer (32 bytes). Caller must allocate.
- **Returns:** 0 on success, -1 on error.
- **Memory:** No internal allocation.

### schnorr_sign
```c
int schnorr_sign(const unsigned char *msg, size_t msg_len,
                  const unsigned char *sk,
                  unsigned char *sig, size_t *sig_len);
```
- **msg:** Input message (any length).
- **sk:** 32-byte secret key.
- **sig:** Output buffer. Must be >= 65 bytes.
- **sig_len:** Input: size of sig buffer. Output: actual signature length (65).
- **Returns:** 0 on success, -1 on error.
- **Memory:** No internal allocation.

### schnorr_verify
```c
int schnorr_verify(const unsigned char *sig, size_t sig_len,
                    const unsigned char *msg, size_t msg_len,
                    const unsigned char *pk);
```
- **sig:** 65-byte signature.
- **msg:** Original message.
- **pk:** 33-byte public key.
- **Returns:** 0 if valid, -1 if invalid or error.

## Fractal Functions

### schnorr_fractal_sign
```c
int schnorr_fractal_sign(const unsigned char *msg, size_t msg_len,
                          const unsigned char *sk,
                          FractalSignature *root,
                          size_t depth, size_t branches);
```
- **depth:** Starting depth (usually 0).
- **branches:** Number of children per node.
- **⚠️ Memory:** Allocates children dynamically. MUST call schnorr_fractal_free().

### schnorr_fractal_free
```c
void schnorr_fractal_free(FractalSignature *node);
```
- **MUST be called** after fractal operations to free allocated memory.
