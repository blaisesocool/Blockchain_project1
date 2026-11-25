#pragma once
#include <cstddef>
#include <cstdint>

#include "helpers.hpp"
#include "sbox.hpp"

namespace twofish
{
    struct KeySchedule
    {
        uint32_t K[40]; // K[0..3] input whitening, K[4..7] output whitening,
                        // K[8..39] round subkeys (2 per round, 16 rounds)

        SBoxKey sbox; // key-dependent S-box material used by g()
    };

    // Expands a 16/24/32-byte key. Throws std::invalid_argument for other lengths.
    KeySchedule expand_key(const helper::Bytes& key);
} // namespace twofish
