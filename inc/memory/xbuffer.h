#ifndef AURA_MEMORY_XBUFFER_H_
#define AURA_MEMORY_XBUFFER_H_

/**
 * @file xbuffer.h
 * @brief Managed buffer with shared ownership, move semantics, and iterators.
 *
 * @example
 *   au::memory::XBuffer<float> buf(1024);
 *   buf[0] = 3.14f;
 *   buf.fill(0.0f);
 *
 *   for (auto& v : buf) { v *= 2.0f; }
 *
 *   // Shared (shallow) copy:
 *   auto buf2 = buf;  // buf2 shares the same memory
 *
 *   // Move:
 *   auto buf3 = std::move(buf);  // buf is now empty
 */

#include <memory>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <cassert>

namespace au { namespace memory {

template <typename T>
class XBuffer {
public:
    XBuffer() = default;

    /** @brief Allocate buffer of count elements (zero-initialized). */
    explicit XBuffer(size_t count)
        : mData(new T[count](), std::default_delete<T[]>())
        , mSize(count) {}

    /** @brief Allocate buffer and fill with value. */
    XBuffer(size_t count, const T& value)
        : mData(new T[count], std::default_delete<T[]>())
        , mSize(count) {
        std::fill(data(), data() + mSize, value);
    }

    ~XBuffer() = default;

    // Shallow copy (shared ownership)
    XBuffer(const XBuffer&) = default;
    XBuffer& operator=(const XBuffer&) = default;

    // Move
    XBuffer(XBuffer&& other) noexcept
        : mData(std::move(other.mData)), mSize(other.mSize) {
        other.mSize = 0;
    }

    XBuffer& operator=(XBuffer&& other) noexcept {
        if (this != &other) {
            mData = std::move(other.mData);
            mSize = other.mSize;
            other.mSize = 0;
        }
        return *this;
    }

    // -- Access --

    T* data() { return mData.get(); }
    const T* data() const { return mData.get(); }

    T& operator[](size_t i) { assert(i < mSize); return mData.get()[i]; }
    const T& operator[](size_t i) const { assert(i < mSize); return mData.get()[i]; }

    // -- Size --

    size_t size() const { return mSize; }
    size_t sizeBytes() const { return mSize * sizeof(T); }
    bool empty() const { return mSize == 0; }

    // -- Iterators --

    T* begin() { return data(); }
    T* end() { return data() + mSize; }
    const T* begin() const { return data(); }
    const T* end() const { return data() + mSize; }

    // -- Mutation --

    void fill(const T& value) { std::fill(begin(), end(), value); }

    /**
     * @brief Resize the buffer (allocates new memory, copies old data).
     *        New elements are zero-initialized.
     */
    void resize(size_t newCount) {
        if (newCount == mSize) return;
        std::shared_ptr<T> newData(new T[newCount](), std::default_delete<T[]>());
        if (mData && mSize > 0) {
            std::copy(data(), data() + std::min(mSize, newCount), newData.get());
        }
        mData = newData;
        mSize = newCount;
    }

    void clear() {
        mData.reset();
        mSize = 0;
    }

private:
    std::shared_ptr<T> mData;
    size_t mSize = 0;
};

}}  // namespace au::memory

#endif // AURA_MEMORY_XBUFFER_H_
