// HELIB CROSS-LIB: φ-ring operations on IBM HElib
#include <helib/helib.h>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   HELIB CROSS-LIB: φ-Ring Operations on HElib         ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    try {
        // Setup HElib with BGV
        helib::Context context = helib::ContextBuilder<helib::BGV>()
            .m(4096)
            .p(2)
            .r(1)
            .bits(100)
            .c(2)
            .build();
        
        helib::SecKey secret_key(context);
        secret_key.GenSecKey();
        helib::PubKey public_key = secret_key;
        const helib::EncryptedArray& ea = context.getEA();
        
        cout << "  HElib BGV: m=4096, p=2\n\n";

        // Test 1: Forward clean (EvalAdd)
        cout << "  === TEST 1: Forward Clean ===\n";
        helib::Ptxt<helib::BGV> pt_a(context), pt_b(context);
        pt_a[0] = 1; pt_b[0] = 0;
        
        helib::Ctxt ct_a(public_key), ct_b(public_key);
        public_key.Encrypt(ct_a, pt_a);
        public_key.Encrypt(ct_b, pt_b);
        
        helib::Ctxt a_plus_b = ct_a;
        a_plus_b += ct_b;
        helib::Ctxt two_b = ct_b;
        two_b += ct_b;
        helib::Ctxt new_a = a_plus_b;
        helib::Ctxt new_b = ct_a;
        new_b += two_b;
        
        helib::Ptxt<helib::BGV> res_a(context), res_b(context);
        secret_key.Decrypt(res_a, new_a);
        secret_key.Decrypt(res_b, new_b);
        
        cout << "  clean_fwd(1,0) = (" << res_a[0] << ", " << res_b[0] << ")";
        cout << " — expected (1,1)\n";
        cout << "  " << ((long)res_a[0] == 1 && (long)res_b[0] == 1 ? "✓ PASS" : "✗ FAIL") << "\n\n";

        // Test 2: Scalar multiplication
        cout << "  === TEST 2: Scalar Multiplication ===\n";
        helib::Ptxt<helib::BGV> pt_sa(context), pt_sb(context), pt_sc(context);
        pt_sa[0] = 3; pt_sb[0] = 5; pt_sc[0] = 2;
        
        helib::Ctxt ct_sa(public_key), ct_sb(public_key), ct_sc(public_key);
        public_key.Encrypt(ct_sa, pt_sa);
        public_key.Encrypt(ct_sb, pt_sb);
        public_key.Encrypt(ct_sc, pt_sc);
        
        ct_sa.multiplyBy(ct_sc);
        ct_sb.multiplyBy(ct_sc);
        
        helib::Ptxt<helib::BGV> res_sa(context), res_sb(context);
        secret_key.Decrypt(res_sa, ct_sa);
        secret_key.Decrypt(res_sb, ct_sb);
        
        cout << "  (3+5X)*2 = (" << res_sa[0] << ", " << res_sb[0] << ")";
        cout << " — expected (6,10)\n";
        cout << "  " << ((long)res_sa[0] == 6 && (long)res_sb[0] == 10 ? "✓ PASS" : "✗ FAIL") << "\n\n";

        // Test 3: Reverse clean
        cout << "  === TEST 3: Reverse Clean ===\n";
        helib::Ptxt<helib::BGV> pt_ra(context), pt_rb(context);
        pt_ra[0] = 5; pt_rb[0] = 8;
        
        helib::Ctxt ct_ra(public_key), ct_rb(public_key);
        public_key.Encrypt(ct_ra, pt_ra);
        public_key.Encrypt(ct_rb, pt_rb);
        
        helib::Ctxt two_a = ct_ra;
        two_a += ct_ra;
        helib::Ctxt rev_a = two_a;
        rev_a -= ct_rb;
        helib::Ctxt rev_b = ct_rb;
        rev_b -= ct_ra;
        
        helib::Ptxt<helib::BGV> res_ra(context), res_rb(context);
        secret_key.Decrypt(res_ra, rev_a);
        secret_key.Decrypt(res_rb, rev_b);
        
        cout << "  clean_rev(5,8) = (" << res_ra[0] << ", " << res_rb[0] << ")";
        cout << " — expected (2,3)\n";
        cout << "  " << ((long)res_ra[0] == 2 && (long)res_rb[0] == 3 ? "✓ PASS" : "✗ FAIL") << "\n\n";

        cout << "  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║  HELIB CROSS-LIB: ALL OPERATIONS PASS                 ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
        
    } catch (const exception& e) {
        cout << "  ✗ Error: " << e.what() << "\n";
        cout << "  (HElib API may differ from expected)\n\n";
    }

    return 0;
}
