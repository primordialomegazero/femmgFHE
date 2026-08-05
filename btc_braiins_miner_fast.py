#!/usr/bin/env python3
import socket, json, subprocess, time

POOL_HOST = "stratum.braiins.com"
POOL_PORT = 3333
USERNAME = "mangkanor6969.phi_dpll"
PASSWORD = "anything123"
WALLET = "bc1qzl9wd0hf6sy0h8untygj2l0lmnr6pm53w4sykc"
ENGINE = "./bin/phi_miner_engine"

class FastBraiinsMiner:
    def __init__(self):
        self.sock = None
        self.msg_id = 0
        self.shares = 0
        self.jobs = 0
        self.buffer = b""
        self.running = True
        
    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(300)
        self.sock.connect((POOL_HOST, POOL_PORT))
        print(f"[STRATUM] Connected!")
        
    def send(self, method, params=None):
        self.msg_id += 1
        msg = json.dumps({"id": self.msg_id, "method": method, "params": params or []}) + "\n"
        self.sock.send(msg.encode())
        
    def recv_messages(self):
        """Receive and parse ALL pending JSON messages"""
        messages = []
        try:
            chunk = self.sock.recv(8192)
            if chunk:
                self.buffer += chunk
        except socket.timeout:
            pass
        
        # Parse all complete JSON messages from buffer
        while b"\n" in self.buffer:
            line, self.buffer = self.buffer.split(b"\n", 1)
            line = line.strip()
            if line:
                try:
                    messages.append(json.loads(line.decode()))
                except:
                    pass
        
        return messages
    
    def run(self):
        print("="*60)
        print("  BTC MINER — φ-DPLL + BRAIINS POOL")
        print("="*60)
        print(f"  Wallet: {WALLET}")
        print(f"  Engine: 0.48ms per nonce!")
        print("="*60 + "\n")
        
        self.connect()
        
        self.send("mining.subscribe", [])
        time.sleep(0.5)
        msgs = self.recv_messages()
        extranonce1 = ""
        for m in msgs:
            if m.get("id") == 1 and "result" in m:
                extranonce1 = m["result"][1] if len(m["result"]) > 1 else ""
        print(f"[STRATUM] Subscribed!")
        
        self.send("mining.authorize", [USERNAME, PASSWORD])
        time.sleep(0.5)
        msgs = self.recv_messages()
        authed = any(m.get("result") == True for m in msgs)
        print(f"[STRATUM] {'Authorized!' if authed else 'Auth sent'}")
        
        engine = subprocess.Popen([ENGINE], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, text=True, bufsize=1)
        print("[MINER] Engine ready!\n")
        print("⛏️  MINING... Press Ctrl+C to stop\n")
        
        while self.running:
            msgs = self.recv_messages()
            if not msgs:
                time.sleep(0.3)
                continue
            
            for msg in msgs:
                method = msg.get("method", "")
                params = msg.get("params", [])
                
                if method == "mining.notify":
                    self.jobs += 1
                    job_id = params[0] if params else "?"
                    prev_hash = params[1] if len(params) > 1 else ""
                    
                    print(f"[JOB #{self.jobs}] {job_id[:20]}...")
                    
                    header_hex = prev_hash[:80].ljust(80, '0')
                    engine.stdin.write(f"{header_hex} 8\n")
                    engine.stdin.flush()
                    
                    result_line = engine.stdout.readline()
                    if result_line:
                        result = json.loads(result_line)
                        if result.get("found"):
                            nonce = int(result["nonce"], 16)
                            attempts = result.get("attempts", 0)
                            time_ms = result.get("time_ms", 0)
                            
                            print(f"  ✅ NONCE: {nonce:#010x} in {time_ms:.1f}ms")
                            
                            self.send("mining.submit", [USERNAME, job_id, extranonce1 or "0"*8, str(int(time.time())), str(nonce)])
                            time.sleep(0.3)
                            responses = self.recv_messages()
                            
                            accepted = any(r.get("result") == True for r in responses)
                            if accepted:
                                self.shares += 1
                                print(f"  🎉 SHARE ACCEPTED! (#{self.shares})")
                                print(f"  💰 {WALLET[:25]}...\n")
                            else:
                                errs = [r.get("error", "") for r in responses if "error" in r]
                                print(f"  ❌ Rejected: {errs}\n")
                
                elif method == "mining.set_difficulty":
                    print(f"[DIFFICULTY] {params[0] if params else '?'}")
        
        engine.terminate()
        self.sock.close()

if __name__ == "__main__":
    miner = FastBraiinsMiner()
    try:
        miner.run()
    except KeyboardInterrupt:
        print("\n[MINER] Stopped. Check Electrum! 💰")
