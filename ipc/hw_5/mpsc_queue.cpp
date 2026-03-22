#include "mpsc_queue.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

ProducerNode::ProducerNode(
    const std::string& path,
    size_t buffer_size
)
    : path_(path)
    , capacity_(buffer_size)
{
    int fd = shm_open(path_.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        std::cerr << "shm_open failed\n";
        exit(1);
    }

    total_size_ = sizeof(QueueHeader) + buffer_size;
    if (ftruncate(fd, total_size_) == -1) {
        std::cerr << "ftruncate failed\n";
        exit(1);
    }

    shm_base_ = mmap(
        nullptr,
        total_size_,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    close(fd);

    if (shm_base_ == MAP_FAILED) {
        std::cerr << "mmap failed\n";
        exit(1);
    }

    header_ = static_cast<QueueHeader*>(shm_base_);
    buffer_ = static_cast<char*>(shm_base_) + sizeof(QueueHeader);

    uint32_t expected = 0;
    if (
        header_->protocol_version.compare_exchange_strong(
            expected,
            PROTOCOL_VERSION
        )
    ) {
        header_->buffer_capacity = buffer_size;
        header_->write_pos.store(0, std::memory_order_seq_cst);
        header_->read_pos.store(0, std::memory_order_seq_cst);
    } else if (
        header_->protocol_version.load(
            std::memory_order_seq_cst
        ) != PROTOCOL_VERSION
    ) {
        std::cerr << "Protocol version mismatch\n";
        exit(1);
    }
}

ProducerNode::~ProducerNode() {
    munmap(shm_base_, total_size_);
}

bool ProducerNode::push(
    uint32_t type,
    const void* payload,
    size_t len
) {
    size_t total = sizeof(MessageHeader) + len;

    auto& wpos = header_->write_pos;
    auto& rpos = header_->read_pos;

    uint64_t pos;
    while (true) {
        pos = wpos.load(std::memory_order_seq_cst);
        uint64_t r = rpos.load(std::memory_order_seq_cst);

        if (pos + total > r + capacity_) {
            return false;
        }

        if (
            wpos.compare_exchange_weak(
                pos,
                pos + total,
                std::memory_order_seq_cst
            )
        ) {
            break;
        }
    }

    MessageHeader h{type, static_cast<uint32_t>(len)};
    copy_to_ring(pos, &h, sizeof(h));
    copy_to_ring(pos + sizeof(MessageHeader), payload, len);

    return true;
}

void ProducerNode::copy_to_ring(
    uint64_t abs_pos,
    const void* src,
    size_t sz
) {
    if (sz == 0) {
        return;
    }

    uint64_t start = abs_pos % capacity_;
    size_t to_end = capacity_ - start;

    if (sz <= to_end) {
        std::memcpy(buffer_ + start, src, sz);
    } else {
        std::memcpy(buffer_ + start, src, to_end);
        std::memcpy(
            buffer_,
            static_cast<const char*>(src) + to_end,
            sz - to_end
        );
    }
}

ConsumerNode::ConsumerNode(
    const std::string& path,
    size_t buffer_size
)
    : path_(path)
    , capacity_(buffer_size)
{
    int fd = shm_open(path_.c_str(), O_RDWR, 0666);
    if (fd == -1) {
        std::cerr << "shm_open failed (queue not created)\n";
        exit(1);
    }

    total_size_ = sizeof(QueueHeader) + buffer_size;
    shm_base_ = mmap(
        nullptr,
        total_size_,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    close(fd);

    if (shm_base_ == MAP_FAILED) {
        std::cerr << "mmap failed\n";
        exit(1);
    }

    header_ = static_cast<QueueHeader*>(shm_base_);
    buffer_ = static_cast<char*>(shm_base_) + sizeof(QueueHeader);

    if (
        header_->protocol_version.load(
            std::memory_order_seq_cst
        ) != PROTOCOL_VERSION ||
        header_->buffer_capacity != buffer_size
    ) {
        std::cerr << "Protocol version or buffer size mismatch\n";
        exit(1);
    }
}

ConsumerNode::~ConsumerNode() {
    munmap(shm_base_, total_size_);
}

bool ConsumerNode::read_message(
    uint32_t expected_type,
    std::vector<uint8_t>& payload
) {
    auto& rpos = header_->read_pos;
    auto& wpos = header_->write_pos;

    while (true) {
        uint64_t r = rpos.load(std::memory_order_seq_cst);
        uint64_t w = wpos.load(std::memory_order_seq_cst);
        if (r == w) return false;

        MessageHeader h;
        copy_from_ring(r, &h, sizeof(h));
        size_t total = sizeof(MessageHeader) + h.length;

        if (h.type == expected_type) {
            payload.resize(h.length);
            copy_from_ring(r + sizeof(MessageHeader), payload.data(), h.length);
            rpos.store(r + total, std::memory_order_seq_cst);
            return true;
        } else {
            rpos.store(r + total, std::memory_order_seq_cst);
        }
    }
}

void ConsumerNode::copy_from_ring(
    uint64_t abs_pos,
    void* dest,
    size_t sz
) {
    if (sz == 0) {
        return;
    }

    uint64_t start = abs_pos % capacity_;
    size_t to_end = capacity_ - start;

    if (sz <= to_end) {
        std::memcpy(dest, buffer_ + start, sz);
    } else {
        std::memcpy(dest, buffer_ + start, to_end);
        std::memcpy(
            static_cast<char*>(dest) + to_end,
            buffer_,
            sz - to_end
        );
    }
}
