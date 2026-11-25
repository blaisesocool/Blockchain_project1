#include "sbox.hpp"
#include "helpers.hpp"

namespace twofish
{
    using namespace helper;

    namespace
    {
        constexpr uint16_t MDS_POLY = 0x169; // x^8 + x^6 + x^5 + x^3 + 1
        constexpr uint16_t RS_POLY = 0x14D; // x^8 + x^6 + x^3 + x^2 + 1

        // 4-bit tables from the Twofish spec: {t0, t1, t2, t3} for q0 and q1.
        const uint8_t Q0_T[4][16] = {
            {0x8, 0x1, 0x7, 0xD, 0x6, 0xF, 0x3, 0x2, 0x0, 0xB, 0x5, 0x9, 0xE, 0xC, 0xA, 0x4},
            {0xE, 0xC, 0xB, 0x8, 0x1, 0x2, 0x3, 0x5, 0xF, 0x4, 0xA, 0x6, 0x7, 0x0, 0x9, 0xD},
            {0xB, 0xA, 0x5, 0xE, 0x6, 0xD, 0x9, 0x0, 0xC, 0x8, 0xF, 0x3, 0x2, 0x4, 0x7, 0x1},
            {0xD, 0x7, 0xF, 0x4, 0x1, 0x2, 0x6, 0xE, 0x9, 0xB, 0x3, 0x0, 0x8, 0x5, 0xC, 0xA},
        };
        const uint8_t Q1_T[4][16] = {
            {0x2, 0x8, 0xB, 0xD, 0xF, 0x7, 0x6, 0xE, 0x3, 0x1, 0x9, 0x4, 0x0, 0xA, 0xC, 0x5},
            {0x1, 0xE, 0x2, 0xB, 0x4, 0xC, 0x3, 0x7, 0x6, 0xD, 0xA, 0x5, 0xF, 0x9, 0x0, 0x8},
            {0x4, 0xC, 0x7, 0x5, 0x1, 0x6, 0x9, 0xA, 0x0, 0xE, 0xD, 0x8, 0x2, 0xB, 0x3, 0xF},
            {0xB, 0x9, 0x5, 0x1, 0xC, 0x3, 0xD, 0xE, 0x6, 0x4, 0x7, 0xF, 0x2, 0x0, 0x8, 0xA},
        };


        uint8_t q_perm(uint8_t x, const uint8_t t[4][16])
        {
            uint8_t a0 = x >> 4, b0 = x & 0xF;
            uint8_t a1 = a0 ^ b0;
            uint8_t b1 = (a0 ^ rotate<uint8_t>(b0, 1, Direction::RIGHT, 4) ^ (a0 << 3)) & 0xF;
            uint8_t a2 = t[0][a1], b2 = t[1][b1];
            uint8_t a3 = a2 ^ b2;
            uint8_t b3 = (a2 ^ rotate<uint8_t>(b2, 1, Direction::RIGHT, 4) ^ (a2 << 3)) & 0xF;
            uint8_t a4 = t[2][a3], b4 = t[3][b3];
            return static_cast<uint8_t>((b4 << 4) | a4);
        }

        // Full 256-entry tables, built once on first use.
        struct QTables
        {
            uint8_t q0[256], q1[256];

            QTables()
            {
                for (int i = 0; i < 256; ++i)
                {
                    q0[i] = q_perm(static_cast<uint8_t>(i), Q0_T);
                    q1[i] = q_perm(static_cast<uint8_t>(i), Q1_T);
                }
            }
        };

        const QTables& tables()
        {
            static const QTables t;
            return t;
        }

        const uint8_t MDS[4][4] = {
            {0x01, 0xEF, 0x5B, 0x5B},
            {0x5B, 0xEF, 0xEF, 0x01},
            {0xEF, 0x5B, 0x01, 0xEF},
            {0xEF, 0x01, 0xEF, 0x5B},
        };

        const uint8_t RS[4][8] = {
            {0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E},
            {0xA4, 0x56, 0x82, 0xF3, 0x1E, 0xC6, 0x68, 0xE5},
            {0x02, 0xA1, 0xFC, 0xC1, 0x47, 0xAE, 0x3D, 0x19},
            {0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, 0x03},
        };
    } // namespace

    uint8_t q0(uint8_t x) { return tables().q0[x]; }
    uint8_t q1(uint8_t x) { return tables().q1[x]; }

    SBoxKey make_sbox_key(const uint8_t* key, size_t key_len)
    {
        SBoxKey sk{};
        sk.k = key_len / 8;
        for (size_t i = 0; i < sk.k; ++i)
            sk.S[sk.k - 1 - i] = gf_matvec(RS, key + KEY_CHUNK_BYTES * i, RS_POLY);
        return sk;
    }

    uint32_t h(uint32_t x, const uint32_t* L, size_t k)
    {
        const QTables& t = tables();
        uint8_t y[4] = {byte_of(x, 0), byte_of(x, 1), byte_of(x, 2), byte_of(x, 3)};

        if (k == 4)
        {
            y[0] = t.q1[y[0]] ^ byte_of(L[3], 0);
            y[1] = t.q0[y[1]] ^ byte_of(L[3], 1);
            y[2] = t.q0[y[2]] ^ byte_of(L[3], 2);
            y[3] = t.q1[y[3]] ^ byte_of(L[3], 3);
        }
        if (k >= 3)
        {
            y[0] = t.q1[y[0]] ^ byte_of(L[2], 0);
            y[1] = t.q1[y[1]] ^ byte_of(L[2], 1);
            y[2] = t.q0[y[2]] ^ byte_of(L[2], 2);
            y[3] = t.q0[y[3]] ^ byte_of(L[2], 3);
        }
        y[0] = t.q1[t.q0[t.q0[y[0]] ^ byte_of(L[1], 0)] ^ byte_of(L[0], 0)];
        y[1] = t.q0[t.q0[t.q1[y[1]] ^ byte_of(L[1], 1)] ^ byte_of(L[0], 1)];
        y[2] = t.q1[t.q1[t.q0[y[2]] ^ byte_of(L[1], 2)] ^ byte_of(L[0], 2)];
        y[3] = t.q0[t.q1[t.q1[y[3]] ^ byte_of(L[1], 3)] ^ byte_of(L[0], 3)];

        return gf_matvec(MDS, y, MDS_POLY);
    }

    uint32_t g(uint32_t x, const SBoxKey& sk)
    {
        return h(x, sk.S, sk.k);
    }
} // namespace twofish
