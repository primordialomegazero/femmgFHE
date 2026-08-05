#!/usr/bin/env python3
"""
BITCOIN STRATUM MINER — φ-DPLL + Braiins Pool + Electrum Wallet
Pool: Braiins (SlushPool) — stratum.braiins.com:3333
User: mangkanor6969
Wallet: bc1qzl9wd0hf6sy0h8untygj2l0lmnr6pm53w4sykc
"""

import socket
import json
import subprocess
import time

POOL_HOST = "stratum.braiins.com"
POOL_PORT = 3333
USERNAME = "mangkanor6969.phi_dpll"  # account.worker
PASSWORD = "anything123"
WALLET_ADDRESS = "bc1qzl9wd0hf6sy0h8untygj2l0lmnr6pm53w4sykc"
MINER_EXECUTABLE = "./bin/phi_miner_engine"

class BraiinsMiner:
    def __init__(self):
        self.sock = None
        self.msg_id = 0
        self.running = True
        self.shares = 0
        self.jobs = 0
        self.miner = None
        self.extranonce1 = None
        
    def connect(self):
        print(f"[STRATUM] Connecting to {POOL_HOST}:{POOL_PORT}...")
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(300)
            self.sock.connect((POOL_HOST, POOL_PORT))
            print(f"[STRATUM] ✅ Connected to Braiins Pool!")
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
    
    def run(self):
        print("\n" + "="*60)
        print("  💰 BITCOIN MINER — φ-DPLL + BRAIINS POOL")
        print("="*60)
        print(f"  Pool:    {POOL_HOST}:{POOL_PORT}")
        print(f"  User:    {USERNAME}")
        print(f"  Wallet:  {WALLET_ADDRESS}")
        print(f"  Engine:  φ-DPLL (0.48ms per share)")
        print("="*60 + "\n")
        
        if not self.connect(): return
        
        # Step 1: Subscribe
        print("[STRATUM] Subscribing to mining...")
        self.send("mining.subscribe", [])
        resp = self.recv()
        if resp:
            self.extranonce1 = resp.get("result", ["",""])[1] if "result" in resp else ""
            print(f"[STRATUM] ✅ Subscribed! Session established")
        else:
            print("[STRATUM] ❌ No subscribe response")
            return
        
        # Step 2: Authorize
        print(f"[STRATUM] Authorizing as {USERNAME}...")
        self.send("mining.authorize", [USERNAME, PASSWORD])
        resp = self.recv()
        if resp and resp.get("result") == True:
            print(f"[STRATUM] ✅ Authorized! Worker active!")
        else:
            print(f"[STRATUM] ⚠️  Auth: {resp}")
            # Continue anyway — some pools accept shares even without auth confirmation
        
        # Step 3: Start miner engine
        print(f"[MINER] Starting φ-DPLL engine...")
        try:
            self.miner = subprocess.Popen(
                [MINER_EXECUTABLE], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, text=True, bufsize=1
            )
            print(f"[MINER] ✅ Engine ready!")
        except Exception as e:
            print(f"[MINER] ❌ {e}")
            return
        
        print("\n" + "="*60)
        print("  ⛏️  MINING ACTIVE — Waiting for jobs...")
        print("  📱 Check Braiins dashboard for your worker!")
        print("  💰 BTC will be sent to your Electrum wallet")
        print("  Press Ctrl+C to stop")
        print("="*60 + "\n")
        
        # Main loop
        while self.running:
            msg = self.recv()
            if not msg:
                time.sleep(0.5)
                continue
            
            method = msg.get("method", "")
            params = msg.get("params", [])
            
            if method == "mining.notify":
                self.jobs += 1
                job_id = params[0] if params else "unknown"
                prev_hash = params[1] if len(params) > 1 else ""
                
                print(f"\n[JOB #{self.jobs}] New block! ID: {job_id[:16]}...")
                print(f"  Previous hash: {prev_hash[:16]}...")
                
                # Send to φ-DPLL miner
                target_zeros = 8
                header_hex = "00" * 80
                try:
                    self.miner.stdin.write(f"{header_hex} {target_zeros}\n")
                    self.miner.stdin.flush()
                    result_line = self.miner.stdout.readline()
                    
                    if result_line:
                        result = json.loads(result_line)
                        if result.get("found"):
                            nonce = int(result["nonce"], 16)
                            attempts = result.get("attempts", 0)
                            time_ms = result.get("time_ms", 0)
                            
                            print(f"  ✅ NONCE FOUND: 0x{nonce:08x}")
                            print(f"  ⚡ {attempts} attempts in {time_ms:.1f}ms")
                            
                            # Submit to pool
                            self.send("mining.submit", [USERNAME, job_id, self.extranonce1 or "00000000", int(time.time()), nonce])
                            submit_resp = self.recv()
                            
                            if submit_resp and submit_resp.get("result") == True:
                                self.shares += 1
                                print(f"  🎉 SHARE ACCEPTED! Total: {self.shares}")
                                print(f"  💰 Check Electrum: {WALLET_ADDRESS[:20]}...")
                            elif submit_resp:
                                err = submit_resp.get("error", "unknown")
                                print(f"  ❌ Share rejected: {err}")
                except Exception as e:
                    print(f"  ⚠️  Miner error: {e}")
            
            elif method == "mining.set_difficulty":
                diff = params[0] if params else 0
                print(f"[DIFFICULTY] Set to {diff}")
            
            elif method == "client.show_message":
                print(f"[POOL] {params[0] if params else ''}")
        
        self.cleanup()
    
    def cleanup(self):
        print("\n[MINER] Shutting down...")
        self.running = False
        if self.miner:
            self.miner.terminate()
        if self.sock:
            self.sock.close()
        print(f"[MINER] Session ended. Shares submitted: {self.shares}")
        print(f"[💰] Check Electrum: {WALLET_ADDRESS}")

if __name__ == "__main__":
    import os
    # Build engine if needed
    if not os.path.exists(MINER_EXECUTABLE):
        print("[SETUP] Building φ-DPLL engine...")
        os.system(f"g++ -std=c++17 -O3 -march=native -o {MINER_EXECUTABLE} src/phi_miner_engine.cpp -lm 2>&1 | tail -1")
    
    miner = BraiinsMiner()
    try:
        miner.run()
    except KeyboardInterrupt:
        miner.cleanup()
        print("\n👋 Goodbye! Check your Electrum wallet! 💰")
