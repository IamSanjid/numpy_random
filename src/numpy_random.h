#pragma once
#include <cstdint>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>

struct bitgen;
struct aug_bitgen;
struct s_binomial_t;

namespace numpy_random_internel {
extern "C" {
void random_bounded_uint64_fill(bitgen* bitgen_state, uint64_t off, uint64_t rng, intptr_t cnt,
                                bool use_masked, uint64_t* out);
void random_bounded_uint32_fill(bitgen* bitgen_state, uint32_t off, uint32_t rng, intptr_t cnt,
                                bool use_masked, uint32_t* out);
void random_bounded_uint16_fill(bitgen* bitgen_state, uint16_t off, uint16_t rng, intptr_t cnt,
                                bool use_masked, uint16_t* out);
void random_bounded_uint8_fill(bitgen* bitgen_state, uint8_t off, uint8_t rng, intptr_t cnt,
                               bool use_masked, uint8_t* out);
void random_bounded_bool_fill(bitgen* bitgen_state, unsigned char off, unsigned char rng,
                              intptr_t cnt, bool use_masked, unsigned char* out);

double random_uniform(bitgen* bitgen_state, double lower, double range);

double legacy_beta(aug_bitgen* aug_state, double a, double b);
int64_t legacy_random_binomial(bitgen* bitgen_state, double p, int64_t n, s_binomial_t* binomial);
double legacy_gauss(aug_bitgen* aug_state);
}
} // namespace numpy_random_internel

template <class SrcIter, class DestIter>
SrcIter uneven_copy(SrcIter src_first, DestIter dest_first, DestIter dest_last, std::true_type) {
    typedef typename std::iterator_traits<SrcIter>::value_type src_t;
    typedef typename std::iterator_traits<DestIter>::value_type dest_t;

    constexpr uint8_t SRC_SIZE = sizeof(src_t);
    constexpr uint8_t DEST_SIZE = sizeof(dest_t);
    constexpr uint8_t DEST_BITS = DEST_SIZE * CHAR_BIT;
    constexpr uint8_t SCALE = SRC_SIZE / DEST_SIZE;

    size_t count = 0;
    src_t value = 0;

    while (dest_first != dest_last) {
        if ((count++ % SCALE) == 0)
            value = *src_first++;
        else
            value >>= DEST_BITS;

        *dest_first++ = dest_t(value);
    }
    return src_first;
}

template <class SrcIter, class DestIter>
SrcIter uneven_copy(SrcIter src_first, DestIter dest_first, DestIter dest_last, std::false_type) {
    typedef typename std::iterator_traits<SrcIter>::value_type src_t;
    typedef typename std::iterator_traits<DestIter>::value_type dest_t;

    constexpr auto SRC_SIZE = sizeof(src_t);
    constexpr auto SRC_BITS = SRC_SIZE * CHAR_BIT;
    constexpr auto DEST_SIZE = sizeof(dest_t);
    constexpr auto SCALE = (DEST_SIZE + SRC_SIZE - 1) / SRC_SIZE;

    while (dest_first != dest_last) {
        dest_t value(0UL);
        unsigned int shift = 0;

        for (size_t i = 0; i < SCALE; ++i) {
            value |= dest_t(*src_first++) << shift;
            shift += SRC_BITS;
        }

        *dest_first++ = value;
    }
    return src_first;
}

template <class IdxRef, class DestIter>
size_t uneven_copy_safe(const IdxRef& src, size_t src_size, DestIter dest_first, DestIter dest_last,
                        std::true_type) {
    typedef decltype(std::declval<IdxRef>()[0]) src_t;
    typedef typename std::iterator_traits<DestIter>::value_type dest_t;

    constexpr uint8_t SRC_SIZE = sizeof(src_t);
    constexpr uint8_t DEST_SIZE = sizeof(dest_t);
    constexpr uint8_t DEST_BITS = DEST_SIZE * CHAR_BIT;
    constexpr uint8_t SCALE = SRC_SIZE / DEST_SIZE;

    size_t count = 0;
    src_t value = 0;
    size_t idx = 0;

    while (dest_first != dest_last) {
        if (idx >= src_size) {
            return src_size;
        }

        if ((count++ % SCALE) == 0)
            value = src[idx++];
        else
            value >>= DEST_BITS;

        *dest_first++ = dest_t(value);
    }

    return idx;
}

template <class IdxRef, class DestIter>
size_t uneven_copy_safe(IdxRef& src, size_t src_size, DestIter dest_first, DestIter dest_last,
                        std::false_type) {
    typedef decltype(std::declval<IdxRef>()[0]) src_t;
    typedef typename std::iterator_traits<DestIter>::value_type dest_t;

    constexpr size_t SRC_SIZE = sizeof(src_t);
    constexpr size_t SRC_BITS = SRC_SIZE * CHAR_BIT;
    constexpr size_t DEST_SIZE = sizeof(dest_t);
    constexpr size_t SCALE = (DEST_SIZE + SRC_SIZE - 1u) / SRC_SIZE;

    size_t idx = 0;

    while (dest_first != dest_last) {
        dest_t value(0UL);
        unsigned int shift = 0;

        for (size_t i = 0; i < SCALE && idx < src_size; ++i, idx++) {
            value |= dest_t(src[idx]) << shift;
            shift += SRC_BITS;
        }

        *dest_first++ = value;
    }

    return idx;
}

struct internal_random_state {
    template <typename RngEngine>
    friend class RandomState;

private:
    void init(void* raw_engine, uint64_t (*next_uint64)(void* st),
              uint32_t (*next_uint32)(void* st), double (*next_double)(void* st),
              uint64_t (*next_raw)(void* st));
    void uninit();

    bitgen* _bitgen = nullptr;
    aug_bitgen* _aug_state = nullptr;
    s_binomial_t* _binomial = nullptr;
};

template <class T>
using raw_type = typename std::remove_cv_t<std::remove_reference_t<T>>;

template <class F, class T, class = T>
struct _is_static_castable : public std::false_type {};

template <class F, class T>
struct _is_static_castable<F, T, decltype(static_cast<T>(std::declval<F>()))> : std::true_type {};

template <class F, class T>
_INLINE_VAR constexpr bool is_static_castable_v =
    _is_static_castable<raw_type<F>, raw_type<T>>::value;

template <class F, class T>
struct is_static_castable : std::bool_constant<is_static_castable_v<F, T>> {};

template <class T, class... Types>
_INLINE_VAR constexpr bool is_any_static_castable_v =
    std::disjunction_v<is_static_castable<T, Types>...>;

template <class T>
_INLINE_VAR constexpr bool is_arithmetic_castable_v =
    is_any_static_castable_v<T, bool, char, signed char, unsigned char, wchar_t,
#ifdef __cpp_char8_t
                             char8_t,
#endif // __cpp_char8_t
                             char16_t, char32_t, short, unsigned short, int, unsigned int, long,
                             unsigned long, long long, unsigned long long>;

template <typename RngEngine>
class RandomState {
    template <typename, typename = void>
    struct has_bracket_overload : std::false_type {};

    template <typename T>
    struct has_bracket_overload<T, std::void_t<decltype(std::declval<T&>()[0])>> : std::true_type {
    };

    template <typename, typename = void>
    struct has_shr_overload : std::false_type {};

    template <typename T>
    struct has_shr_overload<T, std::void_t<decltype(std::declval<T&>() >> ((T)0))>>
        : std::true_type {};

    template <typename, typename = void>
    struct has_and_overload : std::false_type {};

    template <typename T>
    struct has_and_overload<T, std::void_t<decltype(std::declval<T&>() & ((T)0))>>
        : std::true_type {};

    template <typename, typename = void>
    struct has_size_fn : std::false_type {};

    template <typename T>
    struct has_size_fn<T, std::void_t<decltype(std::declval<T&>().size())>> : std::true_type {};

    template <typename T>
    static constexpr bool valid_custom_arithmetic() {
        if constexpr (has_shr_overload<T>::value && has_and_overload<T>::value) {
            using type_shr = decltype(std::declval<T>() >> ((T)0));
            using type_and = decltype(std::declval<T>() & ((T)0));
            return is_arithmetic_castable_v<raw_type<type_shr>> &&
                   is_arithmetic_castable_v<raw_type<type_and>>;
        }
        else {
            return false;
        }
    }

    template <typename T>
    static constexpr bool valid_container() {
        if constexpr (has_bracket_overload<T>::value) {
            if constexpr (has_size_fn<T>::value == false && sizeof(T) % 2 != 0) {
                return false;
            }
            else {
                using type = decltype(std::declval<T>()[0]);
                return std::is_arithmetic_v<raw_type<type>> || valid_custom_arithmetic<type>();
            }
        }
        else {
            return false;
        }
    }

    using RngReturn = typename raw_type<decltype(std::declval<RngEngine>()())>;
    static constexpr bool is_arithmetic = std::is_arithmetic_v<RngReturn>;
    static constexpr bool is_container_arithmetic = valid_container<RngReturn>();
    static constexpr bool is_custom_arithmetic =
        is_arithmetic_castable_v<RngReturn> && valid_custom_arithmetic<RngReturn>();

    static_assert(
        is_arithmetic || is_container_arithmetic || is_custom_arithmetic,
        "**RngEngine** must implement operator(), the return type can be an"
        "*arithmetic type* or an *arithmetic container type* or a *custom arithmetic type*"
        "which must implement *operator>>* and *operator&* and can be *explicitly/implicitly"
        "cast to* another arithmetic type. (eg. maybe a custom uint128_t)");

public:
    RandomState() {
        init();
    }

    template <typename... Ts>
    RandomState(Ts&&... args) : _engine{std::forward<Ts>(args)...} {
        init();
    }

    ~RandomState() {
        std::lock_guard lock{mutex};
        _internal_state.uninit();
    }

    RngEngine& get_engine() {
        std::lock_guard lock{mutex};
        return _engine;
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    T beta(T a, T b) {
        if (_internal_state._bitgen == nullptr || _internal_state._aug_state == nullptr) {
            return (T)0;
        }
        std::lock_guard lock{mutex};
        return (T)legacy_beta(_internal_state._aug_state, (double)a, (double)b);
    }

    template <typename T, typename U,
              std::enable_if_t<std::is_arithmetic_v<T> && std::is_floating_point_v<U>, bool> = true>
    int64_t binomial(T n, U p) {
        if (_internal_state._bitgen == nullptr || _internal_state._binomial == nullptr) {
            return 0LL;
        }
        std::lock_guard lock{mutex};
        return legacy_random_binomial(_internal_state._bitgen, (double)p, (int64_t)n,
                                      _internal_state._binomial);
    }

    template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
    T uniform(T high) {
        T low = (T)0;
        return (T)uniform(low, high);
    }

    template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
    T uniform(T low, T high) {
        double _low = (double)low;
        double _high = (double)high;
        double range = _high - _low;
        if (!isfinite(range) || _internal_state._bitgen == nullptr) {
            return (T)0;
        }
        std::lock_guard lock{mutex};
        return (T)numpy_random_internel::random_uniform(_internal_state._bitgen, _low, range);
    }

    template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    T rand_int(T high) {
        T low = (T)0;
        return (T)rand_int(low, high);
    }

    template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    T rand_int(T low, T high) {
        if (_internal_state._bitgen == nullptr) {
            return (T)0;
        }
        std::lock_guard lock{mutex};
        return (T)random_bounded_fill(low, high - low, 1, true);
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    T rand_n() {
        if (_internal_state._bitgen == nullptr || _internal_state._aug_state == nullptr) {
            return (T)0;
        }
        std::lock_guard lock{mutex};
        return (T)numpy_random_internel::legacy_gauss(_internal_state._aug_state);
    }

private:
    template <typename T = bool>
    inline bool random_bounded_fill(bool off, bool rng, intptr_t cnt, bool use_masked) {
        unsigned char out_val = 0;
        numpy_random_internel::random_bounded_bool_fill(_internal_state._bitgen, (unsigned char)off,
                                                        (unsigned char)rng, cnt, use_masked,
                                                        &out_val);
        return (bool)out_val;
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t>,
                                           bool> = true>
    inline T random_bounded_fill(T off, T rng, intptr_t cnt, bool use_masked) {
        uint8_t out_val = 0;
        numpy_random_internel::random_bounded_uint8_fill(_internal_state._bitgen, (uint8_t)off,
                                                         (uint8_t)rng, cnt, use_masked, &out_val);
        return (T)out_val;
    }

    template <
        typename T,
        std::enable_if_t<std::is_same_v<T, uint16_t> || std::is_same_v<T, int16_t>, bool> = true>
    inline T random_bounded_fill(T off, T rng, intptr_t cnt, bool use_masked) {
        uint16_t out_val = 0;
        numpy_random_internel::random_bounded_uint16_fill(_internal_state._bitgen, (uint16_t)off,
                                                          (uint16_t)rng, cnt, use_masked, &out_val);
        return (T)out_val;
    }

    template <
        typename T,
        std::enable_if_t<std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t>, bool> = true>
    inline T random_bounded_fill(T off, T rng, intptr_t cnt, bool use_masked) {
        uint32_t out_val = 0;
        numpy_random_internel::random_bounded_uint32_fill(_internal_state._bitgen, (uint32_t)off,
                                                          (uint32_t)rng, cnt, use_masked, &out_val);
        return (T)out_val;
    }

    template <
        typename T,
        std::enable_if_t<std::is_same_v<T, uint64_t> || std::is_same_v<T, int64_t>, bool> = true>
    inline T random_bounded_fill(T off, T rng, intptr_t cnt, bool use_masked) {
        uint64_t out_val = 0;
        numpy_random_internel::random_bounded_uint64_fill(_internal_state._bitgen, (uint64_t)off,
                                                          (uint64_t)rng, cnt, use_masked, &out_val);
        return (T)out_val;
    }

    bool get_from_container(uint64_t& out) {
        if (_uintegers_cnt > 0) {
            _uintegers_cnt -= 1;
            out = _uintegers.front();
            _uintegers.pop_front();

            return true;
        }
        return false;
    }

private:
    template <typename Src, typename Dest>
    static size_t copy_to_container(const Src& src, Dest& dest) {
        typedef decltype(std::declval<Src>()[0]) src_type;
        typedef typename std::decay<decltype(*dest.begin())>::type dest_type;

        constexpr bool DEST_IS_SMALLER = sizeof(dest_type) < sizeof(src_type);
        constexpr size_t SRC_SIZE = sizeof(src_type);
        constexpr size_t DEST_SIZE = sizeof(dest_type);
        constexpr size_t SCALE =
            DEST_IS_SMALLER ? SRC_SIZE / DEST_SIZE : (DEST_SIZE + SRC_SIZE - 1u) / SRC_SIZE;

        size_t last_cnt = (size_t)dest.size();
        size_t src_size = 0;

        if constexpr (has_size_fn<Src>::value) {
            src_size = (size_t)src.size();
        }
        else {
            src_size = sizeof(src) / SRC_SIZE;
        }

        size_t new_cnt = src_size;
        if constexpr (DEST_IS_SMALLER) {
            new_cnt *= SCALE;
        }
        else {
            new_cnt /= SCALE;
        }
        new_cnt += last_cnt;

        dest.resize(new_cnt);

        auto begin = dest.begin();
        std::advance(begin, (int)last_cnt);

        uneven_copy_safe(src, src_size, begin, dest.end(), std::bool_constant<DEST_IS_SMALLER>{});

        return new_cnt - last_cnt;
    }

    static inline uint64_t get_raw(void* ptr) {
        auto& _this = *(RandomState<RngEngine>*)ptr;
        auto& _engine = _this._engine;
        auto& _uintegers_cnt = _this._uintegers_cnt;
        auto& _uintegers = _this._uintegers;

        if constexpr (is_arithmetic) {
            return (uint64_t)_engine();
        }
        else {
            uint64_t next = 0;
            if (_this.get_from_container(next)) {
                return next;
            }

            if constexpr (is_container_arithmetic) {
                RngReturn container = _engine();
                _uintegers_cnt += copy_to_container(container, _uintegers);
            }
            else {
                if constexpr (sizeof(uint64_t) < sizeof(RngReturn)) {
                    RngReturn container[1]{_engine()};
                    _uintegers_cnt += copy_to_container(container, _uintegers);
                }
                else {
                    return (uint64_t)_engine();
                }
            }

            _this.get_from_container(next);
            return next;
        }
    }

    template <
        typename T,
        std::enable_if_t<std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>, bool> = true>
    static inline T get(void* ptr) {
        constexpr bool IS_32BIT =
            (is_arithmetic || is_custom_arithmetic) && sizeof(RngReturn) <= sizeof(uint32_t);

        auto& _this = *(RandomState<RngEngine>*)ptr;
        auto& _engine = _this._engine;
        auto& _has_integer = _this._has_integer;
        auto& _uinteger = _this._uinteger;

        if constexpr (std::is_same_v<T, uint32_t>) {
            if constexpr (IS_32BIT) {
                return (T)get_raw(ptr);
            }
            else {
                if (_has_integer) {
                    _has_integer = false;
                    return _uinteger;
                }
                uint64_t next = get_raw(ptr);
                _has_integer = true;
                _uinteger = (T)(next >> 32);
                return (T)(next & 0xffffffff);
            }
        }
        else {
            if constexpr (IS_32BIT) {
                return (T)get_raw(ptr) << 32 | get_raw(ptr);
            }
            else {
                return (T)get_raw(ptr);
            }
        }
    }

    static uint64_t next_uint64(void* ptr) {
        return get<uint64_t>(ptr);
    }

    static uint32_t next_uint32(void* ptr) {
        return get<uint32_t>(ptr);
    }

    static double next_double(void* ptr) {
        if constexpr (is_arithmetic) {
            auto& _this = *(RandomState<RngEngine>*)ptr;
            auto& _engine = _this._engine;

            if constexpr (sizeof(RngReturn) <= sizeof(uint32_t)) {
                int32_t a = _engine() >> 5, b = _engine() >> 6;
                return (a * 67108864.0 + b) / 9007199254740992.0;
            }
            else {
                uint64_t rnd = (uint64_t)_engine();
                return (double)((rnd >> 11) * (1.0 / 9007199254740992.0));
            }
        }
        else {
            uint64_t rnd = get_raw(ptr);
            return ((double)(rnd >> 11) * (1.0 / 9007199254740992.0));
        }
    }

    static uint64_t next_raw(void* ptr) {
        return get_raw(ptr);
    }

private:
    void init() {
        std::lock_guard lock{mutex};
        _internal_state.init(
            this, &RandomState<RngEngine>::next_uint64, &RandomState<RngEngine>::next_uint32,
            &RandomState<RngEngine>::next_double, &RandomState<RngEngine>::next_raw);
    }

private:
    RngEngine _engine{};
    internal_random_state _internal_state{};

    /* for fast access */
    bool _has_integer = false;
    uint32_t _uinteger = 0;

    size_t _uintegers_cnt = 0;
    std::deque<uint64_t> _uintegers{};

    mutable std::mutex mutex{};
};

struct internal_numpy_seed_sequence {
    template <typename result_type, size_t pool_size>
    friend class NumpySeedSequence;

private:
    internal_numpy_seed_sequence(size_t);
    uint32_t generate();
    void set_entropy(std::vector<uint32_t>&&);
    void mix_entropy();

    std::vector<uint32_t> _pool;
    std::vector<uint32_t> _entropy;
    size_t _pool_idx = 0;
    uint32_t _last_hash_const = 0;
};

/*
NOTE: NumPy's SeedSequence is slightly different, NumPy's implementation resets it's hash after
every **n_words** generation which slightly feels wrong, so this implementation doesn't reset it. In
order to get the same behaviour as the NumPy's implementation don't reuse the same instance just
create a new instance after every generation. (Again this should be used to set the initial state of
a RngEngine so one time use should be enough.)
*/
template <typename result_type = unsigned int, size_t pool_size = 4>
class NumpySeedSequence {
    static_assert(std::is_same_v<result_type, uint32_t> || std::is_same_v<result_type, uint64_t>,
                  "**result_type** can only be uint32_t or uint64_t.");

public:
    NumpySeedSequence() {
        _inner.mix_entropy();
    }

    template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
    NumpySeedSequence(T int_entropy) {
        static_assert(sizeof(T) <= sizeof(uint64_t));
        std::vector<uint32_t> entropy;

        if constexpr (sizeof(T) <= sizeof(uint32_t)) {
            entropy = std::vector<uint32_t>(1, (uint32_t)int_entropy);
        }
        else {
            entropy = std::vector<uint32_t>(2, 0);
            entropy[0] = (uint32_t)((uint64_t)int_entropy >> 32);
            entropy[1] = (uint32_t)((uint64_t)int_entropy & 0xffffffff);
        }

        _inner.set_entropy(std::move(entropy));
        _inner.mix_entropy();
    }

    NumpySeedSequence(const std::vector<uint32_t>& entropy) {
        _inner.set_entropy(std::move(entropy));
        _inner.mix_entropy();
    }

    template <typename DestIter>
    void generate(DestIter start, DestIter finish) {
        typedef typename std::iterator_traits<DestIter>::value_type dest_t;

        constexpr uint8_t SRC_SIZE = sizeof(result_type);
        constexpr uint8_t DEST_SIZE = sizeof(dest_t);

        constexpr bool DEST_IS_SMALLER = sizeof(dest_t) < sizeof(result_type);
        constexpr uint8_t SCALE =
            DEST_IS_SMALLER ? SRC_SIZE / DEST_SIZE : (DEST_SIZE + SRC_SIZE - 1) / SRC_SIZE;

        size_t elems = (size_t)(finish - start);

        if constexpr (DEST_IS_SMALLER) {
            elems /= SCALE;
        }
        else {
            elems *= SCALE;
        }

        auto generated = generate(elems);
        uneven_copy(generated.begin(), start, finish,
                    std::integral_constant<bool, DEST_IS_SMALLER>{});

        // If converting from bigger ints.
        if constexpr (DEST_IS_SMALLER) {
            // For consistency across different endiannesses, view first as little - endian then
            // convert the values to the native endianness. This might be changed in the future.
            std::reverse(start, finish);
        }
    }

    static constexpr result_type(min)() {
        return std::numeric_limits<result_type>::min();
    }

    static constexpr result_type(max)() {
        return std::numeric_limits<result_type>::max();
    }

    result_type operator()() {
        return generate();
    }

private:
    result_type generate() {
        if constexpr (sizeof(result_type) <= sizeof(uint32_t)) {
            return (result_type)_inner.generate();
        }
        else {
            uint32_t state1 = _inner.generate();
            uint32_t state2 = _inner.generate();
            return (result_type)((uint64_t)state1 << 32 | state2);
        }
    }

    std::vector<result_type> generate(size_t n_words) {
        std::vector<result_type> state(n_words, 0);

        for (auto i = 0; i < n_words; i++) {
            state[i] = generate();
        }

        return state;
    }

private:
    internal_numpy_seed_sequence _inner{pool_size};
};