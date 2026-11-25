#include "feistel.hpp"
#include "helpers.hpp"
#include <utility>

namespace twofish
{

    using namespace helper;

    std::tuple<uint32_t, uint32_t> round_function(const KeySchedule& ks, uint32_t r0, uint32_t r1, int round)
    {
        uint32_t f0, f1;
        uint32_t t0 = g(r0, ks.sbox);
        uint32_t t1 = g(rotate(r1, 8, Direction::LEFT), ks.sbox);

        pht(t0, t1, f0, f1);
        f0 += ks.K[2 * round + ROUND_KEY_OFFSET];
        f1 += ks.K[2 * round + ROUND_KEY_OFFSET + 1];

        return {f0, f1};
    }

    void encrypt_block(const KeySchedule& ks, const uint8_t in[16], uint8_t out[16])
    {
        uint32_t r[4];
        for (int i = 0; i < 4; ++i) r[i] = load_le32(in + 4 * i) ^ ks.K[i];

        for (int round = 0; round < ROUNDS; ++round)
        {
            auto [f0, f1] = round_function(ks, r[0], r[1], round);
            r[2] = rotate(r[2] ^ f0, 1, Direction::RIGHT);
            r[3] = rotate(r[3], 1, Direction::LEFT) ^ f1;
            std::swap(r[0], r[2]);
            std::swap(r[1], r[3]);
        }

        // The last swap is undone by the output permutation (R2,R3,R0,R1).
        store_le32(out + 0, r[2] ^ ks.K[4]);
        store_le32(out + 4, r[3] ^ ks.K[5]);
        store_le32(out + 8, r[0] ^ ks.K[6]);
        store_le32(out + 12, r[1] ^ ks.K[7]);
    }

    void decrypt_block(const KeySchedule& ks, const uint8_t in[16], uint8_t out[16])
    {
        uint32_t r[4];
        r[2] = load_le32(in + 0) ^ ks.K[4];
        r[3] = load_le32(in + 4) ^ ks.K[5];
        r[0] = load_le32(in + 8) ^ ks.K[6];
        r[1] = load_le32(in + 12) ^ ks.K[7];

        for (int round = ROUNDS - 1; round >= 0; --round)
        {
            std::swap(r[0], r[2]);
            std::swap(r[1], r[3]);
            auto [f0, f1] = round_function(ks, r[0], r[1], round);
            r[2] = rotate(r[2], 1, Direction::LEFT) ^ f0;
            r[3] = rotate(r[3] ^ f1, 1, Direction::RIGHT);
        }

        for (int i = 0; i < 4; ++i) store_le32(out + 4 * i, r[i] ^ ks.K[i]);
    }
} // namespace twofish
