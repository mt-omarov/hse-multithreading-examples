#pragma once

#include <atomic>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

static inline int FutexWait(
    void* addr,
    int expected
) {
    return syscall(
        SYS_futex,
        addr,
        FUTEX_WAIT_PRIVATE,
        expected,
        nullptr,
        nullptr,
        0
    );
}

static inline int FutexWake(
    void* addr,
    int count
) {
    return syscall(
        SYS_futex,
        addr,
        FUTEX_WAKE_PRIVATE,
        count,
        nullptr,
        nullptr,
        0
    );
}

class Mutex {
public:
    Mutex() = default;
    ~Mutex() = default;

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    void lock() {
        uint32_t expected = 0;

        if (
            state_.compare_exchange_strong(
                expected,
                1,
                std::memory_order_acquire
            )
        ) {
            return;
        }

        while (true) {
            if (expected != 2) {
                expected = state_.exchange(2, std::memory_order_acquire);
            }

            if (expected == 0) {
                return;
            }

            FutexWait(&state_, 2);

            expected = 0;
            if (
                state_.compare_exchange_strong(
                    expected, 2,
                    std::memory_order_acquire
                )
            ) {
                return;
            }
        }
    }

    void unlock() noexcept {
        if (state_.exchange(0, std::memory_order_release) == 2) {
            FutexWake(&state_, 1);
        }
    }

private:
    std::atomic<uint32_t> state_{0};
};
