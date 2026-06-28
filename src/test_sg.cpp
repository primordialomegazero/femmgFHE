#include <iostream>
#include <string>

std::string sg(const std::string& b, const std::string& k) {
    std::string s = "\""+k+"\":"; 
    size_t p = b.find(s); 
    if(p == std::string::npos) return "NOT_FOUND";
    p += s.size(); 
    std::cout << "  After key skip, p=" << p << " char='" << b[p] << "'" << std::endl;
    while(p < b.size() && (b[p]==' ' || b[p]=='"')) { 
        std::cout << "  Skipping char='" << b[p] << "'" << std::endl;
        p++; 
    }
    if(p >= b.size()) return "END_OF_STRING";
    
    std::cout << "  After skip, p=" << p << " char='" << b[p] << "'" << std::endl;
    
    if(b[p] == '"') { 
        std::cout << "  STRING MODE" << std::endl;
        p++; 
        std::string r; 
        while(p < b.size() && b[p] != '"') { 
            if(b[p] == '\\' && p+1 < b.size()) p++; 
            r += b[p]; 
            p++; 
        } 
        return r; 
    }
    
    // Numeric/boolean mode
    size_t e = p; 
    while(e < b.size() && b[e]!=',' && b[e]!='}' && b[e]!=' ') e++; 
    return b.substr(p, e-p);
}

int main() {
    std::string body = "{\"action\":\"register\",\"client_id\":\"debugtest\"}";
    std::cout << "Body: " << body << std::endl;
    std::cout << "action: '" << sg(body, "action") << "'" << std::endl;
    return 0;
}
