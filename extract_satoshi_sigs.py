#!/usr/bin/env python3
"""
🪐 EXTRACT SATOSHI SIGNATURES FROM LOCAL BLOCKCHAIN 🪐
Parse blk*.dat files, extract early block transactions,
pull out signatures targeting Satoshi's known public keys.
"""
import os, sys, struct, hashlib

# ═══════════════════════════════════════════
# CONFIGURATION
# ═══════════════════════════════════════════
BITCOIN_DIR = os.path.expanduser('~/.bitcoin/blocks')
MAX_BLOCK = 2000  # Only early blocks (Satoshi era: 2009-2010)
TARGET_PUBKEYS = {
    # Block 9 coinbase pubkey (Satoshi's known key)
    'Satoshi_Block9': (
        0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c,
        0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3
    ),
}

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

def modinv(a, m):
    return pow(a, -1, m)

# ═══════════════════════════════════════════
# BLOCK PARSER
# ═══════════════════════════════════════════
def read_varint(data, offset):
    """Read Bitcoin variable-length integer."""
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

def parse_script(script):
    """Extract pubkey or signature from script."""
    result = {'type': 'unknown', 'data': []}
    i = 0
    while i < len(script):
        opcode = script[i]
        i += 1
        if opcode == 0x00:
            result['data'].append(('OP_0', b''))
        elif 1 <= opcode <= 75:
            chunk = script[i:i+opcode]
            i += opcode
            result['data'].append(('PUSH', chunk))
        elif opcode == 0x41:  # PUSH 65 bytes (uncompressed pubkey)
            chunk = script[i:i+65]
            i += 65
            result['data'].append(('PUBKEY', chunk))
        elif opcode == 0x21:  # PUSH 33 bytes (compressed pubkey)
            chunk = script[i:i+33]
            i += 33
            result['data'].append(('PUBKEY_COMPRESSED', chunk))
        elif opcode == 0x47:  # PUSH 71 bytes (signature, typical)
            chunk = script[i:i+71]
            i += 71
            result['data'].append(('SIG', chunk))
        elif opcode == 0x48:  # PUSH 72 bytes (signature)
            chunk = script[i:i+72]
            i += 72
            result['data'].append(('SIG', chunk))
        elif opcode == 0x49:  # PUSH 73 bytes (signature)
            chunk = script[i:i+73]
            i += 73
            result['data'].append(('SIG', chunk))
        elif opcode in [0xAC, 0xAE]:  # OP_CHECKSIG, OP_CHECKMULTISIG
            result['type'] = 'checksig'
    return result

def parse_der_signature(sig_bytes):
    """Parse DER-encoded ECDSA signature into (r, s)."""
    if len(sig_bytes) < 8:
        return None, None
    # Signature format: 30 [len] 02 [r_len] [r_bytes] 02 [s_len] [s_bytes] [hash_type]
    try:
        if sig_bytes[0] != 0x30:  # DER sequence tag
            return None, None
        
        idx = 2
        if sig_bytes[1] > 0x80:
            idx += sig_bytes[1] - 0x80
        
        if idx >= len(sig_bytes) or sig_bytes[idx] != 0x02:
            return None, None
        
        r_len = sig_bytes[idx + 1]
        r_start = idx + 2
        r_bytes = sig_bytes[r_start:r_start + r_len]
        r = int.from_bytes(r_bytes, 'big')
        
        idx = r_start + r_len
        if idx >= len(sig_bytes) or sig_bytes[idx] != 0x02:
            return None, None
        
        s_len = sig_bytes[idx + 1]
        s_start = idx + 2
        s_bytes = sig_bytes[s_start:s_start + s_len]
        s = int.from_bytes(s_bytes, 'big')
        
        # Hash type is the last byte
        hash_type = sig_bytes[-1] if len(sig_bytes) > s_start + s_len else 0x01
        
        return r, s, hash_type
    except:
        return None, None, None

# ═══════════════════════════════════════════
# MAIN EXTRACTION
# ═══════════════════════════════════════════
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 EXTRACTING SATOSHI SIGNATURES FROM BLOCKS 🪐          ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

signatures = []
blocks_parsed = 0
txs_parsed = 0

# Walk through blk files
blk_files = sorted([f for f in os.listdir(BITCOIN_DIR) if f.startswith('blk') and f.endswith('.dat')])

print(f"Found {len(blk_files)} block files\n")

for blk_file in blk_files:
    filepath = os.path.join(BITCOIN_DIR, blk_file)
    file_size = os.path.getsize(filepath)
    print(f"Reading {blk_file} ({file_size:,} bytes)...")
    
    with open(filepath, 'rb') as f:
        data = f.read()
    
    offset = 0
    while offset < len(data) - 8:
        # Read block magic + size
        if offset + 8 > len(data):
            break
        
        magic = struct.unpack('<I', data[offset:offset+4])[0]
        block_size = struct.unpack('<I', data[offset+4:offset+8])[0]
        offset += 8
        
        if magic not in [0xD9B4BEF9, 0x0709110B]:  # Mainnet or testnet
            offset += 1
            continue
        
        if offset + block_size > len(data):
            break
        
        block_data = data[offset:offset + block_size]
        boff = 0
        
        # Version (4 bytes)
        version = struct.unpack('<I', block_data[boff:boff+4])[0]
        boff += 4
        
        # Previous block hash (32 bytes)
        prev_hash = block_data[boff:boff+32][::-1].hex()
        boff += 32
        
        # Merkle root (32 bytes)
        merkle_root = block_data[boff:boff+32][::-1].hex()
        boff += 32
        
        # Timestamp (4 bytes)
        timestamp = struct.unpack('<I', block_data[boff:boff+4])[0]
        boff += 4
        
        # Bits (4 bytes)
        bits = struct.unpack('<I', block_data[boff:boff+4])[0]
        boff += 4
        
        # Nonce (4 bytes)
        nonce = struct.unpack('<I', block_data[boff:boff+4])[0]
        boff += 4
        
        # Transaction count
        tx_count, boff = read_varint(block_data, boff)
        
        # Block height from coinbase
        block_height = None
        
        for tx_idx in range(tx_count):
            if boff + 4 > len(block_data):
                break
            
            tx_start = boff
            
            # Version
            tx_ver = struct.unpack('<I', block_data[boff:boff+4])[0]
            boff += 4
            
            # Input count
            in_count, boff = read_varint(block_data, boff)
            
            inputs = []
            for i in range(in_count):
                # Previous tx hash (32 bytes)
                prev_tx = block_data[boff:boff+32][::-1].hex()
                boff += 32
                
                # Previous output index (4 bytes)
                prev_idx = struct.unpack('<I', block_data[boff:boff+4])[0]
                boff += 4
                
                # Script length
                script_len, boff = read_varint(block_data, boff)
                
                # Script
                script, boff = read_bytes(block_data, boff, script_len)
                
                # Sequence
                sequence = struct.unpack('<I', block_data[boff:boff+4])[0]
                boff += 4
                
                # Extract block height from coinbase
                if prev_tx == '0' * 64 and block_height is None:
                    try:
                        block_height = int.from_bytes(script[1:script[0]+1], 'little')
                    except:
                        pass
                
                inputs.append({
                    'prev_tx': prev_tx,
                    'prev_idx': prev_idx,
                    'script': script,
                    'parsed': parse_script(script)
                })
            
            # Output count
            out_count, boff = read_varint(block_data, boff)
            
            outputs = []
            for o in range(out_count):
                # Value (8 bytes)
                value = struct.unpack('<Q', block_data[boff:boff+8])[0]
                boff += 8
                
                # Script length
                script_len, boff = read_varint(block_data, boff)
                
                # Script
                script, boff = read_bytes(block_data, boff, script_len)
                
                outputs.append({
                    'value': value,
                    'script': script,
                    'parsed': parse_script(script)
                })
            
            # Lock time
            locktime = struct.unpack('<I', block_data[boff:boff+4])[0]
            boff += 4
            
            txs_parsed += 1
            
            # Check if this transaction involves Satoshi's pubkey
            for out_idx, output in enumerate(outputs):
                for item_type, item_data in output['parsed']['data']:
                    if item_type in ['PUBKEY', 'PUBKEY_COMPRESSED']:
                        if item_type == 'PUBKEY' and len(item_data) == 65:
                            px = int.from_bytes(item_data[1:33], 'big')
                            py = int.from_bytes(item_data[33:65], 'big')
                            
                            for key_name, (target_x, target_y) in TARGET_PUBKEYS.items():
                                if px == target_x and py == target_y:
                                    print(f"\n🎯 FOUND SATOSHI OUTPUT!")
                                    print(f"  Block height: {block_height}")
                                    print(f"  Output index: {out_idx}")
                                    print(f"  Value: {output['value'] / 1e8:.8f} BTC")
                                    print(f"  Key: {key_name}")
                                    
                                    # Check inputs for signatures that spend this
                                    for inp in inputs:
                                        for item_type, item_data in inp['parsed']['data']:
                                            if item_type == 'SIG':
                                                r, s, hash_type = parse_der_signature(item_data)
                                                if r is not None:
                                                    sig_info = {
                                                        'block_height': block_height,
                                                        'r': r,
                                                        's': s,
                                                        'hash_type': hash_type,
                                                        'pubkey': (target_x, target_y),
                                                        'key_name': key_name,
                                                    }
                                                    signatures.append(sig_info)
                                                    print(f"    Signature found: r={hex(r)[:40]}...")
                                                    print(f"                      s={hex(s)[:40]}...")
                                                    print(f"                      hash_type={hash_type}")
        
        blocks_parsed += 1
        if blocks_parsed >= MAX_BLOCK:
            break
        
        offset += block_size
    
    if blocks_parsed >= MAX_BLOCK:
        break

print(f"\n═══ EXTRACTION SUMMARY ═══")
print(f"  Blocks parsed: {blocks_parsed}")
print(f"  Transactions parsed: {txs_parsed}")
print(f"  Satoshi signatures found: {len(signatures)}")

# ═══════════════════════════════════════════
# NONCE ANALYSIS
# ═══════════════════════════════════════════
if len(signatures) >= 2:
    print(f"\n═══ NONCE ANALYSIS ═══\n")
    
    # Check for duplicate r values
    r_values = {}
    for i, sig in enumerate(signatures):
        r = sig['r']
        if r not in r_values:
            r_values[r] = []
        r_values[r].append(i)
    
    duplicates = {r: indices for r, indices in r_values.items() if len(indices) > 1}
    
    if duplicates:
        print("🚨 DUPLICATE r VALUES FOUND! NONCE REUSE DETECTED! 🚨\n")
        for r, indices in duplicates.items():
            print(f"  r = {hex(r)[:40]}... appears in signatures: {indices}")
            print(f"  → Can recover private key!")
    else:
        print("  ✅ No duplicate r values (no nonce reuse detected)")
        print(f"  Total unique r values: {len(r_values)}")
    
    # Check for biased nonces (small r values might indicate weak k)
    print(f"\n═══ NONCE BIAS CHECK ═══\n")
    for sig in signatures:
        r_bits = sig['r'].bit_length()
        if r_bits < 200:
            print(f"  ⚠️  Small r ({r_bits} bits): {hex(sig['r'])[:40]}...")
    
    # List all found signatures
    print(f"\n═══ ALL SATOSHI SIGNATURES ═══\n")
    for i, sig in enumerate(signatures):
        print(f"  Sig #{i}:")
        print(f"    Block: {sig['block_height']}")
        print(f"    r = {hex(sig['r'])}")
        print(f"    s = {hex(sig['s'])}")
        print(f"    hash_type = {sig['hash_type']}")
        print()

elif len(signatures) == 1:
    print(f"\n  Found 1 Satoshi signature — need at least 2 for nonce comparison")
    sig = signatures[0]
    print(f"  r = {hex(sig['r'])}")
    print(f"  s = {hex(sig['s'])}")
    
else:
    print(f"\n  No Satoshi signatures found in first {MAX_BLOCK} blocks")
    print(f"  Possible reasons:")
    print(f"  - Satoshi used different public keys")
    print(f"  - P2PK outputs use different encoding")
    print(f"  - Try increasing MAX_BLOCK range")

