// thread_pool.h
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "my_future.hpp"

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop_(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex_);
                        condition_.wait(
                            lock,
                            [this] { return stop_ || !tasks_.empty(); }
                        );
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread &worker : workers_) {
            worker.join();
        }
    }

    template <typename Func, typename... Args>
    auto Submit(Func&& f, Args&&... args) -> MyFuture<typename std::invoke_result_t<Func, Args...>> {
        using ReturnType = typename std::invoke_result_t<Func, Args...>;

        auto state = std::make_shared<SharedState<ReturnType>>();
        auto bound = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);

        auto task = [state, bound]() mutable {
            try {
                if constexpr (std::is_void_v<ReturnType>) {
                    bound();
                    state->set_value();
                } else {
                    state->set_value(bound());
                }
            } catch (...) {
                state->set_exception(std::current_exception());
            }
        };

        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            tasks_.emplace(std::move(task));
        }
        condition_.notify_one();
        return MyFuture<ReturnType>(state);
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_;
};

#endif // THREAD_POOL_H
