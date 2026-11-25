#include <cstdio>
#include <exception>
#include <string>
#include "text.hpp"

using namespace helper;


static void print_hex(const char* label, const Bytes& b)
{
    std::printf("%s", label);
    for (size_t i = 0; i < b.size(); ++i) {
        std::printf("%02X", b[i]);
        if (i % 16 == 15 && i + 1 < b.size()) std::printf("\n%*s", static_cast<int>(std::string(label).size()), "");
    }
    std::printf("\n");
}

int main()
{
    // 16 ASCII characters = a 128-bit key. For real use, generate random bytes
    // (or derive the key from a passphrase with a proper KDF).
    const Bytes key = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

    const std::string message = "Hello, Twofish! This text is longer than one block.";

    Bytes cipher = encrypt_text(message, key);
    print_hex("ciphertext: ", cipher);

    std::string decrypted = decrypt_text(cipher, key);
    std::printf("decrypted:  %s\n", decrypted.c_str());
    std::printf("round trip: %s\n", decrypted == message ? "OK" : "FAIL");

    // Wrong key must not silently give the original text back.
    const Bytes wrong = {'X','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    try {
        std::string bad = decrypt_text(cipher, wrong);
        std::printf("wrong key:  garbage (%zu bytes)\n", bad.size());
    } catch (const std::exception& e) {
        std::printf("wrong key:  %s\n", e.what());
    }
    return decrypted == message ? 0 : 1;
}
