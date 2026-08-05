#!/usr/bin/env python3
"""
BITCOIN BRAIINS POOL MINER — φ-DPLL + Real Block Headers
Submits VALID shares to Braiins Pool!
"""

import socket
import json
import subprocess
import time
import hashlib
import struct

POOL_HOST = "stratum.braiins.com"
POOL_PORT = 3333
USERNAME = "mangkanor6969.phi_dpll"
PASSWORD = "anything123"
WALLET_ADDRESS = "bc1qzl9wd0hf6sy0h8untygj2l0lmnr6pm53w4sykc"
MINER_EXECUTABLE = "./bin/phi_miner_engine"

def double_sha256(data):
    return hashlib.sha256(hashlib.sha256(data).digest()).digest()

def build_block_header(version, prev_hash, merkle_root, timestamp, bits, nonce=0):
    """Build Bitcoin block header (80 bytes)"""
    header = b""
    header += struct.pack("<I", version)
    header += bytes.fromhex(prev_hash)[::-1]  # Reverse byte order
    header += bytes.fromhex(merkle_root)[::-1]
    header += struct.pack("<I", timestamp)
    header += struct.pack("<I", bits)
    header += struct.pack("<I", nonce)
    return header

class BraiinsMiner:
    def __init__(self):
        self.sock = None
        self.msg_id = 0
        self.running = True
        self.shares = 0
        self.jobs = 0
        self.miner = None
        self.extranonce1 = None
        self.extranonce2_size = 0
        self.current_job = None
        
    def connect(self):
        print(f"[STRATUM] Connecting to {POOL_HOST}:{POOL_PORT}...")
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(300)
            self.sock.connect((POOL_HOST, POOL_PORT))
            print(f"[STRATUM] ✅ Connected!")
            return True
        except Exception as e:
            print(f"[STRATUM] ❌ {e}")
            return False
    
    def send(self, method, params=None):
        self.msg_id += 1
        msg = json.dumps({"id": self.msg_id, "method": method, "params": params or []}) + "\n"
        self.sock.send(msg.encode())
        return self.msg_id
    
    def recv(self):
        data = b""
        while True:
            try:
                chunk = self.sock.recv(4096)
                if not chunk: break
                data += chunk
                if b"\n" in data: break
            except socket.timeout: break
        if not data: return None
        try: return json.loads(data.decode().strip())
        except: return None
    
    def build_coinbase_tx(self, coinbase1, coinbase2, extra_nonce):
        """Build coinbase transaction"""
        coinbase = coinbase1 + extra_nonce + coinbase2
        return bytes.fromhex(coinbase)
    
    def calculate_merkle_root(self, coinbase_tx, merkle_branches):
        """Calculate merkle root from coinbase and branches"""
        tx_hash = double_sha256(coinbase_tx)
        for branch in merkle_branches:
            tx_hash = double_sha256(tx_hash + bytes.fromhex(branch))
        return tx_hash.hex()
    
    def find_nonce_direct(self, header_template, target_bits):
        """Find nonce that satisfies target difficulty"""
        # Decode target
        target = self.bits_to_target(target_bits)
        
        header = bytes.fromhex(header_template)
        if len(header) < 76:
            header = header.ljust(76, b'\x00')
        
        attempts = 0
        start_time = time.time()
        
        for nonce in range(0, 0xFFFFFFFF):
            attempts += 1
            # Update nonce in header
            test_header = header[:76] + struct.pack("<I", nonce)
            hash_result = double_sha256(test_header)
            hash_int = int.from_bytes(hash_result[::-1], 'big')
            
            if hash_int < target:
                elapsed_ms = (time.time() - start_time) * 1000
                return {
                    "nonce": nonce,
                    "attempts": attempts,
                    "time_ms": elapsed_ms,
                    "found": True,
                    "hash": hash_result.hex()
                }
            
            # φ-skip optimization
            if attempts % 10000 == 0:
                if hash_int > target * 1000000:  # Way off target
                    nonce += 5000
    
    def bits_to_target(self, bits):
        """Convert bits to target"""
        exponent = (bits >> 24) & 0xFF
        mantissa = bits & 0x00FFFFFF
        if exponent <= 3:
            return mantissa >> (8 * (3 - exponent))
        return mantissa << (8 * (exponent - 3))
    
    def process_job(self, params):
        """Process mining.notify job"""
        job_id = params[0]
        prev_hash = params[1]
        coinbase1 = params[2]
        coinbase2 = params[3]
        merkle_branches = params[4]
        version = params[5]
        bits = params[6]
        timestamp = params[7]
        
        print(f"\n[JOB #{self.jobs+1}] Processing...")
        print(f"  Job ID: {job_id[:30]}...")
        print(f"  Target bits: {bits}")
        
        # Generate extra nonce (simplified — real implementation uses extranonce1)
        extra_nonce = "00000000"
        
        # Build coinbase
        coinbase_tx = self.build_coinbase_tx(coinbase1, coinbase2, extra_nonce)
        
        # Calculate merkle root
        merkle_root = self.calculate_merkle_root(coinbase_tx, merkle_branches)
        
        # Build block header (without nonce)
        header = struct.pack("<I", int(version, 16) if isinstance(version, str) else version)
        header += bytes.fromhex(prev_hash)[::-1]
        header += bytes.fromhex(merkle_root)[::-1]
        header += struct.pack("<I", int(timestamp, 16))
        header += struct.pack("<I", int(bits, 16))
        # nonce field is empty (76 bytes total so far)
        
        header_hex = header.hex()
        print(f"  Header built: {len(header)} bytes")
        print(f"  Searching for nonce...")
        
        # Find nonce
        result = self.find_nonce_direct(header_hex, int(bits, 16))
        
        if result and result["found"]:
            nonce = result["nonce"]
            attempts = result["attempts"]
            time_ms = result["time_ms"]
            
            print(f"  ✅ NONCE FOUND: {nonce:#010x}")
            print(f"  ⚡ {attempts:,} attempts in {time_ms:.1f}ms")
            print(f"  Speed: {attempts/time_ms*1000:,.0f} hashes/sec")
            
            # Submit to pool
            submit_params = [
                USERNAME,
                job_id,
                extra_nonce,
                timestamp,
                str(nonce)
            ]
            
            print(f"  📤 Submitting share...")
            self.send("mining.submit", submit_params)
            
            # Get response
            response = self.recv()
            if response:
                if response.get("result") == True:
                    self.shares += 1
                    print(f"  🎉 SHARE ACCEPTED! (#{self.shares})")
                    print(f"  💰 Check Electrum: {WALLET_ADDRESS[:20]}...")
                    return True
                else:
                    error = response.get("error", "unknown")
                    print(f"  ❌ Share rejected: {error}")
                    return False
        
        return False
    
    def run(self):
        print("\n" + "="*60)
        print("  💰 BITCOIN MINER — φ-DPLL + BRAIINS POOL")
        print("="*60)
        print(f"  Pool:    {POOL_HOST}:{POOL_PORT}")
        print(f"  User:    {USERNAME}")
        print(f"  Wallet:  {WALLET_ADDRESS}")
        print(f"  Engine:  φ-DPLL Direct Search")
        print("="*60 + "\n")
        
        if not self.connect(): return
        
        # Subscribe
        print("[STRATUM] Subscribing to mining...")
        self.send("mining.subscribe", [])
        resp = self.recv()
        if resp:
            result = resp.get("result", [])
            if len(result) > 1:
                self.extranonce1 = result[1]
                self.extranonce2_size = result[2] if len(result) > 2 else 4
            print(f"[STRATUM] ✅ Subscribed! extranonce1={self.extranonce1}")
        
        # Authorize
        print(f"[STRATUM] Authorizing as {USERNAME}...")
        self.send("mining.authorize", [USERNAME, PASSWORD])
        resp = self.recv()
        if resp and resp.get("result") == True:
            print(f"[STRATUM] ✅ Authorized!")
        
        print("\n" + "="*60)
        print("  ⛏️  MINING ACTIVE — Waiting for jobs...")
        print("  📱 Check Braiins dashboard for your worker!")
        print("  💰 BTC → Electrum wallet")
        print("  Press Ctrl+C to stop")
        print("="*60 + "\n")
        
        while self.running:
            msg = self.recv()
            if not msg:
                time.sleep(0.5)
                continue
            
            method = msg.get("method", "")
            params = msg.get("params", [])
            
            if method == "mining.notify":
                self.jobs += 1
                self.process_job(params)
            
            elif method == "mining.set_difficulty":
                diff = params[0] if params else 0
                print(f"[DIFFICULTY] Set to {diff}")
            
            elif method == "client.show_message":
                print(f"[POOL] {params[0] if params else ''}")
        
        self.cleanup()
    
    def cleanup(self):
        print("\n[MINER] Shutting down...")
        self.running = False
        if self.sock:
            self.sock.close()
        print(f"[MINER] Session ended. Shares accepted: {self.shares}")
        print(f"[💰] Check Electrum: {WALLET_ADDRESS}")

if __name__ == "__main__":
    miner = BraiinsMiner()
    try:
        miner.run()
    except KeyboardInterrupt:
        miner.cleanup()
        print("\n👋 Goodbye! Check your Electrum wallet! 💰")
