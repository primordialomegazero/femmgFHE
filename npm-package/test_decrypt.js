// Quick math test to find correct formula
const phi = 1.565670;
const lambda = 0.569744;
const PHI_PUBLIC = 1.618034;

// Encrypt 6 and 7
function encrypt(m, nonce) {
  return m * phi + lambda + nonce;
}

// Test with actual nonces from the real encryption
// Let's work backwards from the server result

const m1 = 6, m2 = 7;
const expected = m1 * m2; // 42

// Try different nonce values
for (let n1 = 0; n1 < 0.05; n1 += 0.001) {
  for (let n2 = 0; n2 < 0.05; n2 += 0.001) {
    const e1 = encrypt(m1, n1);
    const e2 = encrypt(m2, n2);
    const serverResult = (e1 * e2) / PHI_PUBLIC;
    
    // Try to find what formula gives 42
    // The correct approach: multiply back by PHI_PUBLIC, then divide by phi^2
    const fullProduct = serverResult * PHI_PUBLIC;
    const decrypted = Math.round((fullProduct) / (phi * phi));
    
    if (decrypted === expected) {
      // Found it! But is it always correct?
      // Actually we need to subtract the lambda*phi terms
      const dec2 = Math.round((fullProduct - lambda * phi * (m1 + m2)) / (phi * phi));
      // Hmm, this needs m1+m2 which we don't know
    }
  }
}

// SIMPLE FIX: Just divide by phi, not PHI_PUBLIC
// The server divides by PHI_PUBLIC, we multiply back by PHI_PUBLIC/phi
const e1_test = encrypt(6, 0.001);
const e2_test = encrypt(7, 0.002);
const server_test = (e1_test * e2_test) / PHI_PUBLIC;
const dec_simple = Math.round((server_test * PHI_PUBLIC) / (phi * phi));
console.log('Simple formula:', dec_simple);

// Better: treat the multiplication as (e1*e2)/PHI_PUBLIC then decrypt normally
// decrypt(m) = round((m - lambda)/phi)
// But e_result isn't in the form m*phi + lambda
// e_result = (e1*e2)/PHI_PUBLIC = ((m1*phi+lambda+n1)*(m2*phi+lambda+n2))/PHI_PUBLIC
// For small nonces: ~= (m1*m2*phi^2)/PHI_PUBLIC
// So: m1*m2 = round(e_result * PHI_PUBLIC / phi^2)

const dec_final = Math.round(server_test * PHI_PUBLIC / (phi * phi));
console.log('Final formula:', dec_final);
