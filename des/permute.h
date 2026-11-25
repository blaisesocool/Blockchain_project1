#pragma once
#include <cstdint>

namespace permute
{

    uint64_t initial_permute(uint64_t x);

    uint64_t expand_permute(uint32_t x);

    uint64_t transpose(uint64_t x);

}