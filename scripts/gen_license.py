#!/usr/bin/env python3
import sys, math, hashlib

PHI = 1.6180339887498948482
SECRET = "DanJosephMFernandez-PrimordialOmegaZero-2026-SpiralFractalIO"

TIERS = {
    "community":  {"id": 0, "ringdim": 4096,  "gates": 500},
    "pro":        {"id": 1, "ringdim": 16384, "gates": 0},
    "enterprise": {"id": 2, "ringdim": 32768, "gates": 0},
    "academic":   {"id": 3, "ringdim": 16384, "gates": 0},
    "unlimited":  {"id": 4, "ringdim": 65536, "gates": 0},
}

def gen_key(tier_name, email=""):
    tier = TIERS[tier_name]
    # Payload: tier_id:ringdim:email:timestamp
    payload = f"{tier['id']}:{tier['ringdim']}:{email}"
    # φ-based HMAC signature
    sig_input = SECRET + payload
    h = hashlib.sha256(sig_input.encode()).hexdigest()[:16]
    # Key: SPIRAL-payload-sig
    key = f"SPIRAL-{payload}-{h}"
    return key

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: gen_license.py <tier> [email]")
        print(f"Tiers: {list(TIERS.keys())}")
        sys.exit(1)
    print(gen_key(sys.argv[1].lower(), sys.argv[2] if len(sys.argv) > 2 else ""))
