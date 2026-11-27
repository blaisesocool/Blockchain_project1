#include "ecc.h"

#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

static int checks = 0;

static void check(bool condition, const std::string& description) {
    ++checks;
    // Unlike assert(), this still checks results in a Release build.
    if (!condition) {
        throw std::runtime_error(description);
    }
}

static bool samePoint(const Point& P, const Point& Q) {
    if (P.infinity || Q.infinity) {
        return P.infinity == Q.infinity;
    }
    return P.x == Q.x && P.y == Q.y;
}

template <typename Function>
static void expectInvalid(Function operation, const std::string& description) {
    try {
        operation();
    } catch (const std::invalid_argument&) {
        check(true, description);
        return;
    }
    check(false, description);
}

static void testHelpers() {
    check(mod(25, 23) == 2, "positive modulo");
    check(mod(-7, 23) == 16, "negative modulo");
    check(mod(-23, 23) == 0, "negative multiple modulo");
    check(mod(23, 23) == 0, "zero remainder");
    check(modInverse(4, 23) == 6, "lecture inverse(4, 23)");
    check(modInverse(-4, 23) == 17, "negative inverse input");
    check(modInverse(50, 23) == 6, "inverse input larger than modulus");
    check(modInverse(5, 12) == 5, "invertible input with composite modulus");
    expectInvalid([] { mod(1, 0); }, "zero modulus rejected");
    expectInvalid([] { mod(1, -23); }, "negative modulus rejected");
    expectInvalid([] { modInverse(0, 23); }, "zero has no inverse");
    expectInvalid([] { modInverse(23, 23); }, "multiple of modulus has no inverse");
    expectInvalid([] { modInverse(6, 15); }, "non-coprime input has no inverse");
    expectInvalid([] { modInverse(1, 1); }, "inverse modulus one rejected");
    expectInvalid([] { modInverse(1, 0); }, "inverse modulus zero rejected");
    expectInvalid([] { modInverse(1, -23); }, "inverse negative modulus rejected");

    for (Integer p : {5, 7, 23, 257}) {
        for (Integer x = 1; x < p; ++x) {
            check(mod(x * modInverse(x, p), p) == 1, "inverse multiplication identity");
        }
    }
}

static void testLectureArithmetic() {
    Curve curve{1, 1, 23, Point{}, 0};  // G and n are not needed for point arithmetic.
    Point P(3, 10), Q(9, 7), infinity;

    check(isValidCurve(curve), "lecture curve valid");
    check(isOnCurve(Q, curve), "lecture point valid");
    check(!isOnCurve(Point(9, 8), curve), "off-curve point rejected");
    check(!isOnCurve(Point(-14, 7), curve), "negative coordinate rejected");
    check(!isOnCurve(Point(32, 7), curve), "coordinate outside field rejected");
    check(!isOnCurve(Point(9, 30), curve), "y coordinate outside field rejected");
    check(isOnCurve(infinity, curve), "infinity belongs to valid curve");
    check(samePoint(pointNegate(Point(13, 7), curve), Point(13, 16)), "lecture negation");
    check(samePoint(pointAdd(P, Q, curve), Point(17, 20)), "lecture addition");
    check(samePoint(pointAdd(P, P, curve), Point(7, 12)), "lecture doubling");
    check(samePoint(pointAdd(P, infinity, curve), P), "P + O = P");
    check(samePoint(pointAdd(infinity, P, curve), P), "O + P = P");
    check(pointAdd(infinity, infinity, curve).infinity, "O + O = O");
    check(pointNegate(infinity, curve).infinity, "-O = O");
    check(pointAdd(P, pointNegate(P, curve), curve).infinity, "P + (-P) = O");
    check(isOnCurve(Point(4, 0), curve), "zero-y test point is actually on curve");
    check(pointAdd(Point(4, 0), Point(4, 0), curve).infinity, "doubling y=0 gives O");
    check(scalarMultiply(0, P, curve).infinity, "0P = O");
    check(samePoint(scalarMultiply(1, P, curve), P), "1P = P");
    check(samePoint(scalarMultiply(-1, P, curve), pointNegate(P, curve)), "(-1)P = -P");
    check(scalarMultiply(10, infinity, curve).infinity, "kO = O");

    Curve scalarCurve{9, 17, 23, Point{}, 0};
    Point base(16, 5);
    std::vector<Point> expected = {
        Point(16, 5), Point(20, 20), Point(14, 14), Point(19, 20), Point(13, 10),
        Point(7, 3), Point(8, 7), Point(12, 17), Point(4, 5)
    };
    for (Integer k = 1; k <= 9; ++k) {
        check(samePoint(scalarMultiply(k, base, scalarCurve), expected[k - 1]),
              "lecture scalar multiplication table");
    }

    for (Curve invalid : {Curve{0, 0, 23, Point{}, 0}, Curve{1, 1, 15, Point{}, 0},
                          Curve{1, 1, 0, Point{}, 0}, Curve{1, 1, 3, Point{}, 0},
                          Curve{1, 1, 2147483648LL, Point{}, 0}}) {
        check(!isValidCurve(invalid), "singular or unsupported curve rejected");
        check(!isOnCurve(infinity, invalid), "infinity does not bypass curve validation");
    }
    expectInvalid([&] { pointAdd(P, Point(9, 8), curve); }, "addition rejects invalid point");
    expectInvalid([&] { pointNegate(Point(9, 8), curve); }, "negation rejects invalid point");
    expectInvalid([&] { scalarMultiply(0, Point(9, 8), curve); }, "zero scalar validates point");

    // Signed scalar extremes must not overflow when handling a negative k.
    Integer order = pointOrder(P, curve);
    Integer smallest = std::numeric_limits<Integer>::min();
    Integer largest = std::numeric_limits<Integer>::max();
    check(samePoint(scalarMultiply(smallest, P, curve), scalarMultiply(mod(smallest, order), P, curve)),
          "minimum signed scalar");
    check(samePoint(scalarMultiply(largest, P, curve), scalarMultiply(mod(largest, order), P, curve)),
          "maximum signed scalar");
    check(pointOrder(infinity, curve) == 1, "infinity has order one");

    // Check the largest supported field using a separate big-integer reference.
    Curve largestCurve{1, 1, 2147483647LL, Point{}, 0};
    check(isOnCurve(Point(0, 1), largestCurve), "point in largest supported field");
    check(samePoint(pointAdd(Point(0, 1), Point(0, 1), largestCurve),
                    Point(536870912, 1879048190)), "doubling near the integer limit");
    Curve reducedCoefficients = curve;
    reducedCoefficients.a = -22;
    reducedCoefficients.b = 24;
    check(samePoint(pointAdd(P, Q, reducedCoefficients), Point(17, 20)),
          "equivalent coefficients are reduced modulo p");
}

static void testGroupLaws(const Curve& curve) {
    std::vector<Point> points{Point{}};
    for (Integer x = 0; x < curve.p; ++x) {
        for (Integer y = 0; y < curve.p; ++y) {
            if (isOnCurve(Point(x, y), curve)) {
                points.push_back(Point(x, y));
            }
        }
    }

    // Exhaustively check both small E23 groups, including their infinity point.
    for (const Point& P : points) {
        Point repeated;
        for (Integer k = 0; k <= 2 * curve.p; ++k) {
            check(samePoint(scalarMultiply(k, P, curve), repeated), "double-and-add vs repeated addition");
            check(samePoint(scalarMultiply(-k, P, curve), pointNegate(repeated, curve)), "negative scalar");
            repeated = pointAdd(repeated, P, curve);
        }
        for (const Point& Q : points) {
            Point sum = pointAdd(P, Q, curve);
            check(isOnCurve(sum, curve), "group closure");
            check(samePoint(sum, pointAdd(Q, P, curve)), "commutativity");
            for (const Point& R : points) {
                check(samePoint(pointAdd(sum, R, curve), pointAdd(P, pointAdd(Q, R, curve), curve)),
                      "associativity");
            }
        }
    }
}

static void testElGamal() {
    Curve curve{0, -4, 257, Point(2, 2), 129};
    check(pointOrder(curve.G, curve) == 129, "verified lecture generator order");
    for (Integer k = 1; k < curve.n; ++k) {
        check(!scalarMultiply(k, curve.G, curve).infinity, "no earlier multiple of G is infinity");
    }
    check(scalarMultiply(129, curve.G, curve).infinity, "129G = O");

    KeyPair bob = generateKeys(101, curve);
    check(bob.privateKey == 101, "private key preserved");
    check(samePoint(bob.publicKey, Point(197, 167)), "lecture public key");
    Point message(112, 26);
    check(samePoint(scalarMultiply(41, bob.publicKey, curve), Point(68, 84)), "lecture shared mask");
    Ciphertext ciphertext = encryptWithK(message, bob.publicKey, 41, curve);
    check(samePoint(ciphertext.C1, Point(136, 128)), "lecture C1");
    check(samePoint(ciphertext.C2, Point(246, 174)), "lecture C2");
    check(samePoint(scalarMultiply(101, ciphertext.C1, curve), Point(68, 84)), "Bob recovers same mask");
    check(samePoint(decrypt(ciphertext, bob.privateKey, curve), message), "lecture recovered message");

    // O is a valid message and C2 may legitimately equal O.
    Ciphertext zeroMessage = encryptWithK(Point{}, bob.publicKey, 41, curve);
    check(decrypt(zeroMessage, 101, curve).infinity, "infinity message round trip");
    Point negativeMask = pointNegate(Point(68, 84), curve);
    Ciphertext zeroC2 = encryptWithK(negativeMask, bob.publicKey, 41, curve);
    check(zeroC2.C2.infinity, "C2 may be infinity");
    check(samePoint(decrypt(zeroC2, 101, curve), negativeMask), "infinity C2 round trip");

    // Every allowed k works with Bob's full-order public key.
    for (Integer k = 1; k < curve.n; ++k) {
        Ciphertext encrypted = encryptWithK(message, bob.publicKey, k, curve);
        check(samePoint(decrypt(encrypted, bob.privateKey, curve), message), "round trip for each k");
    }
    for (Integer invalid : {-1, 0, 129}) {
        expectInvalid([&] { generateKeys(invalid, curve); }, "invalid private scalar rejected");
        expectInvalid([&] { encryptWithK(message, bob.publicKey, invalid, curve); }, "invalid k rejected");
        expectInvalid([&] { decrypt(ciphertext, invalid, curve); }, "invalid decryption scalar rejected");
    }
    Curve wrongOrder = curve;
    wrongOrder.n = 258;
    expectInvalid([&] { generateKeys(101, wrongOrder); }, "a multiple of the order is not the order");
    expectInvalid([&] { encryptWithK(Point(0, 0), bob.publicKey, 41, curve); }, "invalid message rejected");
    expectInvalid([&] { encryptWithK(message, Point{}, 41, curve); }, "infinity public key rejected");
    expectInvalid([&] { encryptWithK(message, Point(0, 0), 41, curve); }, "invalid public key rejected");
    expectInvalid([&] { decrypt(Ciphertext{Point{}, ciphertext.C2}, 101, curve); }, "infinity C1 rejected");
    expectInvalid([&] { decrypt(Ciphertext{ciphertext.C1, Point(0, 0)}, 101, curve); }, "invalid C2 rejected");

    KeyPair smallOrderKey = generateKeys(3, curve);
    expectInvalid([&] { encryptWithK(message, smallOrderKey.publicKey, 43, curve); },
                  "composite-order infinity mask rejected");
}

int main() {
    try {
        testHelpers();
        testLectureArithmetic();
        testGroupLaws(Curve{1, 1, 23, Point{}, 0});
        testGroupLaws(Curve{9, 17, 23, Point{}, 0});
        testElGamal();
        std::cout << "All " << checks << " ECC checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ECC test failed after " << checks << " checks: " << error.what() << '\n';
        return 1;
    }
}
