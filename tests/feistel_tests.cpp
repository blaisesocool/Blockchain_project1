#include "feistel.hpp"
#include "text.hpp"
#include <cstdio>
#include <exception>
#include <random>
#include <string>
#include <vector>

int main()
{
    try
    {
        std::mt19937 random(42); // Fixed seed so we can repeat the same tests.

        std::string binary;
        for (int i = 0; i < 256; ++i) binary += static_cast<char>(i);

        std::vector<std::string> messages = {
            "", "A", "abc", "Hello, Twofish!",
            std::string(15, 'A'), std::string(16, 'A'), std::string(17, 'A'),
            "ABCDABCDABCDABCDABCDABCDABCDABCD",
            std::string("A\0B\0C", 5), binary, std::string(1000, 'Z')
        };
        messages.push_back(std::string(257, '\0'));
        for (char& byte : messages.back()) byte = static_cast<char>(random() % 256);

        int passed = 0;
        for (size_t key_size : {16u, 24u, 32u})
        {
            for (int key_number = 0; key_number < 5; ++key_number)
            {
                helper::Bytes key(key_size);
                for (auto& byte : key) byte = static_cast<uint8_t>(random() % 256);

                for (const auto& message : messages)
                {
                    const auto cipher = encrypt_text(message, key);
                    const auto recovered = decrypt_text(cipher, key);
                    if (recovered != message)
                    {
                        std::printf("FAIL: %zu-bit key #%d, %zu-byte message\n",
                                    key_size * 8, key_number + 1, message.size());
                        return 1;
                    }
                    ++passed;
                }
            }
            std::printf("PASS: %zu-bit keys, 5 random keys x %zu messages\n",
                        key_size * 8, messages.size());
        }

        // Published Twofish results for an all-zero block and all-zero keys.
        // Source: Twofish submission, Appendix A.2, I=1 for each key size.
        // https://www.schneier.com/wp-content/uploads/2016/02/paper-twofish-paper.pdf
        const helper::Bytes expected[] = {
            {0x9F, 0x58, 0x9F, 0x5C, 0xF6, 0x12, 0x2C, 0x32, 0xB6, 0xBF, 0xEC, 0x2F, 0x2A, 0xE8, 0xC3, 0x5A},
            {0xEF, 0xA7, 0x1F, 0x78, 0x89, 0x65, 0xBD, 0x44, 0x53, 0xF8, 0x60, 0x17, 0x8F, 0xC1, 0x91, 0x01},
            {0x57, 0xFF, 0x73, 0x9D, 0x4D, 0xC9, 0x2C, 0x1B, 0xD7, 0xFC, 0x01, 0x70, 0x0C, 0xC8, 0x21, 0x6F}
        };
        for (size_t i = 0; i < 3; ++i)
        {
            const auto ks = twofish::expand_key(helper::Bytes(16 + 8 * i, 0));
            helper::Bytes plain(16, 0), cipher(16), recovered(16);
            twofish::encrypt_block(ks, plain.data(), cipher.data());
            twofish::decrypt_block(ks, expected[i].data(), recovered.data());
            if (cipher != expected[i] || recovered != plain)
            {
                std::printf("FAIL: %zu-bit published test vector\n", (16 + 8 * i) * 8);
                return 1;
            }
        }
        std::printf("All %d message tests and 3 published test vectors passed.\n", passed);
        return 0;
    }
    catch (const std::exception& error)
    {
        std::printf("FAIL: %s\n", error.what());
        return 1;
    }
}
