#include "thread_pool.hpp"
#include "process_pool.hpp"

#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "=== ThreadPool Demo ===\n";
    ThreadPool tp(4);

    auto f1 = tp.Submit([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 42;
    });

    auto f2 = tp.Submit([](int a, int b) { return a + b; }, 10, 20);

    auto f3 = tp.Submit([]() {
        std::cout << "Hello from thread pool!\n";
    });

    std::cout << "Result 1: " << f1.get() << std::endl;
    std::cout << "Result 2: " << f2.get() << std::endl;
    f3.get();

    std::cout << "\n=== ProcessPool Demo ===\n";
    ProcessPool pp(2);

    auto pf1 = pp.Submit(5);
    auto pf2 = pp.Submit(7);
    auto pf3 = pp.Submit(10);

    std::cout << "5^2 = " << pf1.get() << std::endl;
    std::cout << "7^2 = " << pf2.get() << std::endl;
    std::cout << "10^2 = " << pf3.get() << std::endl;

    return 0;
}
