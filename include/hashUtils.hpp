#pragma once
#include <functional>

namespace wseml {
    namespace hash {

        inline size_t hash_mix(size_t v) {
            static_assert(sizeof(size_t) == 8);
            size_t const m = 0xe9846af9b1a6180ULL;

            v ^= v >> 32;
            v *= m;
            v ^= v >> 32;
            v *= m;
            v ^= v >> 28;

            return v;
        }

        template <class T>
        inline void hash_combine(size_t& seed, T const& v) {
            static_assert(sizeof(size_t) == 8);
            std::hash<T> hasher;
            seed = hash_mix(seed + 0x9e3779b97f4a7c15ULL + hasher(v));
        }

        template <class It>
        inline void hash_range(size_t& seed, It first, It last) {
            for (; first != last; ++first) {
                hash_combine(seed, *first);
            }
        }

        template <class It>
        inline size_t hash_range(It first, It last) {
            size_t seed = 0;
            hash_range(seed, first, last);
            return seed;
        }

        template <class It>
        inline void hash_unordered_range(size_t& seed, It first, It last) {
            size_t accumulation = 0;
            size_t const original_seed = seed;

            for (; first != last; ++first) {
                size_t element_specific_seed = original_seed;
                hash_combine(element_specific_seed, *first);
                accumulation += element_specific_seed;
            }
            seed += accumulation;
        }

        template <class It>
        inline size_t hash_unordered_range(It first, It last) {
            size_t seed = 0;
            hash_unordered_range(seed, first, last);
            return seed;
        }

    } // namespace hash
} // namespace wseml
