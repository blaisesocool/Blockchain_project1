#include "keys.hpp"
#include <stdexcept>

#include "helpers.hpp"

namespace twofish
{
    namespace
    {
        constexpr uint32_t RHO = 0x01010101;
    } // namespace

    using namespace helper;

    KeySchedule expand_key(const uint8_t* key, size_t key_len)
    {
        if (key_len != 16 && key_len != 24 && key_len != 32)
            throw std::invalid_argument("Twofish key must be 16, 24 or 32 bytes");

        const size_t k = key_len / 8;

        // Split the key into 32-bit words, then into even (Me) and odd (Mo) halves.
        uint32_t Me[4], Mo[4];
        for (size_t i = 0; i < k; ++i)
        {
            Me[i] = load_le32(key + 8 * i);
            Mo[i] = load_le32(key + 8 * i + 4);
        }

        KeySchedule ks{};
        ks.sbox = make_sbox_key(key, key_len);

        for (uint32_t i = 0; i < 20; ++i)
        {
            uint32_t A = h(2 * i * RHO, Me, k);
            uint32_t B = rotate(h((2 * i + 1) * RHO, Mo, k), 8, Direction::LEFT);

            uint32_t x, y;
            pht(A, B, x, y);
            ks.K[2 * i] = x;
            ks.K[2 * i + 1] = rotate(y, 9, Direction::LEFT);
        }
        return ks;
    }
} // namespace twofish
