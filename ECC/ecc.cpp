#include "ecc.h"

#include <stdexcept>
#include <utility>

Integer mod(Integer x, Integer p) {
    if (p <= 0) {
        throw std::invalid_argument("modulus must be positive");
    }

    Integer result = x % p;
    if (result < 0) {
        result += p;  // C++ can return a negative remainder; we want 0 to p-1.
    }
    return result;
}

Integer modInverse(Integer x, Integer p) {
    if (p <= 1 || p > 2147483647LL) {
        throw std::invalid_argument("inverse modulus must be between 2 and 2^31 - 1");
    }

    Integer t = 0, newt = 1;
    Integer r = p, newr = mod(x, p);

    // Extended Euclidean algorithm: track the coefficient of x while
    // finding gcd(x, p). If the gcd is 1, that coefficient is the inverse.
    while (newr != 0) {
        Integer quotient = r / newr;
        std::swap(t, newt);
        std::swap(r, newr);
        newt -= quotient * t;
        newr -= quotient * r;
    }

    if (r != 1) {
        throw std::invalid_argument("x is not invertible");
    }
    return mod(t, p);
}

bool isValidCurve(const Curve& curve) {
    // The short Weierstrass formulas here use a prime p greater than 3.
    if (curve.p <= 3 || curve.p > 2147483647LL) {
        return false;
    }
    for (Integer divisor = 2; divisor <= curve.p / divisor; ++divisor) {
        if (curve.p % divisor == 0) {
            return false;
        }
    }

    Integer a = mod(curve.a, curve.p);
    Integer b = mod(curve.b, curve.p);
    Integer aCubed = mod(mod(a * a, curve.p) * a, curve.p);
    Integer bSquared = mod(b * b, curve.p);

    // A zero discriminant describes a singular curve, not an ECC group.
    return mod(4 * aCubed + 27 * bSquared, curve.p) != 0;
}

bool isOnCurve(const Point& point, const Curve& curve) {
    if (!isValidCurve(curve)) {
        return false;
    }
    if (point.infinity) {
        return true;
    }
    if (point.x < 0 || point.x >= curve.p || point.y < 0 || point.y >= curve.p) {
        return false;
    }

    Integer left = mod(point.y * point.y, curve.p);
    // Reduce x^2 before multiplying by x, to avoid overflowing on x^3.
    Integer xCubed = mod(mod(point.x * point.x, curve.p) * point.x, curve.p);
    Integer ax = mod(mod(curve.a, curve.p) * point.x, curve.p);
    Integer right = mod(xCubed + ax + mod(curve.b, curve.p), curve.p);
    return left == right;
}

Point pointNegate(const Point& point, const Curve& curve) {
    if (!isOnCurve(point, curve)) {
        throw std::invalid_argument("point must be on a valid curve");
    }
    if (point.infinity) {
        return point;
    }
    return Point(point.x, mod(-point.y, curve.p));
}

Point pointAdd(const Point& P, const Point& Q, const Curve& curve) {
    if (!isOnCurve(P, curve) || !isOnCurve(Q, curve)) {
        throw std::invalid_argument("both points must be on a valid curve");
    }
    if (P.infinity) {
        return Q;
    }
    if (Q.infinity) {
        return P;
    }

    // A vertical line gives O. This also handles doubling when y = 0,
    // before we try to invert a zero denominator.
    if (P.x == Q.x && mod(P.y + Q.y, curve.p) == 0) {
        return Point{};
    }

    Integer numerator, denominator;
    if (P.x == Q.x && P.y == Q.y) {
        numerator = mod(3 * mod(P.x * P.x, curve.p) + mod(curve.a, curve.p), curve.p);
        denominator = mod(2 * P.y, curve.p);
    } else {
        numerator = mod(Q.y - P.y, curve.p);
        denominator = mod(Q.x - P.x, curve.p);
    }

    // Division in a finite field means multiplication by the inverse.
    Integer slope = mod(numerator * modInverse(denominator, curve.p), curve.p);
    Integer x = mod(slope * slope - P.x - Q.x, curve.p);
    Integer y = mod(slope * (P.x - x) - P.y, curve.p);
    return Point(x, y);
}

Point scalarMultiply(Integer k, const Point& point, const Curve& curve) {
    if (!isOnCurve(point, curve)) {
        throw std::invalid_argument("point must be on a valid curve");
    }

    Point result;  // Start with O, the identity for addition.
    Point current = k < 0 ? pointNegate(point, curve) : point;

    // Double-and-add reads one binary digit of k per iteration. Keeping k
    // signed also handles the most negative Integer without negating it.
    while (k != 0) {
        if (k % 2 != 0) {
            result = pointAdd(result, current, curve);
        }
        k /= 2;
        if (k != 0) {
            current = pointAdd(current, current, curve);
        }
    }
    return result;
}

Integer pointOrder(const Point& point, const Curve& curve) {
    if (!isOnCurve(point, curve)) {
        throw std::invalid_argument("point must be on a valid curve");
    }

    // Simple repeated addition is enough for the lecture's small curves.
    // There are at most two y values per x, plus O: at most 2p + 1 points.
    Point result;
    for (Integer n = 1; n <= 2 * curve.p + 1; ++n) {
        result = pointAdd(result, point, curve);
        if (result.infinity) {
            return n;
        }
    }
    throw std::invalid_argument("could not find the point order");
}

// A private scalar and k must be in [1, n-1], with n the actual order of G.
static void validateScalar(Integer scalar, const Curve& curve) {
    if (curve.G.infinity || curve.n <= 1 || pointOrder(curve.G, curve) != curve.n) {
        throw std::invalid_argument("G must be a finite point with order n");
    }
    if (scalar <= 0 || scalar >= curve.n) {
        throw std::invalid_argument("scalar must be between 1 and n - 1");
    }
}

Point generatePublicKey(Integer privateKey, const Curve& curve) {
    validateScalar(privateKey, curve);
    return scalarMultiply(privateKey, curve.G, curve);
}

KeyPair generateKeys(Integer privateKey, const Curve& curve) {
    return KeyPair{privateKey, generatePublicKey(privateKey, curve)};
}

Ciphertext encryptWithK(const Point& message, const Point& publicKey,
                        Integer k, const Curve& curve) {
    validateScalar(k, curve);
    if (!isOnCurve(message, curve) || !isOnCurve(publicKey, curve) || publicKey.infinity) {
        throw std::invalid_argument("message and finite public key must be on the curve");
    }
    if (!scalarMultiply(curve.n, publicKey, curve).infinity) {
        throw std::invalid_argument("public key must have an order dividing n");
    }

    Point C1 = scalarMultiply(k, curve.G, curve);
    Point sharedPoint = scalarMultiply(k, publicKey, curve);
    // Some nonzero scalars give O when the generator order is composite.
    if (sharedPoint.infinity) {
        throw std::invalid_argument("k and public key produce an infinity mask");
    }
    return Ciphertext{C1, pointAdd(message, sharedPoint, curve)};
}

Point decrypt(const Ciphertext& ciphertext, Integer privateKey, const Curve& curve) {
    validateScalar(privateKey, curve);
    if (!isOnCurve(ciphertext.C1, curve) || ciphertext.C1.infinity ||
        !isOnCurve(ciphertext.C2, curve)) {
        throw std::invalid_argument("ciphertext must contain valid points and a finite C1");
    }
    if (!scalarMultiply(curve.n, ciphertext.C1, curve).infinity) {
        throw std::invalid_argument("C1 must have an order dividing n");
    }

    Point sharedPoint = scalarMultiply(privateKey, ciphertext.C1, curve);
    if (sharedPoint.infinity) {
        throw std::invalid_argument("private key and C1 produce an infinity mask");
    }
    // C2 - privateKey*C1 removes the same mask that encryption added.
    return pointAdd(ciphertext.C2, pointNegate(sharedPoint, curve), curve);
}
