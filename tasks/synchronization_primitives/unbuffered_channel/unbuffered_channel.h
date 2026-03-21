#pragma once

#include <optional>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

template <class T>
class UnbufferedChannel {
public:
    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (closed_) {
            throw std::runtime_error("send on closed channel");
        }

        cv_.wait(lock, [this]() {
            return !slot_.has_value() || closed_;
        });
        if (closed_) {
            throw std::runtime_error("send on closed channel");
        }

        slot_ = value;
        cv_.notify_one();

        cv_.wait(lock, [this]() {
            return !slot_.has_value() || closed_;
        });

        if (slot_.has_value()) {
            slot_ = std::nullopt;
            throw std::runtime_error("send on closed channel");
        }
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this]() {
            return slot_.has_value() || closed_;
        });

        if (slot_.has_value()) {
            T result = std::move(*slot_);
            slot_ = std::nullopt;
            cv_.notify_one();
            return result;
        }

        return std::nullopt;
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (closed_) {
            return;
        }
        closed_ = true;
        cv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::optional<T> slot_;
    bool closed_ = false;
};
