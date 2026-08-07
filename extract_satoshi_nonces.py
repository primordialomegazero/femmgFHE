#!/usr/bin/env python3
"""
🪐 SATOSHI NONCE EXTRACTION — BLOCKCHAIN PARSING 🪐
Parse local blk*.dat files.
Find transactions spending from Satoshi's known outputs.
Extract DER signatures (r, s) + transaction hashes (z).
Check for nonce reuse.
"""
import os, sys, struct, hashlib

# ═══════════════════════════════════════════
# secp256k1
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8

# ═══════════════════════════════════════════
# SATOSHI KNOWN PUBLIC KEYS
# ═══════════════════════════════════════════
# Block 9 coinbase pubkey (the famous Satoshi key)
SATOSHI_KEYS = {
    'block9_coinbase': {
        'x': 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c,
        'y': 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3,
        'label': 'Block 9 Coinbase (Satoshi)'
    }
}

# Also track by pubkey hash for P2PKH outputs
SATOSHI_ADDRESSES = {
    '1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa': 'Genesis',
    '12c6DSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX': 'Block 9 Output',
}

# ═══════════════════════════════════════════
# TOOLS
# ═══════════════════════════════════════════
def modinv(a, m):
    return pow(a, -1, m)

def read_varint(data, offset):
    if offset >= len(data):
        return 0, offset
    first = data[offset]
    if first < 0xFD:
        return first, offset + 1
    elif first == 0xFD:
        return struct.unpack('<H', data[offset+1:offset+3])[0], offset + 3
    elif first == 0xFE:
        return struct.unpack('<I', data[offset+1:offset+5])[0], offset + 5
    else:
        return struct.unpack('<Q', data[offset+1:offset+9])[0], offset + 9

def read_bytes(data, offset, count):
    return data[offset:offset+count], offset + count

def hash256(data):
    return hashlib.sha256(hashlib.sha256(data).digest()).digest()

def ripemd160_sha256(data):
    return hashlib.new('ripemd160', hashlib.sha256(data).digest()).digest()

def base58_check(prefix, payload):
    """Encode to base58check (for address matching)"""
    data = prefix + payload
    checksum = hash256(data)[:4]
    return base58_encode(data + checksum)

def base58_encode(data):
    alphabet = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'
    num = int.from_bytes(data, 'big')
    result = ''
    while num > 0:
        num, rem = divmod(num, 58)
        result = alphabet[rem] + result
    for byte in data:
        if byte == 0:
            result = '1' + result
        else:
            break
    return result

def pubkey_to_address(pubkey_bytes):
    """Convert 65-byte uncompressed pubkey to P2PKH address"""
    sha = hashlib.sha256(pubkey_bytes).digest()
    ripe = hashlib.new('ripemd160', sha).digest()
    return base58_check(b'\x00', ripe)

# ═══════════════════════════════════════════
# DER SIGNATURE PARSER
# ═══════════════════════════════════════════
def parse_der_signature(sig_bytes):
    """Parse DER-encoded ECDSA signature -> (r, s, hash_type)"""
    if len(sig_bytes) < 8:
        return None, None, None
    
    try:
        # 30 [total_len] 02 [r_len] [r] 02 [s_len] [s] [hash_type]
        if sig_bytes[0] != 0x30:
            return None, None, None
        
        idx = 2
        seq_len = sig_bytes[1]
        if seq_len > 0x80:
            len_bytes = seq_len - 0x80
            idx += len_bytes
        
        if idx >= len(sig_bytes) or sig_bytes[idx] != 0x02:
            return None, None, None
        
        r_len = sig_bytes[idx + 1]
        r_start = idx + 2
        r_bytes = sig_bytes[r_start:r_start + r_len]
        r = int.from_bytes(r_bytes, 'big')
        
        idx = r_start + r_len
        if idx >= len(sig_bytes) or sig_bytes[idx] != 0x02:
            return None, None, None
        
        s_len = sig_bytes[idx + 1]
        s_start = idx + 2
        s_bytes = sig_bytes[s_start:s_start + s_len]
        s = int.from_bytes(s_bytes, 'big')
        
        hash_type = sig_bytes[-1] if len(sig_bytes) > s_start + s_len else 0x01
        
        return r, s, hash_type
    except:
        return None, None, None

# ═══════════════════════════════════════════
# MAIN EXTRACTION
# ═══════════════════════════════════════════
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 SATOSHI NONCE EXTRACTION 🪐                          ║")
print("║  Parsing blk*.dat files for Satoshi transaction signatures ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

BITCOIN_DIR = os.path.expanduser('~/.bitcoin/blocks')
MAX_BLOCK = 5000  # Scan first 5000 blocks (2009-2010 era)

# Track outputs that belong to Satoshi
# (txid_hex, output_index) -> pubkey info
satoshis_utxos = {}
# Collected signatures
signatures = []

blk_files = sorted([f for f in os.listdir(BITCOIN_DIR) if f.startswith('blk') and f.endswith('.dat')])
print(f"Block files: {len(blk_files)}\n")

tx_count = 0
satoshi_tx_count = 0
blocks_parsed = 0

for blk_file in blk_files:
    filepath = os.path.join(BITCOIN_DIR, blk_file)
    file_size = os.path.getsize(filepath)
    print(f"  Reading {blk_file} ({file_size/1024/1024:.0f} MB)...")
    
    with open(filepath, 'rb') as f:
        data = f.read()
    
    offset = 0
    while offset < len(data) - 8 and blocks_parsed < MAX_BLOCK:
        # Block header
        if offset + 8 > len(data):
            break
        
        magic = struct.unpack('<I', data[offset:offset+4])[0]
        block_size = struct.unpack('<I', data[offset+4:offset+8])[0]
        offset += 8
        
        if magic not in [0xD9B4BEF9, 0x0709110B]:
            offset += 1
            continue
        
        if offset + block_size > len(data):
            break
        
        block_start = offset
        block_data = data[offset:offset + block_size]
        boff = 0
        
        # Version
        boff += 4
        # Prev hash
        prev_hash = block_data[boff:boff+32][::-1].hex()
        boff += 32
        # Merkle root
        boff += 32
        # Timestamp
        timestamp = struct.unpack('<I', block_data[boff:boff+4])[0]
        boff += 4
        # Bits
        boff += 4
        # Nonce
        boff += 4
        
        # Transaction count
        tx_count_in_block, boff = read_varint(block_data, boff)
        block_height = None
        
        for tx_idx in range(tx_count_in_block):
            if boff + 4 > len(block_data):
                break
            
            tx_start = boff
            
            # Version
            tx_ver = struct.unpack('<I', block_data[boff:boff+4])[0]
            boff += 4
            
            # Input count
            in_count, boff = read_varint(block_data, boff)
            
            inputs = []
            is_coinbase = False
            
            for i in range(in_count):
                # Previous tx hash
                prev_tx_bytes = block_data[boff:boff+32]
                prev_tx = prev_tx_bytes[::-1].hex()
                boff += 32
                
                # Previous output index
                prev_idx = struct.unpack('<I', block_data[boff:boff+4])[0]
                boff += 4
                
                # Script length & script
                script_len, boff = read_varint(block_data, boff)
                script, boff = read_bytes(block_data, boff, script_len)
                
                # Sequence
                sequence = struct.unpack('<I', block_data[boff:boff+4])[0]
                boff += 4
                
                # Check if coinbase
                if prev_tx == '0' * 64:
                    is_coinbase = True
                    if block_height is None:
                        try:
                            if script_len >= 1 and script[0] <= 4:
                                block_height = int.from_bytes(script[1:1+script[0]], 'little')
                        except:
                            pass
                
                inputs.append({
                    'prev_tx': prev_tx,
                    'prev_idx': prev_idx,
                    'script': script,
                    'is_coinbase': is_coinbase
                })
            
            # Output count
            out_count, boff = read_varint(block_data, boff)
            
            outputs = []
            for o in range(out_count):
                # Value
                value = struct.unpack('<Q', block_data[boff:boff+8])[0]
                boff += 8
                
                # Script length & script
                script_len, boff = read_varint(block_data, boff)
                script, boff = read_bytes(block_data, boff, script_len)
                
                # Parse output script
                pubkey = None
                address = None
                
                if len(script) >= 67 and script[0] == 0x41 and script[1:2] == b'\x04':
                    # P2PK: 0x41 0x04 <64 bytes uncompressed pubkey> OP_CHECKSIG
                    if script[-1] == 0xAC:
                        pk_bytes = script[2:66] if len(script) == 67 else script[1:66]
                        if len(pk_bytes) == 65 and pk_bytes[0] == 0x04:
                            px = int.from_bytes(pk_bytes[1:33], 'big')
                            py = int.from_bytes(pk_bytes[33:65], 'big')
                            pubkey = (px, py)
                            address = pubkey_to_address(pk_bytes)
                
                if len(script) == 25 and script[0] == 0x76 and script[1] == 0xA9:
                    # P2PKH: OP_DUP OP_HASH160 <20 bytes> OP_EQUALVERIFY OP_CHECKSIG
                    if script[2] == 0x14 and script[-2] == 0x88 and script[-1] == 0xAC:
                        hash160 = script[3:23]
                        address = base58_check(b'\x00', hash160)
                
                outputs.append({
                    'value': value,
                    'script': script,
                    'pubkey': pubkey,
                    'address': address
                })
            
            # Lock time
            locktime = struct.unpack('<I', block_data[boff:boff+4])[0]
            boff += 4
            
            tx_count += 1
            
            # Compute transaction hash (for this we need the raw tx)
            raw_tx = block_data[tx_start:boff]
            txid = hash256(raw_tx)[::-1].hex()
            
            # Check if any output sends to Satoshi key
            for out_idx, output in enumerate(outputs):
                if output['pubkey']:
                    px, py = output['pubkey']
                    for key_name, key_info in SATOSHI_KEYS.items():
                        if px == key_info['x'] and py == key_info['y']:
                            satoshis_utxos[(txid, out_idx)] = {
                                'value': output['value'],
                                'key_name': key_name,
                                'block_height': block_height
                            }
                            satoshi_tx_count += 1
                            print(f"    🎯 Satoshi output: {txid[:16]}... idx={out_idx} "
                                  f"value={output['value']/1e8:.8f} BTC block={block_height}")
                
                if output['address'] and output['address'] in SATOSHI_ADDRESSES:
                    satoshis_utxos[(txid, out_idx)] = {
                        'value': output['value'],
                        'key_name': SATOSHI_ADDRESSES[output['address']],
                        'block_height': block_height
                    }
                    satoshi_tx_count += 1
                    print(f"    🎯 Satoshi address: {output['address']} "
                          f"value={output['value']/1e8:.8f} BTC block={block_height}")
            
            # Check if this tx spends a Satoshi UTXO (signature extraction)
            for inp in inputs:
                if inp['is_coinbase']:
                    continue
                
                utxo_key = (inp['prev_tx'], inp['prev_idx'])
                if utxo_key in satoshis_utxos:
                    # Extract signature from input script
                    script = inp['script']
                    
                    # Find DER signature in script
                    sig_start = None
                    sig_end = None
                    
                    for i in range(len(script)):
                        if script[i] == 0x30:  # DER SEQUENCE tag
                            # Check if this looks like a signature
                            if i + 2 < len(script):
                                der_len = script[i+1]
                                if i + 2 + der_len <= len(script):
                                    # Found potential signature
                                    sig_bytes = script[i:i+2+der_len]
                                    
                                    # Check for hash type byte after sig
                                    if i + 2 + der_len < len(script) and script[i+2+der_len] in [0x01, 0x02, 0x03]:
                                        sig_bytes = script[i:i+3+der_len]
                                    
                                    r, s_val, hash_type = parse_der_signature(sig_bytes)
                                    if r is not None and s_val is not None:
                                        sig_info = {
                                            'txid': txid,
                                            'spends': utxo_key,
                                            'spends_info': satoshis_utxos[utxo_key],
                                            'r': r,
                                            's': s_val,
                                            'hash_type': hash_type,
                                            'block_height': block_height,
                                            'raw_sig': sig_bytes.hex()
                                        }
                                        signatures.append(sig_info)
                                        print(f"    ✍️  SIGNATURE FOUND! tx={txid[:16]}... "
                                              f"spends {utxo_key[0][:16]}... idx={utxo_key[1]}")
                                        print(f"       r = {hex(r)[:40]}...")
                                        print(f"       s = {hex(s_val)[:40]}...")
                                        break
        
        blocks_parsed += 1
        offset += block_size
    
    if blocks_parsed >= MAX_BLOCK:
        break

# ═══════════════════════════════════════════
# NONCE ANALYSIS
# ═══════════════════════════════════════════
print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  NONCE ANALYSIS                                            ║")
print(f"╚══════════════════════════════════════════════════════════════╝\n")

print(f"  Blocks parsed: {blocks_parsed}")
print(f"  Transactions parsed: {tx_count}")
print(f"  Satoshi outputs found: {satoshi_tx_count}")
print(f"  Satoshi spending signatures: {len(signatures)}")

if len(signatures) == 0:
    print(f"\n  ⚠️  No Satoshi spending signatures found in first {MAX_BLOCK} blocks")
    print(f"  Satoshi may have spent from addresses we're not tracking")
    print(f"  or spent after block {MAX_BLOCK}")
    print(f"\n  Try increasing MAX_BLOCK or adding more Satoshi addresses")
else:
    # Check for duplicate r values
    print(f"\n═══ NONCE REUSE CHECK ═══\n")
    
    r_map = {}
    for i, sig in enumerate(signatures):
        r = sig['r']
        if r not in r_map:
            r_map[r] = []
        r_map[r].append(i)
    
    duplicates = {r: idxs for r, idxs in r_map.items() if len(idxs) > 1}
    
    if duplicates:
        print(f"  🚨🚨🚨 NONCE REUSE DETECTED! 🚨🚨🚨\n")
        for r, idxs in duplicates.items():
            print(f"  r = {hex(r)}")
            for idx in idxs:
                sig = signatures[idx]
                print(f"    Sig #{idx}: tx={sig['txid'][:16]}... block={sig['block_height']}")
            print()
        
        # Recover private key from first duplicate pair
        print(f"═══ PRIVATE KEY RECOVERY ═══\n")
        
        idx1, idx2 = duplicates[list(duplicates.keys())[0]][:2]
        sig1 = signatures[idx1]
        sig2 = signatures[idx2]
        
        r_shared = sig1['r']
        s1 = sig1['s']
        s2 = sig2['s']
        
        print(f"  r = {hex(r_shared)}")
        print(f"  s1 = {hex(s1)}")
        print(f"  s2 = {hex(s2)}")
        print(f"\n  Need z1, z2 (transaction hashes) to complete recovery.")
        print(f"  z = double-SHA256 of the transaction (with input scripts cleared)")
        print(f"  This requires reconstructing the raw transaction for hashing.")
        
    else:
        print(f"  ✅ No nonce reuse detected ({len(r_map)} unique r values)")
        print(f"\n  All Satoshi signatures found:")
        for i, sig in enumerate(signatures):
            print(f"  #{i}: tx={sig['txid'][:32]}")
            print(f"      block={sig['block_height']}, r={hex(sig['r'])[:30]}...")
        
        # Check for small r (biased nonces)
        print(f"\n═══ NONCE BIAS CHECK ═══\n")
        biased_found = False
        for sig in signatures:
            r_bits = sig['r'].bit_length()
            if r_bits < 200:
                print(f"  ⚠️  Small r ({r_bits} bits): {hex(sig['r'])[:40]}...")
                biased_found = True
        if not biased_found:
            print(f"  ✅ All r values are full 256-bit (no obvious bias)")

