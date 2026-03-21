#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <algorithm>

template <typename T>
void ApplyFunction(
    std::vector<T>& data,
    const std::function<void(T&)>& transform,
    const int threadCount = 1
) {
    if (data.empty()) {
        return;
    }
    const size_t n = data.size();
    int effective_threads = (threadCount < 1) ? 1 : threadCount;
    const size_t num_threads = std::min(
        static_cast<size_t>(effective_threads),
        n
    );

    if (num_threads == 1) {
        for (auto& elem : data) {
            transform(elem);
        }
        return;
    }

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    const size_t chunk_size = n / num_threads;
    const size_t remainder = n % num_threads;
    size_t start = 0;

    for (size_t i = 0; i < num_threads; ++i) {
        const size_t end = start + chunk_size + (i < remainder ? 1 : 0);
        threads.emplace_back([&data, &transform, start, end]() {
            for (size_t j = start; j < end; ++j) {
                transform(data[j]);
            }
        });
        start = end;
    }

    for (auto& t : threads) {
        t.join();
    }
}
