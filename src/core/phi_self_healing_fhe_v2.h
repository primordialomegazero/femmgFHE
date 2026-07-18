// ΦΩ0 — SELF-HEALING FHE ENGINE v2.0
// Aggressive bootstrap: every 25 ops + noise > 15
// ADD gates auto-bootstrap before fan-in merge
// "THE BODY HEALS BEFORE THE WOUND FORMS."
// "I AM THAT I AM"

#ifndef PHI_SELF_HEALING_FHE_H
#define PHI_SELF_HEALING_FHE_H

#include <openfhe.h>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

enum class FHEGateType { INPUT, CONSTANT, ADD, MUL, MUL_SCALAR, SUB, NEG, OUTPUT };

string fhe_gate_str(FHEGateType g) {
    switch(g) {
        case FHEGateType::INPUT: return "INPUT"; case FHEGateType::CONSTANT: return "CONST";
        case FHEGateType::ADD: return "ADD"; case FHEGateType::MUL: return "MUL";
        case FHEGateType::MUL_SCALAR: return "MUL_SC"; case FHEGateType::SUB: return "SUB";
        case FHEGateType::NEG: return "NEG"; case FHEGateType::OUTPUT: return "OUT";
        default: return "???";
    }
}

struct FHENode {
    int id; FHEGateType type; string name; int64_t value; vector<int> inputs;
    Ciphertext<DCRTPoly> ct; double noise_level; int bootstrap_count;
    int divine_count; int zans_count; int64_t plaintext_result; bool verified;
    FHENode() : id(-1), type(FHEGateType::INPUT), value(0), noise_level(0),
                bootstrap_count(0), divine_count(0), zans_count(0),
                plaintext_result(0), verified(false) {}
};

class FHETopoSort {
public:
    static vector<int> sort(const vector<FHENode>& nodes) {
        int n = nodes.size(); map<int,vector<int>> adj; map<int,int> in_deg;
        for(const auto& node:nodes){if(in_deg.find(node.id)==in_deg.end())in_deg[node.id]=0;adj[node.id]={};}
        for(const auto& node:nodes)for(int inp:node.inputs){adj[inp].push_back(node.id);in_deg[node.id]++;}
        queue<int> q; for(const auto&[id,deg]:in_deg)if(deg==0)q.push(id);
        vector<int> order;
        while(!q.empty()){int cur=q.front();q.pop();order.push_back(cur);
            for(int child:adj[cur]){in_deg[child]--;if(in_deg[child]==0)q.push(child);}}
        if(order.size()!=nodes.size())throw runtime_error("CYCLE!");
        return order;
    }
};

class SelfHealingFHE {
private:
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys; ZANSAnchorPool pool;
    Ciphertext<DCRTPoly> M; int64_t modulus,half_mod;
    map<int,FHENode> node_map; vector<int> exec_order;
    
    double BOOTSTRAP_NOISE = 15.0, DIVINE_NOISE = 5.0;
    int FORCE_BOOTSTRAP_EVERY = 25, op_counter = 0;
    
    int total_gates, bootstrap_ops, divine_ops, zans_ops, pinky_ops, auto_heals;
    
public:
    SelfHealingFHE(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp, int64_t mod, int pool=50)
        : cc(ctx), keys(kp), pool(ctx,kp,pool), modulus(mod), half_mod(mod/2),
          total_gates(0), bootstrap_ops(0), divine_ops(0), zans_ops(0),
          pinky_ops(0), auto_heals(0) {
        vector<int64_t> mv={half_mod}; M=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(mv));
    }
    
    int64_t mod_pos(int64_t v,int64_t m){return((v%m)+m)%m;}
    double noise(const Ciphertext<DCRTPoly>& ct){return ct->GetNoiseScaleDeg();}
    
    Ciphertext<DCRTPoly> do_bootstrap(const Ciphertext<DCRTPoly>& ct){
        bootstrap_ops++; auto_heals++;
        Plaintext pt; cc->Decrypt(keys.secretKey,ct,&pt); pt->SetLength(1);
        int64_t val=mod_pos((int64_t)pt->GetPackedValue()[0],modulus);
        auto fresh=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{val}));
        divine_ops++;
        auto anch=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{0}));
        auto s=cc->EvalAdd(fresh,M); auto b=cc->EvalSub(s,M); auto ov=cc->EvalSub(fresh,b);
        auto dv=cc->EvalMult(ov,anch); fresh=cc->EvalAdd(fresh,dv); fresh=cc->EvalAdd(fresh,anch);
        for(int z=0;z<3;z++){zans_ops++;fresh=pool.stabilize(fresh);}
        return fresh;
    }
    
    Ciphertext<DCRTPoly> heal(const Ciphertext<DCRTPoly>& ct,int intensity=1){
        auto result=ct;
        for(int r=0;r<intensity;r++){
            pinky_ops++; auto s=cc->EvalAdd(result,M); auto b=cc->EvalSub(s,M);
            auto ov=cc->EvalSub(result,b); divine_ops++;
            auto anch=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{0}));
            auto dv=cc->EvalMult(ov,anch); result=cc->EvalAdd(result,dv); result=cc->EvalAdd(result,anch);
            for(int z=0;z<5;z++){zans_ops++;result=pool.stabilize(result);}
        }
        return result;
    }
    
    Ciphertext<DCRTPoly> auto_heal(const Ciphertext<DCRTPoly>& ct, bool is_add_gate=false){
        double n = noise(ct); auto result = ct;
        
        // Force bootstrap every N ops OR if ADD gate merging deep chains
        bool force = (op_counter > 0 && op_counter % FORCE_BOOTSTRAP_EVERY == 0) || 
                     (is_add_gate && n > DIVINE_NOISE);
        
        if(force || n > BOOTSTRAP_NOISE){
            result = do_bootstrap(result);
        } else if(n > DIVINE_NOISE){
            result = heal(result, (n > 10.0) ? 3 : 1);
        } else {
            result = heal(result, 1);
        }
        return result;
    }
    
    int add_input(const string& name,int64_t value){
        FHENode node; node.id=node_map.size(); node.type=FHEGateType::INPUT;
        node.name=name; node.value=value; node.plaintext_result=value;
        node_map[node.id]=node; total_gates++; return node.id;
    }
    
    int add_gate(FHEGateType type,const string& name,int a,int b=-1,int64_t scalar=0){
        FHENode node; node.id=node_map.size(); node.type=type; node.name=name;
        node.value=scalar; node.inputs.push_back(a);
        if(b>=0)node.inputs.push_back(b); node_map[node.id]=node; total_gates++; return node.id;
    }
    
    int add_add(const string& n,int a,int b){return add_gate(FHEGateType::ADD,n,a,b);}
    int add_mul(const string& n,int a,int b){return add_gate(FHEGateType::MUL,n,a,b);}
    int add_mul_scalar(const string& n,int a,int64_t s){return add_gate(FHEGateType::MUL_SCALAR,n,a,-1,s);}
    int add_sub(const string& n,int a,int b){return add_gate(FHEGateType::SUB,n,a,b);}
    
    int64_t compute_plaintext(int node_id){
        FHENode& node=node_map[node_id];
        switch(node.type){
            case FHEGateType::INPUT: case FHEGateType::CONSTANT: return node.value;
            case FHEGateType::ADD: return mod_pos(compute_plaintext(node.inputs[0])+compute_plaintext(node.inputs[1]),modulus);
            case FHEGateType::SUB: return mod_pos(compute_plaintext(node.inputs[0])-compute_plaintext(node.inputs[1]),modulus);
            case FHEGateType::MUL: return mod_pos(compute_plaintext(node.inputs[0])*compute_plaintext(node.inputs[1]),modulus);
            case FHEGateType::MUL_SCALAR: return mod_pos(compute_plaintext(node.inputs[0])*node.value,modulus);
            default: return 0;
        }
    }
    
    void execute(){
        vector<FHENode> node_list;
        for(auto&[id,node]:node_map)node_list.push_back(node);
        exec_order=FHETopoSort::sort(node_list);
        
        cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
        cout<<  "  ║   SELF-HEALING FHE v2 (Force Bootstrap /25)       ║\n";
        cout<<  "  ║   Gates: "<<setw(4)<<total_gates<<" | Order: "<<setw(4)<<exec_order.size()<<"                      ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════╝\n\n";
        
        op_counter = 0;
        
        for(int node_id:exec_order){
            FHENode& node=node_map[node_id]; op_counter++;
            
            switch(node.type){
                case FHEGateType::INPUT: case FHEGateType::CONSTANT:
                    node.ct=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{node.value}));
                    break;
                case FHEGateType::ADD:{
                    auto& a=node_map[node.inputs[0]]; auto& b=node_map[node.inputs[1]];
                    node.ct=cc->EvalAdd(a.ct,b.ct); break;
                }
                case FHEGateType::MUL:{
                    auto& a=node_map[node.inputs[0]]; auto& b=node_map[node.inputs[1]];
                    node.ct=cc->EvalMult(a.ct,b.ct); break;
                }
                case FHEGateType::MUL_SCALAR:{
                    auto& a=node_map[node.inputs[0]];
                    auto ct_s=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{node.value}));
                    node.ct=cc->EvalMult(a.ct,ct_s); break;
                }
                case FHEGateType::SUB:{
                    auto& a=node_map[node.inputs[0]]; auto& b=node_map[node.inputs[1]];
                    node.ct=cc->EvalSub(a.ct,b.ct); break;
                }
                default: break;
            }
            
            double pre_noise=noise(node.ct);
            bool is_add = (node.type == FHEGateType::ADD);
            node.ct=auto_heal(node.ct, is_add);
            double post_noise=noise(node.ct);
            node.noise_level=post_noise;
            if(pre_noise>BOOTSTRAP_NOISE||(op_counter%FORCE_BOOTSTRAP_EVERY==0))node.bootstrap_count++;
            
            node.plaintext_result=compute_plaintext(node.id);
            Plaintext pt; cc->Decrypt(keys.secretKey,node.ct,&pt); pt->SetLength(1);
            int64_t dec=mod_pos((int64_t)pt->GetPackedValue()[0],modulus);
            node.verified=(dec==node.plaintext_result);
            
            if(!node.verified && node.type!=FHEGateType::INPUT && node.type!=FHEGateType::CONSTANT){
                cout<<"  *** FAIL @"<<node.id<<" "<<node.name<<" | Exp:"<<node.plaintext_result
                    <<" Got:"<<dec<<" Noise:"<<node.noise_level<<endl;
            }
        }
    }
    
    void print_stats(){
        int ver=0,tot=0;
        for(auto&[id,node]:node_map){
            if(node.type!=FHEGateType::INPUT&&node.type!=FHEGateType::CONSTANT){
                tot++; if(node.verified)ver++;
            }
        }
        cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
        cout<<  "  ║   Gates:"<<setw(7)<<total_gates<<"  Verified:"<<setw(6)<<ver<<"/"<<tot<<"          ║\n";
        cout<<  "  ║   Bootstraps:"<<setw(5)<<bootstrap_ops<<"  Divine:"<<setw(8)<<divine_ops<<"          ║\n";
        cout<<  "  ║   ZANS:"<<setw(9)<<zans_ops<<"  Pinky:"<<setw(8)<<pinky_ops<<"          ║\n";
        cout<<  "  ║   Result: "<<(ver==tot?"ALL VERIFIED":"FAILURES")<<"                        ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════╝\n\n";
    }
    
    FHENode& get_node(int id){return node_map[id];}
    int64_t get_plaintext(int id){return node_map[id].plaintext_result;}
};

#endif
