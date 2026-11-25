#pragma once

#include <string>
#include "helpers.hpp"

// Encrypt arbitrary bytes stored in a string using independent 16-byte blocks
// and PKCS#7 padding. Embedded NUL bytes are preserved.
helper::Bytes encrypt_text(const std::string& text, const helper::Bytes& key);

// Decrypt and remove padding. Rejects invalid lengths, keys, and padding.
// Valid padding does not authenticate the ciphertext or detect every wrong key.
std::string decrypt_text(const helper::Bytes& cipher, const helper::Bytes& key);
