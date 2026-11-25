#include <stdexcept>
#include <string>
#include <vector>
#include "feistel.hpp"
#include "helpers.hpp"

using namespace helper;


Bytes encrypt_text(const std::string& text, const Bytes& key)
{
    twofish::KeySchedule ks = twofish::expand_key(key);

    Bytes data(text.begin(), text.end());
    const auto pad = static_cast<uint8_t>(16 - (data.size() % 16));
    data.insert(data.end(), pad, pad);

    for (size_t i = 0; i < data.size(); i += 16)
        twofish::encrypt_block(ks, &data[i], &data[i]);   // in and out may alias
    return data;
}

std::string decrypt_text(const Bytes& cipher, const Bytes& key)
{
    if (cipher.empty() || cipher.size() % 16 != 0)
        throw std::invalid_argument("ciphertext length must be a non-zero multiple of 16");

    twofish::KeySchedule ks = twofish::expand_key(key);

    Bytes data = cipher;
    for (size_t i = 0; i < data.size(); i += 16)
        twofish::decrypt_block(ks, &data[i], &data[i]);

    // Validate and strip the padding.
    const uint8_t pad = data.back();
    if (pad == 0 || pad > 16 || pad > data.size())
        throw std::runtime_error("bad padding (wrong key or corrupted data)");
    for (size_t i = data.size() - pad; i < data.size(); ++i)
        if (data[i] != pad)
            throw std::runtime_error("bad padding (wrong key or corrupted data)");

    data.resize(data.size() - pad);
    return std::string(data.begin(), data.end());
}

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