# Spiral FHE+iO License v2.1

**"Open Research, Protected Commercial Use"**

Copyright (c) 2026 Dan Joseph M. Fernandez / Primordial Omega Zero

---

## SECTION 1: Research and Academic Use (FREE)

Permission is granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to use, copy, modify, merge, publish, distribute, and sublicense copies of the Software for **non-commercial, research, academic, and educational purposes ONLY**, subject to the following conditions:

### 1.1 Attribution Required

All copies, modifications, or derivative works MUST retain:
- Original author attribution: "Dan Joseph M. Fernandez / Primordial Omega Zero"
- Link to repository: https://github.com/primordialomegazero/femmgFHE
- This license notice in all copies or substantial portions

### 1.2 Citation Required

Any academic paper, publication, or presentation using this Software MUST cite:

```
Fernandez, D.J.M. (2026). Spiral FHE+iO: Complete Homomorphic Encryption
and Indistinguishability Obfuscation System.
GitHub: primordialomegazero/femmgFHE
```

### 1.3 Open Source Derivative Works

Research modifications MUST be made publicly available under this same license.

---

## SECTION 2: Commercial Use (RESTRICTED)

### 2.1 Commercial Use Definition

"Commercial use" includes:
- Integration into products or services sold for profit
- Use within for-profit organizations for operational purposes
- SaaS platforms, cloud services, or managed services
- Consulting services utilizing the Software
- Any use generating revenue from the Software

### 2.2 Commercial License Tiers

| Tier | Price | Scope | Support |
|------|-------|-------|---------|
| Community | Free | Research only | GitHub Issues |
| Pro | Contact | FHE+iO unlimited | Email |
| Enterprise | Contact | FHE+iO + Bridge + TEE | Priority |
| Academic | Free (.edu) | FHE+iO unlimited | Email |

**Note:** Ring dimension and gate count are NOT restricted by license — they are restricted by hardware capabilities. The current implementation supports:
- FHE: 10,000 cycles verified (RingDim 16,384)
- iO: 1,000,000 gates verified (TFHE STD128)

### 2.3 How to Obtain

Contact: devilswithin13@gmail.com  
Subject: "Spiral FHE+iO — Commercial License Request"

Include: Company name, use case, number of developers.

### 2.4 Startup Exception

Small companies with <10 employees, <$500K annual revenue, and active open-source contributions are granted a 2-year royalty-free Pro license, renewable upon request.

---

## SECTION 3: Patent and IP Protection

### 3.1 Prior Art

This repository establishes public prior art for:
- GF-N encryption engine (`src/config/gf_n_encryption.h`)
- DecryptLayer bootstrap architecture (`src/fhe/decrypt_layer.h`)
- TFHE universal circuit iO (`src/io/spiral_io_tfhe.h`)
- DualGate golden projection bridge (`src/bridge/dual_gate_bridge_fixed.h`)
- φ·ψ = -1 as cryptographic invariant

### 3.2 Patent Restriction

No party may patent the core innovations. Any patent application including these mechanisms must name the original author as co-inventor with minimum 50% ownership.

---

## SECTION 4: Attribution Integrity

The names "Spiral FHE+iO", "femmgFHE", "Primordial Omega Zero", and "Dan Fernandez" MUST be preserved in all derivative works, publications, and product documentation.

---

## SECTION 5: Liability and Warranty

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

## SECTION 6: Governing Law

This license shall be governed by the laws of the Republic of the Philippines. Disputes resolved through binding arbitration in Manila, Philippines.

---

## SECTION 7: Violations

Using this Software commercially without a valid license constitutes copyright infringement. Violators have 30 days from notification to cease or obtain proper license.

---

## SECTION 8: Contact

Email: devilswithin13@gmail.com  
GitHub: https://github.com/primordialomegazero/femmgFHE

---

*This license keeps Spiral FHE+iO open for research while protecting author's rights in commercial applications.*

END OF LICENSE
