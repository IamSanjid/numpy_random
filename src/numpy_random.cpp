#include <cassert>
#include <iostream>
#include "numpy_random.h"

extern "C" {
#include "numpy/random/bitgen.h"
#include "numpy/random/distributions.h"
#include "numpy/random/legacy/legacy-distributions.h"
}

void internal_random_state::init(void* raw_engine, uint64_t (*next_uint64)(void* st),
                                 uint32_t (*next_uint32)(void* st), double (*next_double)(void* st),
                                 uint64_t (*next_raw)(void* st)) {
    _bitgen = (bitgen_t*)malloc(sizeof(bitgen_t));
    if (_bitgen == nullptr) {
        return;
    }

    _bitgen->state = raw_engine;
    _bitgen->next_uint64 = next_uint64;
    _bitgen->next_uint32 = next_uint32;
    _bitgen->next_double = next_double;
    _bitgen->next_raw = next_raw;

    _aug_state = (aug_bitgen_t*)malloc(sizeof(aug_bitgen_t));
    if (_aug_state == nullptr) {
        return;
    }

    _aug_state->bit_generator = _bitgen;
    _aug_state->has_gauss = 0;
    _aug_state->gauss = 0.0;

    _binomial = (binomial_t*)malloc(sizeof(binomial_t));
}

void internal_random_state::uninit() {
    if (_bitgen == nullptr) {
        return;
    }

    free(_bitgen);
    free(_aug_state);
    free(_binomial);
    _bitgen = nullptr;
    _aug_state = nullptr;
    _binomial = nullptr;
}

uint32_t INIT_A = 0x43b0d7e5;
uint32_t MULT_A = 0x931e8875;
uint32_t INIT_B = 0x8b51f9dd;
uint32_t MULT_B = 0x58f38ded;
uint32_t MIX_MULT_L = 0xca01f9dd;
uint32_t MIX_MULT_R = 0x4973f715;
uint32_t XSHIFT = sizeof(uint32_t) * 8 / 2;
uint32_t MASK32 = 0xFFFFFFFF;

uint32_t hashmix(uint32_t value, uint32_t* hash_const) {
    value ^= hash_const[0];
    hash_const[0] *= MULT_A;
    value *= hash_const[0];
    value ^= value >> XSHIFT;
    return value;
}

uint32_t mix(uint32_t x, uint32_t y) {
    uint32_t result = (MIX_MULT_L * x - MIX_MULT_R * y);
    result ^= result >> XSHIFT;
    return result;
}

uint32_t rand_range_low(uint32_t low, uint32_t high) {
    uint32_t val;
    assert(low < high);
    assert(high - low <= RAND_MAX);

    uint32_t range = high - low;
    uint32_t scale = RAND_MAX / range;
    do {
        val = rand();
    } while (val >= scale * range);
    return val / scale + low;
}

uint32_t rand_range(uint32_t low, uint32_t high) {
    assert(high > low);
    uint32_t val;
    uint32_t range = high - low;
    if (range < RAND_MAX)
        return rand_range_low(low, high);
    uint32_t scale = range / RAND_MAX;
    do {
        val = rand() + rand_range(0, scale) * RAND_MAX;
    } while (val >= range);
    return val + low;
}

std::vector<uint32_t> rand_uints(size_t count) {
    std::vector<uint32_t> rands(count, 0);
    for (auto& r : rands) {
        r = rand_range(0, UINT32_MAX - 1);
    }
    return rands;
}

internal_numpy_seed_sequence::internal_numpy_seed_sequence(size_t pool_size)
    : _pool(pool_size, 0), _last_hash_const(INIT_B) {}

void internal_numpy_seed_sequence::set_entropy(std::vector<uint32_t>&& entropy) {
    _entropy = entropy;
}

uint32_t internal_numpy_seed_sequence::generate() {
    uint32_t hash_const = _last_hash_const;

    uint32_t state = _pool[_pool_idx];
    state ^= hash_const;
    hash_const *= MULT_B;
    state *= hash_const;
    state ^= state >> XSHIFT;

    _pool_idx++;
    if (_pool_idx >= _pool.size()) {
        _pool_idx = 0;
    }
    _last_hash_const = hash_const;

    return state;
}

void internal_numpy_seed_sequence::mix_entropy() {
    size_t pool_size = _pool.size();
    if (_entropy.size() <= 0) {
        _entropy = rand_uints(pool_size);
    }
    uint32_t hash_const[1]{INIT_A};
    size_t entropy_size = _entropy.size();

    for (size_t i = 0; i < pool_size; i++) {
        if (i < entropy_size) {
            _pool[i] = hashmix(_entropy[i], hash_const);
        }
        else {
            _pool[i] = hashmix(0, hash_const);
        }
    }

    for (size_t i_src = 0; i_src < pool_size; i_src++) {
        for (size_t i_dst = 0; i_dst < pool_size; i_dst++) {
            if (i_src != i_dst) {
                _pool[i_dst] = mix(_pool[i_dst], hashmix(_pool[i_src], hash_const));
            }
        }
    }

    for (size_t i_src = pool_size; i_src < entropy_size; i_src++) {
        for (size_t i_dst = 0; i_dst < pool_size; i_dst++) {
            if (i_src != i_dst) {
                _pool[i_dst] = mix(_pool[i_dst], hashmix(_entropy[i_src], hash_const));
            }
        }
    }
}