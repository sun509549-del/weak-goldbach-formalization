// Finite experiment, not an unbounded proof of weak Goldbach.
// Eratosthenes sieve is scalar. Only candidate primality lookups use AVX2.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#if (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
#define WG_HAS_AVX2_IMPL 1
#include <immintrin.h>
#else
#define WG_HAS_AVX2_IMPL 0
#endif

namespace {
constexpr uint32_t kMaxLimit = 10000000; // bounded memory and signed gather indices
using Clock = std::chrono::steady_clock;

struct Sieve {
    std::vector<uint32_t> prime;
    std::vector<uint32_t> primes;
    explicit Sieve(uint32_t limit) : prime(static_cast<size_t>(limit) + 1, 1) {
        prime[0] = 0;
        if (limit >= 1) prime[1] = 0;
        for (uint32_t p = 2; p <= limit / p; ++p) {
            if (!prime[p]) continue;
            for (uint32_t n = p * p; n <= limit; n += p) prime[n] = 0;
        }
        for (uint32_t p = 2; p <= limit; ++p) if (prime[p]) primes.push_back(p);
    }
};

bool avx2_available() {
#if WG_HAS_AVX2_IMPL
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx2");
#else
    return false;
#endif
}

// Return the smallest prime p with p <= even/2 and even-p prime, or zero.
uint32_t find_scalar(const Sieve& sieve, uint32_t even) {
    for (uint32_t p : sieve.primes) {
        if (p > even / 2) break;
        if (sieve.prime[even - p]) return p;
    }
    return 0;
}

#if WG_HAS_AVX2_IMPL
__attribute__((target("avx2")))
uint32_t find_avx2(const Sieve& sieve, uint32_t even) {
    const uint32_t half = even / 2;
    const __m256i target = _mm256_set1_epi32(static_cast<int>(even));
    const __m256i one = _mm256_set1_epi32(1);
    size_t i = 0;
    // Every lane is checked before its gather; no padded or negative indices.
    while (i + 8 <= sieve.primes.size() && sieve.primes[i + 7] <= half) {
        const __m256i p = _mm256_loadu_si256(
            reinterpret_cast<const __m256i*>(sieve.primes.data() + i));
        const __m256i q = _mm256_sub_epi32(target, p);
        const __m256i flags = _mm256_i32gather_epi32(
            reinterpret_cast<const int*>(sieve.prime.data()), q, 4);
        const unsigned mask = static_cast<unsigned>(_mm256_movemask_ps(
            _mm256_castsi256_ps(_mm256_cmpeq_epi32(flags, one))));
        if (mask) return sieve.primes[i + static_cast<size_t>(__builtin_ctz(mask))];
        i += 8;
    }
    for (; i < sieve.primes.size() && sieve.primes[i] <= half; ++i) {
        const uint32_t p = sieve.primes[i];
        if (sieve.prime[even - p]) return p;
    }
    return 0;
}
#endif

using Finder = uint32_t (*)(const Sieve&, uint32_t);

struct Result {
    std::vector<uint32_t> first_prime; // index i corresponds to n=7+2*i; 0 means absent
    uint64_t checked = 0;
    uint64_t found = 0;
    uint32_t first_failure = 0;
    uint64_t checksum = 14695981039346656037ULL;
    double milliseconds = 0;
};

Result verify(const Sieve& sieve, uint32_t limit, Finder find) {
    Result result;
    result.first_prime.reserve((limit - 7) / 2 + 1);
    const auto start = Clock::now();
    for (uint32_t n = 7; n <= limit; n += 2) {
        const uint32_t p = find(sieve, n - 3);
        result.first_prime.push_back(p);
        ++result.checked;
        if (p) ++result.found;
        else if (!result.first_failure) result.first_failure = n;
        // Explicit numeric checksum; identical across endian/word-size variants.
        for (uint32_t value : {n, 3U, p, p ? n - 3 - p : 0U}) {
            result.checksum ^= value;
            result.checksum *= 1099511628211ULL;
        }
    }
    result.milliseconds = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    return result;
}

bool trial_prime(uint32_t n) {
    if (n < 2) return false;
    for (uint32_t d = 2; d <= n / d; ++d) if (n % d == 0) return false;
    return true;
}

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void self_test() {
    const Sieve sieve(10001);
    for (uint32_t n = 0; n <= 10001; ++n) {
        require(static_cast<bool>(sieve.prime[n]) == trial_prime(n), "sieve/trial-division mismatch");
    }
    uint64_t witnesses = 0;
    for (uint32_t n = 7; n <= 10001; n += 2) {
        const uint32_t p = find_scalar(sieve, n - 3);
        require(p > 0, "small-range fixed-3 witness missing");
        const uint32_t q = n - 3 - p;
        require(trial_prime(3) && trial_prime(p) && trial_prime(q) && 3 + p + q == n,
                "independent witness check failed");
#if WG_HAS_AVX2_IMPL
        if (avx2_available()) require(p == find_avx2(sieve, n - 3), "AVX2/scalar mismatch");
#endif
        ++witnesses;
    }
    // Also exercise empty search, special even 4, vector tails, and limits parity.
    for (uint32_t even = 0; even <= 10000; even += 2) {
        const uint32_t scalar = find_scalar(sieve, even);
#if WG_HAS_AVX2_IMPL
        if (avx2_available()) require(scalar == find_avx2(sieve, even), "AVX2 tail mismatch");
#endif
        if (even < 4) require(scalar == 0, "unexpected empty-search witness");
    }
    for (uint32_t limit : {7U, 8U, 9U, 10U, 31U, 32U, 33U, 255U, 256U, 257U, 10001U}) {
        const Result result = verify(sieve, limit, find_scalar);
        require(result.checked == (limit - 7) / 2 + 1, "interval coverage mismatch");
        require(result.found == result.checked, "boundary witness missing");
    }
    std::cout << "self_test=PASS\nindependent_primality_checks=10002\n"
              << "independent_witness_checks=" << witnesses << "\n"
              << "avx2_available=" << (avx2_available() ? "true" : "false") << "\n"
              << "simd_comparison=" << (avx2_available() ? "PASS" : "SKIPPED_NO_AVX2") << "\n";
}

uint32_t parse_limit(const std::string& text) {
    if (text.empty() || text.size() > 8 ||
        !std::all_of(text.begin(), text.end(), [](char c) { return c >= '0' && c <= '9'; })) {
        throw std::invalid_argument("--limit must be a decimal integer from 7 to 10000000");
    }
    const unsigned long value = std::stoul(text);
    if (value < 7 || value > kMaxLimit) {
        throw std::invalid_argument("--limit must be between 7 and 10000000 inclusive");
    }
    return static_cast<uint32_t>(value);
}

void write_csv(const std::string& path, const Result& result) {
    std::ofstream output(path);
    if (!output) throw std::runtime_error("cannot open CSV output: " + path);
    output << "n,a,b,c\n";
    for (size_t i = 0; i < result.first_prime.size(); ++i) {
        const uint32_t n = 7 + 2 * static_cast<uint32_t>(i);
        const uint32_t p = result.first_prime[i];
        if (p) output << n << ",3," << p << ',' << n - 3 - p << '\n';
    }
    output.close();
    if (!output) throw std::runtime_error("failed while writing CSV: " + path);
}

void help() {
    std::cout << "Usage: verify [--limit N] [--mode auto|scalar|simd] [--cross-check] [--csv FILE]\n"
              << "       verify --self-test\n"
              << "Default N=1000000; allowed 7..10000000 (inclusive upper bound).\n"
              << "Searches every odd n in [7,N] for n=3+p+q with p,q prime.\n"
              << "A failure of this restricted search is not a weak-Goldbach counterexample.\n"
              << "--cross-check compares every witness against the other implementation.\n"
              << "CSV contains successful witnesses only. Exit codes: 0 PASS; 1 error;\n"
              << "2 restricted-search failure (see first_failure). No unbounded proof.\n";
}
} // namespace

int main(int argc, char** argv) {
    try {
        uint32_t limit = 1000000;
        std::string mode = "auto";
        std::string csv;
        bool cross_check = false;
        bool run_self_test = false;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--help") { help(); return 0; }
            if (arg == "--cross-check") { cross_check = true; continue; }
            if (arg == "--self-test") { run_self_test = true; continue; }
            if (arg == "--limit" || arg == "--mode" || arg == "--csv") {
                if (i + 1 >= argc) throw std::invalid_argument("missing value for " + arg);
                const std::string value = argv[++i];
                if (arg == "--limit") limit = parse_limit(value);
                else if (arg == "--mode") mode = value;
                else { if (value.empty()) throw std::invalid_argument("empty CSV path"); csv = value; }
                continue;
            }
            throw std::invalid_argument("unknown argument: " + arg);
        }
        if (mode != "auto" && mode != "scalar" && mode != "simd") {
            throw std::invalid_argument("--mode must be auto, scalar, or simd");
        }
        if (run_self_test) { self_test(); return 0; }
        const bool available = avx2_available();
        if (mode == "simd" && !available) throw std::runtime_error("AVX2 is not available on this CPU/build");
        Finder finder = find_scalar;
        std::string selected = "scalar";
#if WG_HAS_AVX2_IMPL
        if (available && mode != "scalar") { finder = find_avx2; selected = "avx2"; }
#endif
        const auto start = Clock::now();
        const Sieve sieve(limit);
        const double sieve_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
        const Result result = verify(sieve, limit, finder);
        std::cout << std::fixed << std::setprecision(3)
                  << "scope=finite_fixed_3_prime_witness_search\n"
                  << "lower=7\nupper=" << limit << "\nmode=" << selected << "\n"
                  << "avx2_available=" << (available ? "true" : "false") << "\n"
                  << "primes_up_to_limit=" << sieve.primes.size() << "\n"
                  << "checked_odds=" << result.checked << "\nfound_witnesses=" << result.found << "\n"
                  << "first_failure=" << result.first_failure << "\n"
                  << "witness_checksum=" << result.checksum << "\n"
                  << "scalar_sieve_ms=" << sieve_ms << "\nsearch_ms=" << result.milliseconds << "\n";
        if (cross_check) {
#if WG_HAS_AVX2_IMPL
            if (available) {
                const Result other = verify(sieve, limit, selected == "avx2" ? find_scalar : find_avx2);
                require(result.first_prime == other.first_prime && result.checksum == other.checksum,
                        "cross-check found different witnesses");
                std::cout << "cross_check=PASS\nother_search_ms=" << other.milliseconds << "\n";
            } else
#endif
            { std::cout << "cross_check=SKIPPED_NO_AVX2\n"; }
        }
        if (!csv.empty()) { write_csv(csv, result); std::cout << "csv=" << csv << "\n"; }
        std::cout << "result=" << (result.found == result.checked ? "PASS" : "RESTRICTED_SEARCH_FAILURE") << "\n";
        return result.found == result.checked ? 0 : 2;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
