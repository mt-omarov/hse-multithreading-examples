#include "mpsc_queue.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    ConsumerNode cons;

    std::cout << "Consumer started. Reading only messages of type 1...\n";

    std::vector<uint8_t> payload;
    for (int i = 0; i < 30; ++i) {
        if (cons.read_message(1, payload)) {
            std::string text(payload.begin(), payload.end());
            std::cout << "Received type 1: " << text << '\n';
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::cout << "Consumer finished.\n";
    return 0;
}
