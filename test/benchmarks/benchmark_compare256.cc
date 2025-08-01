/* benchmark_compare256.cc -- benchmark compare256 variants
 * Copyright (C) 2022 Nathan Moinvaziri
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include <stdio.h>

#include <benchmark/benchmark.h>

extern "C" {
#  include "zbuild.h"
#  include "zutil_p.h"
#  include "arch_functions.h"
#  include "../test_cpu_features.h"
#  include "arch/generic/compare256_p.h"
}

#define MAX_COMPARE_SIZE (256)

class compare256: public benchmark::Fixture {
private:
    uint8_t *str1;
    uint8_t *str2;

public:
    void SetUp(const ::benchmark::State& state) {
        str1 = (uint8_t *)zng_alloc(MAX_COMPARE_SIZE);
        assert(str1 != NULL);
        memset(str1, 'a', MAX_COMPARE_SIZE);

        str2 = (uint8_t *)zng_alloc(MAX_COMPARE_SIZE);
        assert(str2 != NULL);
        memset(str2, 'a', MAX_COMPARE_SIZE);
    }

    void Bench(benchmark::State& state, compare256_func compare256) {
        int32_t match_len = (int32_t)state.range(0) - 1;
        uint32_t len = 0;

        str2[match_len] = 0;
        for (auto _ : state) {
            len = compare256((const uint8_t *)str1, (const uint8_t *)str2);
        }
        str2[match_len] = 'a';

        benchmark::DoNotOptimize(len);
    }

    void TearDown(const ::benchmark::State& state) {
        zng_free(str1);
        zng_free(str2);
    }
};

#define BENCHMARK_COMPARE256(name, fptr, support_flag) \
    BENCHMARK_DEFINE_F(compare256, name)(benchmark::State& state) { \
        if (!support_flag) { \
            state.SkipWithError("CPU does not support " #name); \
        } \
        Bench(state, fptr); \
    } \
    BENCHMARK_REGISTER_F(compare256, name)->Arg(1)->Arg(10)->Arg(40)->Arg(80)->Arg(100)->Arg(175)->Arg(256);

#ifdef DISABLE_RUNTIME_CPU_DETECTION
BENCHMARK_COMPARE256(native, native_compare256, 1);
#else

BENCHMARK_COMPARE256(8, compare256_8, 1);
BENCHMARK_COMPARE256(16, compare256_16, 1);
#if defined(HAVE_BUILTIN_CTZ)
BENCHMARK_COMPARE256(32, compare256_32, 1);
#endif
#if defined(HAVE_BUILTIN_CTZLL)
BENCHMARK_COMPARE256(64, compare256_64, 1);
#endif

#if defined(X86_SSE2) && defined(HAVE_BUILTIN_CTZ)
BENCHMARK_COMPARE256(sse2, compare256_sse2, test_cpu_features.x86.has_sse2);
#endif
#if defined(X86_AVX2) && defined(HAVE_BUILTIN_CTZ)
BENCHMARK_COMPARE256(avx2, compare256_avx2, test_cpu_features.x86.has_avx2);
#endif
#if defined(X86_AVX512) && defined(HAVE_BUILTIN_CTZLL)
BENCHMARK_COMPARE256(avx512, compare256_avx512, test_cpu_features.x86.has_avx512_common);
#endif
#if defined(ARM_NEON) && defined(HAVE_BUILTIN_CTZLL)
BENCHMARK_COMPARE256(neon, compare256_neon, test_cpu_features.arm.has_neon);
#endif
#ifdef POWER9
BENCHMARK_COMPARE256(power9, compare256_power9, test_cpu_features.power.has_arch_3_00);
#endif
#ifdef RISCV_RVV
BENCHMARK_COMPARE256(rvv, compare256_rvv, test_cpu_features.riscv.has_rvv);
#endif
#if defined(LOONGARCH_LSX) && defined(HAVE_BUILTIN_CTZ)
BENCHMARK_COMPARE256(lsx, compare256_lsx, test_cpu_features.loongarch.has_lsx);
#endif
#if defined(LOONGARCH_LASX) && defined(HAVE_BUILTIN_CTZ)
BENCHMARK_COMPARE256(lasx, compare256_lasx, test_cpu_features.loongarch.has_lasx);
#endif

#endif
