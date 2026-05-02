/*
 * Copyright 2017 - 2021 Justas Masiulis
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#ifndef JM_XORPTR_HPP
#define JM_XORPTR_HPP

#define JM_XORPTR_DISABLE_AVX_INTRINSICS

#if defined(INTERNAL_CHEAT)
#define JM_XORPTR_DISABLE_SSE_INTRINSICS
#endif

#if defined(_M_ARM64) || defined(__aarch64__) || defined(_M_ARM) || defined(__arm__)
#include <arm_neon.h>
#elif defined(_M_X64) || defined(__amd64__) || defined(_M_IX86) || defined(__i386__)
#include <immintrin.h>
#else
#error Unsupported platform
#endif

#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>

#if !defined(DEV_MODE)
#undef DEBUG_APPLICATION
#endif

#if !defined(DEBUG_APPLICATION)
    #define ANTI_REVERSE_POINTER_ENCRYPTION
#endif


#if defined(ANTI_REVERSE_POINTER_ENCRYPTION)
  #define xorptr(ptr) ::jm_xorptr::xor_ptr([]() { return ptr; }, std::integral_constant<std::size_t, sizeof(ptr)>{}, std::make_index_sequence<::jm_xorptr::detail::_buffer_size<sizeof(ptr)>()>{})
  #define xorptr_(ptr) xorptr(ptr).crypt_get()
  #define XOR32_IMPORTANT_ONLY(x) (*xorptr_(x))

  #if !defined(INTERNAL_CHEAT)
    #define XOR32(x) (*xorptr_(x))
    #define XOR64(x) (*xorptr_(x))
  #else
    #define XOR32(x) x
    #define XOR64(x) x
  #endif
#else
  #define xorptr(ptr) ptr
  #define xorptr_(ptr) ptr
  #define XOR32(x) x
  #define XOR64(x) x
  #define XOR32_IMPORTANT_ONLY(x) x
#endif

#ifdef _MSC_VER
#define XORPTR_FORCEINLINE __forceinline
#else
#define XORPTR_FORCEINLINE __attribute__((always_inline)) inline
#endif

#if defined(__clang__) || defined(__GNUC__)
#define JM_XORPTR_LOAD_FROM_REG(x) ::jm_xorptr::detail::load_from_reg(x)
#else
#define JM_XORPTR_LOAD_FROM_REG
#endif

namespace jm_xorptr {

    namespace detail {

        template<std::size_t Size>
        XORPTR_FORCEINLINE constexpr std::size_t _buffer_size()
        {
            return ((Size / 16) + (Size % 16 != 0)) * 2;
        }

        template<std::uint32_t Seed>
        XORPTR_FORCEINLINE constexpr std::uint32_t key4() noexcept
        {
            std::uint32_t value = Seed;
            for(char c : __TIME__)
                value = static_cast<std::uint32_t>((value ^ c) * 16777619ull);
            return value;
        }

        template<std::size_t S>
        XORPTR_FORCEINLINE constexpr std::uint64_t key8()
        {
            constexpr auto first_part  = key4<2166136261 + S>();
            constexpr auto second_part = key4<first_part>();
            return (static_cast<std::uint64_t>(first_part) << 32) | second_part;
        }

        // loads up to 8 characters of string into uint64 and xors it with the key
        template<std::size_t N, class CharT>
        XORPTR_FORCEINLINE constexpr std::uint64_t
        load_xored_str8(std::uint64_t key, std::size_t idx, const CharT str) noexcept
        {
            return key ^ str;
        }

        // forces compiler to use registers instead of stuffing constants in rdata
        XORPTR_FORCEINLINE std::uint64_t load_from_reg(std::uint64_t value) noexcept
        {
#if defined(__clang__) || defined(__GNUC__)
            asm("" : "=r"(value) : "0"(value) :);
#endif
            return value;
        }

    } // namespace detail

    template<class CharT, std::size_t Size, class Keys, class Indices>
    class xor_ptr;

    template<class CharT, std::size_t Size, std::uint64_t... Keys, std::size_t... Indices>
    class xor_ptr<CharT, Size, std::integer_sequence<std::uint64_t, Keys...>, std::index_sequence<Indices...>> {
#ifndef JM_XORPTR_DISABLE_AVX_INTRINSICS
        constexpr static inline std::uint64_t alignment = ((Size > 16) ? 32 : 16);
#else
        constexpr static inline std::uint64_t alignment = 16;
#endif

        alignas(alignment) std::uint64_t _storage[sizeof...(Keys)];

    public:
        using value_type    = CharT;
        using size_type     = std::size_t;
        using pointer       = CharT*;
        using const_pointer = const CharT*;

        template<class L>
        XORPTR_FORCEINLINE xor_ptr(L l, std::integral_constant<std::size_t, Size>, std::index_sequence<Indices...>) noexcept
            : _storage{ JM_XORPTR_LOAD_FROM_REG((std::integral_constant<std::uint64_t, detail::load_xored_str8<Size>(Keys, Indices, l())>::value))... }
        {}

        XORPTR_FORCEINLINE constexpr size_type size() const noexcept
        {
            return Size - 1;
        }

        XORPTR_FORCEINLINE void crypt() noexcept
        {
            // everything is inlined by hand because a certain compiler with a certain linker is _very_ slow
#if defined(__clang__)
            alignas(alignment)
                std::uint64_t arr[]{ JM_XORPTR_LOAD_FROM_REG(Keys)... };
            std::uint64_t*    keys =
                (std::uint64_t*)JM_XORPTR_LOAD_FROM_REG((std::uint64_t)arr);
#else
            alignas(alignment) std::uint64_t keys[]{ JM_XORPTR_LOAD_FROM_REG(Keys)... };
#endif

#if defined(_M_ARM64) || defined(__aarch64__) || defined(_M_ARM) || defined(__arm__)
#if defined(__clang__)
            ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : __builtin_neon_vst1q_v(
                                    reinterpret_cast<uint64_t*>(_storage) + Indices * 2,
                                    veorq_u64(__builtin_neon_vld1q_v(reinterpret_cast<const uint64_t*>(_storage) + Indices * 2, 51),
                                              __builtin_neon_vld1q_v(reinterpret_cast<const uint64_t*>(keys) + Indices * 2, 51)),
                                    51)), ...);
#else // GCC, MSVC
            ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : vst1q_u64(
                        reinterpret_cast<uint64_t*>(_storage) + Indices * 2,
                        veorq_u64(vld1q_u64(reinterpret_cast<const uint64_t*>(_storage) + Indices * 2),
                                  vld1q_u64(reinterpret_cast<const uint64_t*>(keys) + Indices * 2)))), ...);
#endif
#elif !defined(JM_XORPTR_DISABLE_AVX_INTRINSICS)
            ((Indices >= sizeof(_storage) / 32 ? static_cast<void>(0) : _mm256_store_si256(
                reinterpret_cast<__m256i*>(_storage) + Indices,
                _mm256_xor_si256(
                    _mm256_load_si256(reinterpret_cast<const __m256i*>(_storage) + Indices),
                    _mm256_load_si256(reinterpret_cast<const __m256i*>(keys) + Indices)))), ...);

            if constexpr(sizeof(_storage) % 32 != 0)
                _mm_store_si128(
                    reinterpret_cast<__m128i*>(_storage + sizeof...(Keys) - 2),
                    _mm_xor_si128(_mm_load_si128(reinterpret_cast<const __m128i*>(_storage + sizeof...(Keys) - 2)),
                                  _mm_load_si128(reinterpret_cast<const __m128i*>(keys + sizeof...(Keys) - 2))));
#else
#if !defined(JM_XORPTR_DISABLE_SSE_INTRINSICS)
        ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : _mm_store_si128(
            reinterpret_cast<__m128i*>(_storage) + Indices,
            _mm_xor_si128(_mm_load_si128(reinterpret_cast<const __m128i*>(_storage) + Indices),
                          _mm_load_si128(reinterpret_cast<const __m128i*>(keys) + Indices)))), ...);
#else
        ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : _storage[Indices] ^= keys[Indices]), ...);
#endif
#endif
        }

        XORPTR_FORCEINLINE const_pointer get() const noexcept
        {
            return reinterpret_cast<const_pointer>(_storage);
        }

        XORPTR_FORCEINLINE pointer get() noexcept
        {
            return reinterpret_cast<pointer>(_storage);
        }

        XORPTR_FORCEINLINE pointer crypt_get() noexcept
        {
        // crypt() is inlined manually to avoid linker slowdowns
    #if defined(__clang__)
        alignas(alignment)
            std::uint64_t arr[]{ JM_XORPTR_LOAD_FROM_REG(Keys)... };
        std::uint64_t* keys =
            (std::uint64_t*)JM_XORPTR_LOAD_FROM_REG((std::uint64_t)arr);
    #else
        alignas(alignment) std::uint64_t keys[]{ JM_XORPTR_LOAD_FROM_REG(Keys)... };
    #endif

    #if defined(_M_ARM64) || defined(__aarch64__) || defined(_M_ARM) || defined(__arm__)
    #if defined(__clang__)
        ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : __builtin_neon_vst1q_v(
                                    reinterpret_cast<uint64_t*>(_storage) + Indices * 2,
                                    veorq_u64(__builtin_neon_vld1q_v(reinterpret_cast<const uint64_t*>(_storage) + Indices * 2, 51),
                                              __builtin_neon_vld1q_v(reinterpret_cast<const uint64_t*>(keys) + Indices * 2, 51)),
                                    51)), ...);
    #else // GCC, MSVC
        ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : vst1q_u64(
                    reinterpret_cast<uint64_t*>(_storage) + Indices * 2,
                    veorq_u64(vld1q_u64(reinterpret_cast<const uint64_t*>(_storage) + Indices * 2),
                              vld1q_u64(reinterpret_cast<const uint64_t*>(keys) + Indices * 2)))), ...);
    #endif
    #elif !defined(JM_XORPTR_DISABLE_AVX_INTRINSICS)
        ((Indices >= sizeof(_storage) / 32 ? static_cast<void>(0) : _mm256_store_si256(
            reinterpret_cast<__m256i*>(_storage) + Indices,
            _mm256_xor_si256(
                _mm256_load_si256(reinterpret_cast<const __m256i*>(_storage) + Indices),
                _mm256_load_si256(reinterpret_cast<const __m256i*>(keys) + Indices)))), ...);

        if constexpr(sizeof(_storage) % 32 != 0)
            _mm_store_si128(
                reinterpret_cast<__m128i*>(_storage + sizeof...(Keys) - 2),
                _mm_xor_si128(_mm_load_si128(reinterpret_cast<const __m128i*>(_storage + sizeof...(Keys) - 2)),
                              _mm_load_si128(reinterpret_cast<const __m128i*>(keys + sizeof...(Keys) - 2))));
    #else
    #if !defined(JM_XORPTR_DISABLE_SSE_INTRINSICS)
        ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : _mm_store_si128(
            reinterpret_cast<__m128i*>(_storage) + Indices,
            _mm_xor_si128(_mm_load_si128(reinterpret_cast<const __m128i*>(_storage) + Indices),
                          _mm_load_si128(reinterpret_cast<const __m128i*>(keys) + Indices)))), ...);
    #else
        for (size_t i = 0; i < sizeof(_storage) / 16; i++) {
            constexpr size_t chunk_size = 16;
            uint8_t* dst = reinterpret_cast<uint8_t*>(_storage) + (i * chunk_size);
            const uint8_t* src1 = reinterpret_cast<const uint8_t*>(_storage) + (i * chunk_size);
            const uint8_t* src2 = reinterpret_cast<const uint8_t*>(keys) + (i * chunk_size);

            for (size_t j = 0; j < chunk_size; j++) {
                dst[j] = src1[j] ^ src2[j];  // Perform XOR byte-by-byte
            }
        }
    #endif
    #endif

        return (pointer)(_storage);
    }
    };

    template<class L, std::size_t Size, std::size_t... Indices>
    xor_ptr(L l, std::integral_constant<std::size_t, Size>, std::index_sequence<Indices...>) -> xor_ptr<
                std::remove_const_t<std::remove_reference_t<decltype(l())>>,
                Size,
                std::integer_sequence<std::uint64_t, detail::key8<Indices>()...>,
                std::index_sequence<Indices...>>;

} // namespace jm

#endif // include guard
