#include "text.hpp"
#include "feistel.hpp"
#include <stdexcept>

using namespace helper;

Bytes encrypt_text(const std::string& text, const Bytes& key)
{
    twofish::KeySchedule ks = twofish::expand_key(key);

    Bytes data(text.begin(), text.end());
    const auto pad = static_cast<uint8_t>(BLOCK_BYTES - (data.size() % BLOCK_BYTES));
    data.insert(data.end(), pad, pad);

    for (size_t i = 0; i < data.size(); i += BLOCK_BYTES)
        twofish::encrypt_block(ks, &data[i], &data[i]);
    return data;
}

std::string decrypt_text(const Bytes& cipher, const Bytes& key)
{
    if (cipher.empty() || cipher.size() % BLOCK_BYTES != 0)
        throw std::invalid_argument("ciphertext length must be a non-zero multiple of 16");

    twofish::KeySchedule ks = twofish::expand_key(key);

    Bytes data = cipher;
    for (size_t i = 0; i < data.size(); i += BLOCK_BYTES)
        twofish::decrypt_block(ks, &data[i], &data[i]);

    const uint8_t pad = data.back();
    if (pad == 0 || pad > BLOCK_BYTES || pad > data.size())
        throw std::runtime_error("bad padding (wrong key or corrupted data)");
    for (size_t i = data.size() - pad; i < data.size(); ++i)
        if (data[i] != pad)
            throw std::runtime_error("bad padding (wrong key or corrupted data)");

    data.resize(data.size() - pad);
    return std::string(data.begin(), data.end());
}
