#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

struct MessageHeader {
    uint32_t type;
    uint32_t length;
};

struct QueueHeader {
    std::atomic<uint32_t> protocol_version{0};
    uint64_t              buffer_capacity{0};
    std::atomic<uint64_t> write_pos{0};
    std::atomic<uint64_t> read_pos{0};
};

constexpr uint32_t PROTOCOL_VERSION = 1;
constexpr const char* SHM_PATH      = "/mpsc_test_queue";
constexpr size_t      BUFFER_SIZE   = 1 << 20; // 1 MiB

class ProducerNode {
public:
    explicit ProducerNode(
        const std::string& path = SHM_PATH,
        size_t buffer_size = BUFFER_SIZE
    );
    ~ProducerNode();

    bool push(uint32_t type, const void* payload, size_t len);

private:
    void copy_to_ring(uint64_t abs_pos, const void* src, size_t sz);

    void*         shm_base_   = nullptr;
    size_t        total_size_ = 0;
    QueueHeader*  header_     = nullptr;
    char*         buffer_     = nullptr;
    uint64_t      capacity_   = 0;
    std::string   path_;
};

class ConsumerNode {
public:
    explicit ConsumerNode(
        const std::string& path = SHM_PATH,
        size_t buffer_size = BUFFER_SIZE
    );
    ~ConsumerNode();

    bool read_message(uint32_t expected_type, std::vector<uint8_t>& payload);

private:
    void copy_from_ring(uint64_t abs_pos, void* dest, size_t sz);

    void*         shm_base_   = nullptr;
    size_t        total_size_ = 0;
    QueueHeader*  header_     = nullptr;
    char*         buffer_     = nullptr;
    uint64_t      capacity_   = 0;
    std::string   path_;
};
