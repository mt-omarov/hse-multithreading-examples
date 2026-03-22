#include "mutex.hpp"

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iomanip>


int main() {
const int num_threads = std::thread::hardware_concurrency() * 2;
    const long long ops_per_thread = 1'000'000LL;

    std::cout << "Starting mutex correctness tests...\n";
    std::cout << "Threads: " << num_threads
              << " | Operations per thread: " << ops_per_thread << "\n\n";

    { // simple counter test
        auto start = std::chrono::high_resolution_clock::now();

        Mutex mtx;
        std::atomic<long long> counter{0};

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&mtx, &counter]() {
                for (long long j = 0; j < ops_per_thread; ++j) {
                    mtx.lock();
                    counter.fetch_add(1, std::memory_order_relaxed);
                    mtx.unlock();
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(
            end - start
        ).count();

        assert(counter == num_threads * ops_per_thread);

        std::cout << "[PASSED] Test 1 (simple counter) — result = "
                  << counter
                  << " (expected " << num_threads * ops_per_thread
                  << ")\n";

        std::cout << "\t Time: "
                  << std::fixed << std::setprecision(1) << ms << " ms\n\n";
    }
}
