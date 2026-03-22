#include "mpsc_queue.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    ProducerNode prod;

    std::cout << "Producer started. "
              << "Sending 20 messages (types 1 and 2 alternating)...\n";

    for (int i = 0; i < 20; ++i) {
        uint32_t type = (i % 2 == 0) ? 1 : 2;
        std::string msg = "Message #" + std::to_string(i) + " from producer";

        if (prod.push(type, msg.data(), msg.size())) {
            std::cout << "Sent type " << type << ": " << msg << '\n';
        } else {
            std::cout << "Queue full\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "Producer finished.\n";
    return 0;
}
