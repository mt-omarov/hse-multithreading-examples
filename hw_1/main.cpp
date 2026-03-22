#include <iostream>
#include <vector>
#include <stack>
#include <coroutine>
#include <utility>

struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        std::suspend_always initial_suspend() {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void return_void() {}

        void unhandled_exception() {
            std::terminate();
        }
    };

    std::coroutine_handle<promise_type> handle;

    explicit Task(std::coroutine_handle<promise_type> h) : handle(h) {}

    ~Task() {
        if (handle) {
            handle.destroy();
        }
    }

    Task(Task&& other) noexcept : handle(std::exchange(other.handle, nullptr)) {}

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle) {
                handle.destroy();
            }
            handle = std::exchange(other.handle, nullptr);
        }
        return *this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    bool resume() {
        if (!handle || handle.done()) {
            return false;
        }
        handle.resume();
        return !handle.done();
    }

    bool done() const { return !handle || handle.done(); }
};

Task dfs_cooperative(
    const std::vector<std::vector<int>>& graph,
    int start,
    int id
) {
    std::vector<bool> visited(graph.size(), false);
    std::stack<int> st;
    st.push(start);

    while (!st.empty()) {
        int v = st.top();
        st.pop();
        if (visited[v]) continue;
        visited[v] = true;

        std::cout << "Coroutine " << id
                  << " visiting vertex " << v << std::endl;

        for (auto it = graph[v].rbegin(); it != graph[v].rend(); ++it) {
            if (!visited[*it]) {
                st.push(*it);
            }
        }

        co_await std::suspend_always{};
    }

    std::cout << "Coroutine " << id << " finished." << std::endl;
}

int main() {
    // Компонента 1: вершины 0,1,2,3
    // Компонента 2: вершины 4,5,6
    std::vector<std::vector<int>> graph = {
        {1, 3},     // 0 связана с 1 и 3
        {0, 2},     // 1 связана с 0 и 2
        {1, 3},     // 2 связана с 1 и 3
        {0, 2},     // 3 связана с 0 и 2
        {5},        // 4 связана с 5
        {4, 6},     // 5 связана с 4 и 6
        {5}         // 6 связана с 5
    };

    std::vector<Task> tasks;
    tasks.emplace_back(dfs_cooperative(graph, 0, 1));
    tasks.emplace_back(dfs_cooperative(graph, 4, 2));
    tasks.emplace_back(dfs_cooperative(graph, 6, 3));

    bool anyAlive = true;
    while (anyAlive) {
        anyAlive = false;
        for (auto& task : tasks) {
            if (!task.done()) {
                anyAlive = true;
                task.resume();
            }
        }
    }

    return 0;
}
