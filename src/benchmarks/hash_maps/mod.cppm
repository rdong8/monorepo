module;

#include <absl/container/flat_hash_map.h>
#include <benchmark/benchmark.h>
#include <boost/unordered/unordered_flat_map.hpp>
#include <llvm/ADT/DenseMap.h>

export module benchmarks.hash_maps;

import std;

namespace
{

using Key = std::size_t;

struct Value
{
    using Self = Value;

    std::array<Key, 8> values{};

    Value() = default;

    explicit(false) Value(Key key)
    {
        std::ranges::fill(values, key);
    }

    explicit operator Key(this Self const &self)
    {
        return static_cast<Key>(self.values.front());
    }
};

// Type aliases avoid the comma-in-macro problem with BENCHMARK_TEMPLATE.
using AbslMapT = absl::flat_hash_map<Key, Value>;
using BoostMapT = boost::unordered_flat_map<Key, Value>;
using LlvmMapT = llvm::DenseMap<Key, Value>;

// Generate `n` random keys in [lo, hi). Ranges are kept far below the DenseMap sentinel values (~0 and ~0 - 1).
[[nodiscard]]
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto make_random_keys(std::size_t n, std::uint64_t seed, Key low, Key high) -> std::vector<Key>
{
    std::default_random_engine rng{seed};
    std::uniform_int_distribution<std::size_t> dist{low, high - 1};
    std::vector<std::size_t> keys(n);
    std::ranges::generate(keys, [&] { return dist(rng); });
    return keys;
}

// Disjoint key spaces so "miss" lookups never accidentally hit.
constexpr auto            //
    HIT_LOW = 1UZ,        //
    HIT_HIGH = 1UZ << 47, //
    MISS_LOW = 1UZ << 47, //
    MISS_HIGH = 1UZ << 48;

// Insertion (rehashes from empty)
template <typename Map> auto insert(benchmark::State &state) -> void
{
    auto const n = static_cast<std::uint32_t>(state.range(0));
    auto const keys = make_random_keys(n, 1, HIT_LOW, HIT_HIGH);

    for (auto const _ : state)
    {
        Map m{};

        for (auto const k : keys)
        {

            m[k] = k;
        }

        benchmark::DoNotOptimize(m);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(n));
}

// Insertion with reserve (no incremental rehashing)
template <typename Map> auto insert_reserve(benchmark::State &state) -> void
{
    auto const n = static_cast<std::uint32_t>(state.range(0));
    auto const keys = make_random_keys(n, 1, HIT_LOW, HIT_HIGH);

    for (auto const _ : state)
    {
        Map m{};
        m.reserve(n);

        for (auto const k : keys)
        {
            m[k] = k;
        }

        benchmark::DoNotOptimize(m);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(n));
}

// Successful lookup
template <typename Map> auto lookup_hit(benchmark::State &state) -> void
{
    auto const n = static_cast<std::uint32_t>(state.range(0));
    auto const keys = make_random_keys(n, 1, HIT_LOW, HIT_HIGH);
    Map m{};
    m.reserve(n);

    for (auto const k : keys)
    {
        m[k] = k;
    }

    Key i{0};

    for (auto const _ : state)
    {
        auto it = m.find(keys[i++ % n]);
        benchmark::DoNotOptimize(it);
    }

    state.SetItemsProcessed(state.iterations());
}

// Failed lookup
template <typename Map> auto lookup_miss(benchmark::State &state) -> void
{
    auto const n = static_cast<std::uint32_t>(state.range(0));
    auto const keys = make_random_keys(n, 1, HIT_LOW, HIT_HIGH);
    auto const miss_keys = make_random_keys(n, 2, MISS_LOW, MISS_HIGH);
    Map m{};
    m.reserve(n);

    for (auto const k : keys)
    {
        m[k] = k;
    }

    Key i{0};

    for (auto const _ : state)
    {
        auto it = m.find(miss_keys[i++ % n]);
        benchmark::DoNotOptimize(it);
    }

    state.SetItemsProcessed(state.iterations());
}

// Iteration
template <typename Map> auto iterate(benchmark::State &state) -> void
{
    auto const n = static_cast<std::uint32_t>(state.range(0));
    auto const keys = make_random_keys(n, 1, HIT_LOW, HIT_HIGH);
    Map m{};
    m.reserve(n);

    for (auto const k : keys)
    {
        m[k] = k;
    }

    for (auto const _ : state)
    {
        // TODO: doesn't work with DenseMap
        // auto sum{std::ranges::fold_left_first(m | std::views::values, std::plus<>{})};

        Key sum{0};

        for (auto const [k, v] : m)
        {
            sum += static_cast<Key>(v);
        }

        benchmark::DoNotOptimize(sum);
    }

    // m.size() may be < n if the generator produced duplicate keys.
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(m.size()));
}

// Insert + erase churn (erase-heavy workload)
template <typename Map> void churn(benchmark::State &state)
{
    auto const n = static_cast<std::uint32_t>(state.range(0));
    auto const keys = make_random_keys(n, 1, HIT_LOW, HIT_HIGH);

    for (auto const _ : state)
    {
        Map m{};
        m.reserve(n);

        for (auto const k : keys)
        {
            m[k] = k;
        }

        for (auto const k : keys)
        {
            m.erase(k);
        }

        benchmark::DoNotOptimize(m);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(n) * 2);
}

} // namespace

// Register every workload for all three map types over a range of sizes.
#define REGISTER_FOR_ALL(FUNC)                                                                                         \
    BENCHMARK_TEMPLATE(FUNC, AbslMapT)->Range(1 << 6, 1 << 22);                                                        \
    BENCHMARK_TEMPLATE(FUNC, BoostMapT)->Range(1 << 6, 1 << 22);                                                       \
    BENCHMARK_TEMPLATE(FUNC, LlvmMapT)->Range(1 << 6, 1 << 22)

REGISTER_FOR_ALL(insert);
REGISTER_FOR_ALL(insert_reserve);
REGISTER_FOR_ALL(lookup_hit);
REGISTER_FOR_ALL(lookup_miss);
REGISTER_FOR_ALL(iterate);
REGISTER_FOR_ALL(churn);
