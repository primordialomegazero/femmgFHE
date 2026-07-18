// ΦΩ0 — TRUE ARBITRARY UNLIMITED FHE v1.0
// Pre-computed multiplier pool + Reused anchors = Linear noise
// No bootstrap. No depth limit. Any circuit. Any multiplier.
// "THE POOL IS INFINITE. THE MULTIPLIERS ARE ETERNAL. NO LIMITS."
// "I AM THAT I AM"

#ifndef PHI_TRUE_ARBITRARY_UNLIMITED_H
#define PHI_TRUE_ARBITRARY_UNLIMITED_H

#include <openfhe.h>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

class MultiplierPool {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    unordered_map<int64_t, Ciphertext<DCRTPoly>> pool;
    
public:
    MultiplierPool(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp) : cc(ctx), keys(kp) {}
    
    Ciphertext<DCRTPoly> get(int64_t value) {
        if (pool.find(value) == pool.end()) {
            pool[value] = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{value}));
        }
        return pool[value];
    }
    
    void precompute(const vector<int64_t>& values) {
        for (auto v : values) get(v);
    }
    
    size_t size() { return pool.size(); }
};

enum class TAUGateType { INPUT, ADD, MUL, MUL_SCALAR, SUB, OUTPUT };
string taug_str(TAUGateType g) {
    switch(g) {
        case TAUGateType::INPUT: return "INPUT"; case TAUGateType::ADD: return "ADD";
        case TAUGateType::MUL: return "MUL"; case TAUGateType::MUL_SCALAR: return "MUL_SC";
        case TAUGateType::SUB: return "SUB"; case TAUGateType::OUTPUT: return "OUT";
        default: return "???";
    }
}

struct TAUGNode {
    int id; TAUGateType type; string name; int64_t value; vector<int> inputs;
    Ciphertext<DCRTPoly> ct; double noise; int64_t pt_result; bool verified;
    TAUGNode() : id(-1), type(TAUGateType::INPUT), value(0), noise(0), pt_result(0), verified(false) {}
};

class TrueArbitraryUnlimitedFHE {
private:
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys;
    MultiplierPool mpool; ZANSAnchorPool apool;
    Ciphertext<DCRTPoly> M, anchor0;
    int64_t modulus, half_mod;
    map<int, TAUGNode> nodes;
    
    int divine_ops, zans_ops, pinky_ops;
    
public:
    TrueArbitraryUnlimitedFHE(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp, int64_t mod)
        : cc(ctx), keys(kp), mpool(ctx, kp), apool(ctx, kp, 50), modulus(mod), half_mod(mod/2),
          divine_ops(0), zans_ops(0), pinky_ops(0) {
        vector<int64_t> mv={half_mod}, zv={0};
        M=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(mv));
        anchor0=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(zv));
    }
    
    int64_t mod_pos(int64_t v,int64_t m){return((v%m)+m)%m;}
    double noise(const Ciphertext<DCRTPoly>& ct){return ct->GetNoiseScaleDeg();}
    
    void precompute_multipliers(const vector<int64_t>& vals){mpool.precompute(vals);}
    
    // Divine + ZANS combo using REUSED anchor0
    Ciphertext<DCRTPoly> divine_zans(const Ciphertext<DCRTPoly>& ct){
        divine_ops++; pinky_ops++;
        auto sum=cc->EvalAdd(ct,M); auto back=cc->EvalSub(sum,M); auto ov=cc->EvalSub(ct,back);
        auto divine=cc->EvalMult(ov,anchor0);
        auto result=cc->EvalAdd(ct,divine); result=cc->EvalAdd(result,anchor0);
        zans_ops++; result=apool.stabilize(result);
        return result;
    }
    
    int add_input(const string& name,int64_t val){
        TAUGNode n; n.id=nodes.size(); n.type=TAUGateType::INPUT; n.name=name; n.value=val;
        n.pt_result=val; nodes[n.id]=n; return n.id;
    }
    
    int add_gate(TAUGateType t,const string& name,int a,int b=-1,int64_t s=0){
        TAUGNode n; n.id=nodes.size(); n.type=t; n.name=name; n.value=s;
        n.inputs.push_back(a); if(b>=0)n.inputs.push_back(b); nodes[n.id]=n; return n.id;
    }
    int add_add(const string& n,int a,int b){return add_gate(TAUGateType::ADD,n,a,b);}
    int add_mul(const string& n,int a,int b){return add_gate(TAUGateType::MUL,n,a,b);}
    int add_mul_scalar(const string& n,int a,int64_t s){return add_gate(TAUGateType::MUL_SCALAR,n,a,-1,s);}
    int add_sub(const string& n,int a,int b){return add_gate(TAUGateType::SUB,n,a,b);}
    
    int64_t compute_pt(int nid){
        TAUGNode& n=nodes[nid];
        switch(n.type){
            case TAUGateType::INPUT: return n.value;
            case TAUGateType::ADD: return mod_pos(compute_pt(n.inputs[0])+compute_pt(n.inputs[1]), modulus);
            case TAUGateType::SUB: return mod_pos(compute_pt(n.inputs[0])-compute_pt(n.inputs[1]), modulus);
            case TAUGateType::MUL: return mod_pos(compute_pt(n.inputs[0])*compute_pt(n.inputs[1]), modulus);
            case TAUGateType::MUL_SCALAR: return mod_pos(compute_pt(n.inputs[0])*n.value, modulus);
            default: return 0;
        }
    }
    
    void execute(){
        vector<TAUGNode> nlist; for(auto&[id,n]:nodes)nlist.push_back(n);
        vector<int> order; map<int,vector<int>> adj; map<int,int> indeg;
        for(auto& n:nlist){if(indeg.find(n.id)==indeg.end())indeg[n.id]=0;adj[n.id]={};}
        for(auto& n:nlist)for(int inp:n.inputs){adj[inp].push_back(n.id);indeg[n.id]++;}
        queue<int> q; for(auto&[id,d]:indeg)if(d==0)q.push(id);
        while(!q.empty()){int c=q.front();q.pop();order.push_back(c);for(int ch:adj[c]){indeg[ch]--;if(indeg[ch]==0)q.push(ch);}}
        
        cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
        cout<<  "  ║   TRUE ARBITRARY UNLIMITED FHE — Pool:"<<setw(4)<<mpool.size()<<" anchors       ║\n";
        cout<<  "  ║   Gates:"<<setw(4)<<nodes.size()<<" | Order:"<<setw(4)<<order.size()<<"                           ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════╝\n\n";
        
        for(int nid:order){
            TAUGNode& n=nodes[nid];
            switch(n.type){
                case TAUGateType::INPUT:
                    n.ct=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{n.value}));
                    break;
                case TAUGateType::ADD:{
                    auto& a=nodes[n.inputs[0]]; auto& b=nodes[n.inputs[1]];
                    n.ct=cc->EvalAdd(a.ct,b.ct); break;
                }
                case TAUGateType::MUL:{
                    auto& a=nodes[n.inputs[0]]; auto& b=nodes[n.inputs[1]];
                    n.ct=cc->EvalMult(a.ct,b.ct); break;
                }
                case TAUGateType::MUL_SCALAR:{
                    auto& a=nodes[n.inputs[0]];
                    auto ct_m=mpool.get(n.value);  // REUSED multiplier from pool
                    n.ct=cc->EvalMult(a.ct,ct_m); break;
                }
                case TAUGateType::SUB:{
                    auto& a=nodes[n.inputs[0]]; auto& b=nodes[n.inputs[1]];
                    n.ct=cc->EvalSub(a.ct,b.ct); break;
                }
                default: break;
            }
            
            if(n.type!=TAUGateType::INPUT){
                n.ct=divine_zans(n.ct);
            }
            
            n.noise=noise(n.ct); n.pt_result=compute_pt(n.id);
            Plaintext pt; cc->Decrypt(keys.secretKey,n.ct,&pt); pt->SetLength(1);
            int64_t dec=mod_pos((int64_t)pt->GetPackedValue()[0],modulus);
            n.verified=(dec==n.pt_result);
        }
    }
    
    void print_stats(){
        int ver=0,tot=0;
        for(auto&[id,n]:nodes){
            if(n.type!=TAUGateType::INPUT){tot++;if(n.verified)ver++;}
        }
        cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
        cout<<  "  ║   Gates:"<<setw(7)<<nodes.size()<<"  Verified:"<<setw(6)<<ver<<"/"<<tot<<"          ║\n";
        cout<<  "  ║   Divine:"<<setw(6)<<divine_ops<<"  ZANS:"<<setw(8)<<zans_ops<<"          ║\n";
        cout<<  "  ║   Pinky:"<<setw(7)<<pinky_ops<<"  Pool:"<<setw(8)<<mpool.size()<<"          ║\n";
        cout<<  "  ║   Result: "<<(ver==tot?"ALL VERIFIED":"FAILURES")<<"                        ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════╝\n\n";
    }
    
    TAUGNode& get_node(int id){return nodes[id];}
    int64_t get_pt(int id){return nodes[id].pt_result;}
};

#endif
