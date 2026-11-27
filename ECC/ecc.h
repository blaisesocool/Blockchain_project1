#ifndef ECC_H
#define ECC_H

// Ordinary integers are enough for the lecture's small curves. We limit
// curve moduli to 2^31 - 1 so intermediate products fit in this type.
using Integer = long long;

struct Point {
    Integer x = 0;
    Integer y = 0;
    bool infinity = true;

    Point() = default;  // Point{} represents the point at infinity, O.
    Point(Integer x, Integer y) : x(x), y(y), infinity(false) {}
};

struct Curve {
    Integer a;
    Integer b;
    Integer p;

    Point G;
    Integer n;  // Order of G: the first positive n for which nG = O.
};

struct KeyPair {
    Integer privateKey;
    Point publicKey;
};

struct Ciphertext {
    Point C1;
    Point C2;
};

Integer mod(Integer x, Integer p);
Integer modInverse(Integer x, Integer p);

bool isValidCurve(const Curve& curve);
bool isOnCurve(const Point& point, const Curve& curve);
Point pointNegate(const Point& point, const Curve& curve);
Point pointAdd(const Point& P, const Point& Q, const Curve& curve);
Point scalarMultiply(Integer k, const Point& point, const Curve& curve);
Integer pointOrder(const Point& point, const Curve& curve);

// Supply scalars explicitly to reproduce the lecture examples. Callers
// choose the private key and a fresh k for each encryption.
Point generatePublicKey(Integer privateKey, const Curve& curve);
KeyPair generateKeys(Integer privateKey, const Curve& curve);
Ciphertext encryptWithK(const Point& message, const Point& publicKey,
                        Integer k, const Curve& curve);
Point decrypt(const Ciphertext& ciphertext, Integer privateKey, const Curve& curve);

#endif
