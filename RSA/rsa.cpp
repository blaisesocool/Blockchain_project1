// RSA following lecture 04 (Public-Key Cryptography and RSA).
//
// mpz_class (from the GMP library) is used for big integers. It works like
// a normal int, but can hold numbers with thousands of bits.

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <gmpxx.h>
using namespace std;

gmp_randclass rng(gmp_randinit_default);   // random number generator

struct PublicKey  { mpz_class e, n; };      // PU = {e, n}
struct PrivateKey { mpz_class d, n; };      // PR = {d, n}

// Square and multiply: computes (base^exp) mod n (lecture slide 26)
mpz_class modPow(mpz_class base, mpz_class exp, mpz_class n) {
    mpz_class result = 1;
    base = base % n;
    while (exp > 0) {
        if (exp % 2 == 1) {              // current bit of exp is 1 -> multiply
            result = (result * base) % n;
        }
        base = (base * base) % n;        // square
        exp = exp / 2;                   // go to the next bit
    }
    return result;
}

// Key generation from two primes p, q and a chosen e (lecture slide 20)
void makeKeys(mpz_class p, mpz_class q, mpz_class e, PublicKey& pub, PrivateKey& priv) {
    mpz_class n = p * q;                     // step 2: n = p * q
    mpz_class phi = (p - 1) * (q - 1);       // step 3: phi(n) = (p-1)(q-1)

    mpz_class d;                             // step 5: d * e = 1 (mod phi)
    mpz_invert(d.get_mpz_t(), e.get_mpz_t(), phi.get_mpz_t());

    pub.e = e;   pub.n = n;
    priv.d = d;  priv.n = n;
}

// Random prime with the given number of bits
mpz_class randomPrime(int bits) {
    mpz_class x = rng.get_z_bits(bits);
    mpz_setbit(x.get_mpz_t(), bits - 1);     // make sure it really has `bits` bits
    mpz_class p;
    mpz_nextprime(p.get_mpz_t(), x.get_mpz_t());
    return p;
}

// Generate a key pair where n has `bits` bits
void generateKeys(int bits, PublicKey& pub, PrivateKey& priv) {
    mpz_class e = 65537;                     // step 4: common choice for e
    mpz_class p, q, phi;
    do {
        p = randomPrime(bits / 2);           // step 1: two large primes
        q = randomPrime(bits / 2);
        phi = (p - 1) * (q - 1);
    } while (p == q || gcd(e, phi) != 1);    // e must be relatively prime to phi(n)

    makeKeys(p, q, e, pub, priv);
}

// C = M^e mod n
mpz_class encrypt(mpz_class M, PublicKey pub) {
    return modPow(M, pub.e, pub.n);
}

// M = C^d mod n
mpz_class decrypt(mpz_class C, PrivateKey priv) {
    return modPow(C, priv.d, priv.n);
}

// ---------- text messages ----------
// RSA encrypts numbers M with 0 <= M < n, so text has to become numbers.
// Each character is one byte (0-255), so a piece of text is read as a
// number in base 256. A 1024-bit n holds 128 bytes, so we cut the text
// into blocks of 100 characters to stay safely below n.

const int BLOCK_SIZE = 100;

vector<mpz_class> encryptText(string text, PublicKey pub) {
    vector<mpz_class> blocks;
    for (size_t i = 0; i < text.size(); i += BLOCK_SIZE) {
        string part = text.substr(i, BLOCK_SIZE);
        mpz_class M = 0;
        for (unsigned char c : part) {
            M = M * 256 + c;
        }
        blocks.push_back(encrypt(M, pub));
    }
    return blocks;
}

string decryptText(vector<mpz_class> blocks, PrivateKey priv) {
    string text;
    for (mpz_class C : blocks) {
        mpz_class M = decrypt(C, priv);
        string part;
        while (M > 0) {
            char c = (char)mpz_class(M % 256).get_ui();
            part = c + part;
            M = M / 256;
        }
        text += part;
    }
    return text;
}

int main() {
    rng.seed(time(NULL));

    // 1) Example from the lecture: p = 17, q = 11, e = 7  ->  n = 187, d = 23
    PublicKey pub;
    PrivateKey priv;
    makeKeys(17, 11, 7, pub, priv);
    cout << "Lecture example: n = " << pub.n << ", e = " << pub.e << ", d = " << priv.d << endl;

    mpz_class M = 88;
    mpz_class C = encrypt(M, pub);
    cout << "M = " << M << " -> C = " << C << " -> M = " << decrypt(C, priv) << endl << endl;

    // 2) 1024-bit keys with text messages
    generateKeys(1024, pub, priv);
    cout << "1024-bit key:" << endl;
    cout << "n = " << pub.n << endl;
    cout << "e = " << pub.e << endl;
    cout << "d = " << priv.d << endl << endl;

    vector<string> messages = {
        "Hello RSA!",
        "Cryptography & Blockchain - INSA Lyon",
        string(250, 'x') + " (a message longer than one block)",
    };

    for (string msg : messages) {
        vector<mpz_class> cipher = encryptText(msg, pub);
        string decrypted = decryptText(cipher, priv);
        cout << "\"" << msg.substr(0, 40) << (msg.size() > 40 ? "..." : "") << "\""
             << " -> " << cipher.size() << " block(s) -> "
             << (decrypted == msg ? "decrypted correctly" : "ERROR") << endl;
    }
    return 0;
}
