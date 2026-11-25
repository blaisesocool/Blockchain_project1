#include "permute.h"

#include <array>
#include <cstddef>
#include <cstdint>


namespace permute
{
    namespace detail
    {
        constexpr std::array<int, 64> INITIAL_TABLE = {
            57, 49, 41, 33, 25, 17, 9, 1,
            59, 51, 43, 35, 27, 19, 11, 3,
            61, 53, 45, 37, 29, 21, 13, 5,
            63, 55, 47, 39, 31, 23, 15, 7,
            56, 48, 40, 32, 24, 16, 8, 0,
            58, 50, 42, 34, 26, 18, 10, 2,
            60, 52, 44, 36, 28, 20, 12, 4,
            62, 54, 46, 38, 30, 22, 14, 6
        };

        constexpr std::array<int, 48> EXPANSION_TABLE = {
            31, 0, 1, 2, 3, 4,
            3, 4, 5, 6, 7, 8,
            7, 8, 9, 10, 11, 12,
            11, 12, 13, 14, 15, 16,
            15, 16, 17, 18, 19, 20,
            19, 20, 21, 22, 23, 24,
            23, 24, 25, 26, 27, 28,
            27, 28, 29, 30, 31, 0
        };

        constexpr std::array<int, 32> PERMUTE_TABLE = {
            16, 7, 20, 21, 29, 12, 28, 17,
            1, 15, 23, 26, 5, 18, 31, 10,
            2, 8, 24, 14, 32, 27, 3, 9,
            19, 13, 30, 6, 22, 11, 4, 25,
        };


        template <size_t N>
        uint64_t permute(uint64_t x, const std::array<int, N>& table, size_t bits)
        {
            uint64_t result = 0;

            for (size_t i; i < N; i++)
            {
                size_t output_pos = N - 1 - i;
                size_t input_pos = bits - table[i];

                uint64_t t = (x >> input_pos) & 1ULL;
                if (t) result |= t << output_pos;
            }

            return result;
        }
    }

    uint64_t initial_permute(uint64_t x)
    {
        return detail::permute(x, detail::INITIAL_TABLE, 64);
    }

    uint64_t expand_permute(uint32_t x)
    {
        return detail::permute(x, detail::EXPANSION_TABLE, 32);
    }

    uint32_t transpose(uint32_t x)
    {
        return detail::permute(x, detail::PERMUTE_TABLE, 32);
    }
}
