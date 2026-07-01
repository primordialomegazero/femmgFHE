# FEmmg-FHE v22.1 — API Reference

## Base URL
```
http://localhost:8092  (HTTP dev)
https://yourdomain.com (HTTPS prod)
```

## Authentication
```
Header: Authorization: Bearer <Φ-JWT token>
```

---

## Endpoints

### Health Check
```http
GET /health
```
**Response:**
```json
{
  "status": "ok",
  "version": "22.1.0",
  "ctu": "v5 Triple Rashomon",
  "noise": 1.82815,
  "security": "active"
}
```

---

### Register
```http
POST /
Content-Type: application/json

{
  "action": "register",
  "username": "dan",
  "password": "my-secure-password"
}
```
**Response:**
```json
{
  "action": "register",
  "client_id": "dan",
  "status": "registered"
}
```

---

### Encrypt
```http
POST /
Authorization: Bearer <token>
Content-Type: application/json

{
  "action": "fhe_encrypt",
  "client_id": "dan",
  "plaintext": 42
}
```
**Response:**
```json
{
  "action": "fhe_encrypt",
  "ciphertext_index": 0,
  "encrypted_dim0": 3.22762e10,
  "party": 0,
  "7d_banach": true,
  "ctu_v5": true
}
```

---

### Decrypt
```http
POST /
Authorization: Bearer <token>
Content-Type: application/json

{
  "action": "fhe_decrypt",
  "client_id": "dan",
  "ciphertext_index": 0
}
```
**Response:**
```json
{
  "action": "fhe_decrypt",
  "ciphertext_index": 0,
  "decrypted": 42
}
```

---

### Homomorphic Addition
```http
POST /
Authorization: Bearer <token>
Content-Type: application/json

{
  "action": "fhe_add",
  "client_id": "dan",
  "ciphertext_index_1": 0,
  "ciphertext_index_2": 1
}
```
**Response:**
```json
{
  "action": "fhe_add",
  "result_index": 2,
  "computation_blind": true
}
```

---

### Homomorphic Multiplication
```http
POST /
Authorization: Bearer <token>
Content-Type: application/json

{
  "action": "fhe_multiply",
  "client_id": "dan",
  "ciphertext_index_1": 0,
  "ciphertext_index_2": 1
}
```
**Response:**
```json
{
  "action": "fhe_multiply",
  "result_index": 3,
  "computation_blind": true
}
```

---

## Error Responses

### Auth Error
```json
{
  "status": "error",
  "http_code": 401,
  "message": "Authentication required.",
  "retry": true
}
```

### Rate Limit
```json
{
  "status": "error",
  "http_code": 429,
  "message": "Request rate exceeded. Slow down.",
  "retry": true
}
```

### Internal Error
```json
{
  "status": "error",
  "http_code": 500,
  "message": "An error occurred. Please try again.",
  "retry": true
}
```

---

## Security Headers

| Header | Value |
|--------|-------|
| `X-Content-Type-Options` | `nosniff` |
| `X-Frame-Options` | `DENY` |
| `X-XSS-Protection` | `1; mode=block` |
| `Strict-Transport-Security` | `max-age=31536000` |

---

## Rate Limits

| Mode | Limit | Burst Block |
|------|-------|-------------|
| DEV | 10,000 req/s | Never blocked |
| PROD | 100 req/s | 30 req/100ms → 5 min block |

---

## Deployment

See [DEPLOYMENT.md](DEPLOYMENT.md) for production setup.
