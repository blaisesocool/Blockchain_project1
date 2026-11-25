#pragma once
#include <cstddef>
#include <cstdint>

namespace twofish
{
    // Key-dependent S-box material: the vector S = (S_{k-1}, ..., S_0) derived
    // from the key with the Reed-Solomon code. Stored already in the order h()
    // expects it, so S[0] is S_{k-1}.
    struct SBoxKey
    {
        uint32_t S[4];
        size_t k; // number of 64-bit key chunks: 2 (128-bit), 3 (192), 4 (256)
    };

    // Fixed 8x8 permutations q0 / q1 (built from 4-bit lookup tables).
    uint8_t q0(uint8_t x);
    uint8_t q1(uint8_t x);

    // Multiplication in GF(2^8) modulo the given reduction polynomial.
    uint8_t gf_mul(uint8_t a, uint8_t b, uint16_t poly);

    // Reed-Solomon step: turn the raw key bytes (16/24/32) into the S-box key.
    SBoxKey make_sbox_key(const uint8_t* key, size_t key_len);

    // h(X, L): q-permutations keyed by the k words in L, followed by the MDS matrix.
    // Used by the key schedule (with Me / Mo) and by g() (with the S-box key).
    uint32_t h(uint32_t x, const uint32_t* L, size_t k);

    // g(X) = h(X, S): the key-dependent S-box layer used inside the round function.
    uint32_t g(uint32_t x, const SBoxKey& sk);
} // namespace twofish
