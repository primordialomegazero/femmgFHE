#include <iostream>
#include <string>

// Test sg function
std::string sg(const std::string& b, const std::string& k) {
    std::string s = "\""+k+"\":"; size_t p=b.find(s); if(p==std::string::npos) return "";
    p+=s.size(); while(p<b.size()&&(b[p]==' '||b[p]=='"')) p++; if(p>=b.size()) return "";
    if(b[p]=='"') { p++; std::string r; while(p<b.size()&&b[p]!='"') { if(b[p]=='\\'&&p+1<b.size()) p++; r+=b[p]; p++; } return r; }
    size_t e=p; while(e<b.size()&&b[e]!=','&&b[e]!='}'&&b[e]!=' '&&b[e]!='\n') e++; return b.substr(p,e-p);
}

bool is_attack(const std::string& body) {
    if (body.size() > 4096) return true;
    for (char c : body) if (c < 32 && c != '\n' && c != '\r' && c != '\t') return true;
    return false;
}

int main() {
    std::string body = "{\"action\":\"register\",\"client_id\":\"debugtest\"}";
    std::cout << "Body: " << body << std::endl;
    std::cout << "is_attack: " << is_attack(body) << std::endl;
    std::cout << "action: '" << sg(body, "action") << "'" << std::endl;
    std::cout << "client_id: '" << sg(body, "client_id") << "'" << std::endl;
    
    std::string body2 = "{\"action\":\"health\",\"client_id\":\"test\"}";
    std::cout << "\nBody2: " << body2 << std::endl;
    std::cout << "is_attack: " << is_attack(body2) << std::endl;
    std::cout << "action: '" << sg(body2, "action") << "'" << std::endl;
    
    return 0;
}
