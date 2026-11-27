#include "ecc.h"

#include <exception>
#include <iostream>

static void printPoint(const Point& point) {
    if (point.infinity) {
        std::cout << "O";
    } else {
        std::cout << "(" << point.x << ", " << point.y << ")";
    }
}

int main() {
    try {
        // Lecture slides 55-58: y^2 = x^3 - 4 (mod 257).
        Curve curve{0, -4, 257, Point(2, 2), 129};
        std::cout << "Order of G: " << pointOrder(curve.G, curve) << '\n';

        KeyPair bob = generateKeys(101, curve);
        Point message(112, 26);  // The lecture already encodes the message as a point.
        Integer k = 41;         // Fixed only so we can reproduce the example.
        Ciphertext ciphertext = encryptWithK(message, bob.publicKey, k, curve);
        Point recovered = decrypt(ciphertext, bob.privateKey, curve);

        std::cout << "Bob's public key: ";
        printPoint(bob.publicKey);
        std::cout << "\nMessage: ";
        printPoint(message);
        std::cout << "\nk * public key: ";
        printPoint(scalarMultiply(k, bob.publicKey, curve));
        std::cout << "\nC1: ";
        printPoint(ciphertext.C1);
        std::cout << "\nC2: ";
        printPoint(ciphertext.C2);
        std::cout << "\nRecovered message: ";
        printPoint(recovered);
        std::cout << '\n';

        return recovered.infinity == message.infinity &&
               recovered.x == message.x && recovered.y == message.y ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "ECC error: " << error.what() << '\n';
        return 1;
    }
}
