#include <benchmark/benchmark.h>
#include <vector>
#include <functional>
#include <cmath>

#include "apply_function.hpp"

static void BM_Simple_SingleThread(benchmark::State& state) {
    const int num_threads = 1;
    std::function<void(int&)> transform = [](int& x) { x += 1; };

    for (auto _ : state) {
        std::vector<int> data(state.range(0), 0);
        ApplyFunction(data, transform, num_threads);
        benchmark::DoNotOptimize(data);
    }
}

static void BM_Simple_MultiThread(benchmark::State& state) {
    const int num_threads = 8;
    std::function<void(int&)> transform = [](int& x) { x += 1; };

    for (auto _ : state) {
        std::vector<int> data(state.range(0), 0);
        ApplyFunction(data, transform, num_threads);
        benchmark::DoNotOptimize(data);
    }
}

static void BM_Heavy_SingleThread(benchmark::State& state) {
    const int num_threads = 1;
    std::function<void(double&)> transform = [](double& x) {
        for (int i = 0; i < 500; ++i) {
            x = std::sin(x);
        }
    };

    for (auto _ : state) {
        std::vector<double> data(state.range(0), 1.0);
        ApplyFunction(data, transform, num_threads);
        benchmark::DoNotOptimize(data);
    }
}

static void BM_Heavy_MultiThread(benchmark::State& state) {
    const int num_threads = 4;
    std::function<void(double&)> transform = [](double& x) {
        for (int i = 0; i < 500; ++i) {
            x = std::sin(x);
        }
    };

    for (auto _ : state) {
        std::vector<double> data(state.range(0), 1.0);
        ApplyFunction(data, transform, num_threads);
        benchmark::DoNotOptimize(data);
    }
}

BENCHMARK(BM_Simple_SingleThread)
    ->RangeMultiplier(10)
    ->Range(100, 1'000'000);

BENCHMARK(BM_Simple_MultiThread)
    ->RangeMultiplier(10)
    ->Range(100, 1'000'000);

BENCHMARK(BM_Heavy_SingleThread)
    ->RangeMultiplier(10)
    ->Range(10'000, 10'000'000);

BENCHMARK(BM_Heavy_MultiThread)
    ->RangeMultiplier(10)
    ->Range(10'000, 10'000'000);
