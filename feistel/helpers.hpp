#pragma once
#include <limits>
#include <type_traits>


namespace helper
{
    namespace detail
    {
        inline uint8_t gf_mul(uint8_t a, uint8_t b, uint16_t poly)
        {
            uint16_t r = 0, aa = a;
            while (b)
            {
                if (b & 1) r ^= aa;
                aa <<= 1;
                if (aa & 0x100) aa ^= poly;
                b >>= 1;
            }
            return static_cast<uint8_t>(r);
        }
    } // namespace detail

    constexpr size_t BLOCK_BYTES = 16; // 128-bit block
    constexpr size_t BLOCK_WORDS = BLOCK_BYTES / 4; // 4 x 32-bit words
    constexpr int NUM_ROUNDS = 16;
    constexpr size_t WHITENING_WORDS = 8; // K[0..3] input + K[4..7] output
    constexpr size_t ROUND_KEY_OFFSET = WHITENING_WORDS; // round keys start at K[8]
    constexpr size_t NUM_SUBKEYS = WHITENING_WORDS + 2 * NUM_ROUNDS; // 40
    constexpr size_t KEY_CHUNK_BYTES = 8; // one 64-bit chunk of the key
    constexpr size_t MAX_K = 4; // k for a 256-bit key

    // Enum for directions of the bit rotations
    enum class Direction
    {
        LEFT,
        RIGHT
    };

    // Rotates the lowest `width` bits of x by `amount` positions.
    //   - T is any unsigned integer type (uint8_t, uint32_t, ...).
    //   - width defaults to the full size of T (32 for uint32_t), but can be
    //     smaller to rotate a narrower value stored in a bigger type, e.g. a
    //     4-bit nibble in a uint8_t:  rotate<uint8_t>(x, 1, Direction::RIGHT, 4)
    template <typename T>
    constexpr T rotate(T x, int amount, Direction direction,
                       int width = std::numeric_limits<T>::digits)
    {
        static_assert(std::is_unsigned<T>::value, "rotate needs an unsigned type");

        // Mask that keeps only the lowest `width` bits.
        const T mask = (width == std::numeric_limits<T>::digits)
                           ? static_cast<T>(~T(0))
                           : static_cast<T>((T(1) << width) - 1);

        x = static_cast<T>(x & mask);

        amount %= width;
        if (amount == 0) return x; // also avoids shifting by `width` (undefined)

        // A right rotation by n is a left rotation by (width - n).
        if (direction == Direction::RIGHT) amount = width - amount;

        return static_cast<T>(((x << amount) | (x >> (width - amount))) & mask);
    }

    // Reads a specific byte of a unit32_t variable
    inline uint8_t byte_of(uint32_t w, int i)
    {
        return static_cast<uint8_t>(w >> (8 * i));
    }

    // Reads 4 bytes from memory as one 32-bit word, little-endian:
    // p[0] becomes the lowest byte, p[3] the highest.
    inline uint32_t load_le32(const uint8_t* p)
    {
        return static_cast<uint32_t>(p[0])
            | (static_cast<uint32_t>(p[1]) << 8)
            | (static_cast<uint32_t>(p[2]) << 16)
            | (static_cast<uint32_t>(p[3]) << 24);
    }

    // The reverse: writes a 32-bit word to 4 bytes, lowest byte first.
    inline void store_le32(uint8_t* p, uint32_t v)
    {
        p[0] = static_cast<uint8_t>(v);
        p[1] = static_cast<uint8_t>(v >> 8);
        p[2] = static_cast<uint8_t>(v >> 16);
        p[3] = static_cast<uint8_t>(v >> 24);
    }


    template <size_t Rows, size_t Cols>
    inline uint32_t gf_matvec(const uint8_t (&matrix)[Rows][Cols],
                              const uint8_t* vec, uint16_t poly)
    {
        static_assert(Rows == 4, "the result must pack into exactly one 32-bit word");
        uint8_t out[4];
        for (size_t row = 0; row < Rows; ++row)
        {
            uint8_t acc = 0;
            for (size_t col = 0; col < Cols; ++col)
                acc ^= detail::gf_mul(matrix[row][col], vec[col], poly);
            out[row] = acc;
        }
        return load_le32(out);
    }

    // pseudo-Hardman transform: a'=a+b; b'=a+2b (both mod2^32)
    inline void pht(uint32_t a, uint32_t b, uint32_t& x, uint32_t& y)
    {
        x = a + b;
        y = a + 2 * b;
    }
} // namespace helper
