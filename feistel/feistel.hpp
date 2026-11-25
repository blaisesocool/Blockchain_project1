#pragma once
#include <cstdint>
#include <tuple>

#include "keys.hpp"

namespace twofish
{
    // Round function F: outputs (F0, F1) for the two left-hand words (R0, R1)
    // of round `round` (0..15).
    std::tuple<uint32_t, uint32_t> round_function(const KeySchedule& ks, uint32_t r0, uint32_t r1, int round);

    // Single 16-byte block, in and out may alias.
    void encrypt_block(const KeySchedule& ks, const uint8_t in[16], uint8_t out[16]);
    void decrypt_block(const KeySchedule& ks, const uint8_t in[16], uint8_t out[16]);
} // namespace twofish
