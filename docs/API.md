# FEmmg-FHE API Reference

## Base URL

```
http://localhost:8092
```

## Endpoints

### GET /health

Returns system status, architecture info, and metrics.

**Response:**
```json
{
  "status": "ENTERPRISE_ACTIVE",
  "architecture": "LOCK_FREE_MULTI_METAPROGRAMMING",
  "mutex_count": 0,
  "threads": 12,
  "phi_fhe": "TRUE_HOMOMORPHIC",
  "fractal_depth": 7,
  "party_count": 14,
  "uptime_seconds": 120,
  "total_requests": 5000,
  "total_ops": 15000000,
  "noise_bits": 40.04,
  "phi": 1.618034,
  "pqc": "8/8_ALIVE",
  "anti_matter": "ACTIVE"
}
```

### POST /manifest

Single liquid endpoint for all operations.

**Actions:**

| Action | Body | Description |
|--------|------|-------------|
| `help` | `{"action":"help"}` | List all available actions |
| `encrypt` | `{"action":"encrypt","value":"42"}` | Encrypt a value |
| `add` | `{"action":"add","a":"5","b":"3"}` | Homomorphic addition |
| `multiply` | `{"action":"multiply","a":"7","b":"6"}` | Homomorphic multiplication |
| `subtract` | `{"action":"subtract","a":"10","b":"3"}` | Homomorphic subtraction |
| `fractal_chain` | `{"action":"fractal_chain","value":"10","count":"14"}` | Multi-party fractal chain |
| `tps` | `{"action":"tps","duration":"3"}` | TPS benchmark |
| `party_verify` | `{"action":"party_verify"}` | Cross-party verification |
| `pqc` | `{"action":"pqc"}` | PQC algorithm status |
| `antimatter` | `{"action":"antimatter"}` | Security shield status |

### Response Format

All responses include:
```json
{
  "action": "add",
  "true_fhe": true,
  "lock_free": true,
  "phi": 1.618034
}
```

## Error Codes

| Code | Meaning |
|------|---------|
| 200 | Success |
| 400 | Bad request (missing JSON body) |
| 429 | Rate limited (Anti-Matter shield) |
| 404 | Unknown action |

## Rate Limiting

Anti-Matter shield enforces:
- Phi-harmonic rate limit: 0.618s minimum between requests
- Lyapunov anomaly detection: blocks after stability threshold exceeded
- Schumann resonance: 7.83 Hz verification
