#ifndef MY_FUTURE_H
#define MY_FUTURE_H

#include <mutex>
#include <condition_variable>
#include <optional>
#include <exception>
#include <memory>

template <typename T>
class SharedState {
public:
    void set_value(T value) {
        std::lock_guard<std::mutex> lock(mtx_);
        value_ = std::move(value);
        ready_ = true;
        cv_.notify_all();
    }

    void set_exception(std::exception_ptr e) {
        std::lock_guard<std::mutex> lock(mtx_);
        except_ = e;
        ready_ = true;
        cv_.notify_all();
    }

    T get() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return ready_; });
        if (except_) {
            std::rethrow_exception(except_);
        }
        return std::move(*value_);
    }

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    bool ready_ = false;
    std::optional<T> value_;
    std::exception_ptr except_;
};

// for void
template <>
class SharedState<void> {
public:
    void set_value() {
        std::lock_guard<std::mutex> lock(mtx_);
        ready_ = true;
        cv_.notify_all();
    }

    void set_exception(std::exception_ptr e) {
        std::lock_guard<std::mutex> lock(mtx_);
        except_ = e;
        ready_ = true;
        cv_.notify_all();
    }

    void get() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return ready_; });
        if (except_) {
            std::rethrow_exception(except_);
        }
    }

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    bool ready_ = false;
    std::exception_ptr except_;
};

template <typename T>
class MyFuture {
public:
    MyFuture(std::shared_ptr<SharedState<T>> state) : state_(std::move(state)) {}

    T get() {
        return state_->get();
    }

private:
    std::shared_ptr<SharedState<T>> state_;
};

// for void
template <>
class MyFuture<void> {
public:
    MyFuture(std::shared_ptr<SharedState<void>> state) : state_(std::move(state)) {}

    void get() {
        state_->get();
    }

private:
    std::shared_ptr<SharedState<void>> state_;
};

#endif // MY_FUTURE_H
