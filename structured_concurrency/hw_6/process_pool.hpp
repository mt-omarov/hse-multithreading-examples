#ifndef PROCESS_POOL_H
#define PROCESS_POOL_H

#include "my_future.hpp"

#include <vector>
#include <memory>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdexcept>

class ProcessPool {
public:
    ProcessPool(size_t numProcesses) {
        for (size_t i = 0; i < numProcesses; ++i) {
            int toChild[2];   // parent -> child
            int fromChild[2]; // child -> parent

            if (pipe(toChild) == -1 || pipe(fromChild) == -1)
                throw std::runtime_error("pipe creation failed");

            pid_t pid = fork();
            if (pid == -1)
                throw std::runtime_error("fork failed");

            if (pid == 0) { // child
                close(toChild[1]);
                close(fromChild[0]);

                int readFd = toChild[0];
                int writeFd = fromChild[1];

                while (true) {
                    int arg;
                    ssize_t n = read(readFd, &arg, sizeof(arg));
                    if (n <= 0) break;
                    int result = arg * arg; // some operation
                    if (write(writeFd, &result, sizeof(result)) != sizeof(result))
                        break;
                }
                close(readFd);
                close(writeFd);
                exit(0);
            } else { // parent
                close(toChild[0]);
                close(fromChild[1]);

                ProcessInfo info;
                info.pid = pid;
                info.writeFd = toChild[1];
                info.readFd = fromChild[0];
                processes_.push_back(info);
            }
        }
    }

    ~ProcessPool() {
        for (auto& p : processes_) {
            close(p.writeFd);
        }

        for (auto& p : processes_) {
            int status;
            waitpid(p.pid, &status, 0);
            close(p.readFd);
        }
    }

    MyFuture<int> Submit(int arg) {
        auto state = std::make_shared<SharedState<int>>();

        size_t idx = nextProc_ % processes_.size();
        nextProc_++;
        auto& proc = processes_[idx];

        if (write(proc.writeFd, &arg, sizeof(arg)) != sizeof(arg)) {
            state->set_exception(std::make_exception_ptr(std::runtime_error("write failed")));
            return MyFuture<int>(state);
        }

        std::thread reader([state, readFd = proc.readFd]() {
            int result;
            ssize_t n = read(readFd, &result, sizeof(result));
            if (n == sizeof(result))
                state->set_value(result);
            else
                state->set_exception(std::make_exception_ptr(std::runtime_error("read failed")));
        });
        reader.detach();

        return MyFuture<int>(state);
    }

private:
    struct ProcessInfo {
        pid_t pid;
        int writeFd;
        int readFd;
    };
    std::vector<ProcessInfo> processes_;
    size_t nextProc_ = 0;
};

#endif // PROCESS_POOL_H
