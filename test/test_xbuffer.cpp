#include "gtest/gtest.h"
#include "memory/xbuffer.h"

using algernon::memory::XBuffer;

TEST(XBuffer, DefaultEmpty) {
    XBuffer<int> buf;
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0u);
    EXPECT_EQ(buf.data(), nullptr);
}

TEST(XBuffer, AllocateZeroInit) {
    XBuffer<int> buf(10);
    EXPECT_EQ(buf.size(), 10u);
    EXPECT_EQ(buf.sizeBytes(), 10 * sizeof(int));
    for (size_t i = 0; i < buf.size(); ++i) {
        EXPECT_EQ(buf[i], 0);
    }
}

TEST(XBuffer, FillAndAccess) {
    XBuffer<float> buf(5, 3.14f);
    EXPECT_EQ(buf.size(), 5u);
    for (auto& v : buf) EXPECT_FLOAT_EQ(v, 3.14f);
    buf[2] = 1.0f;
    EXPECT_FLOAT_EQ(buf[2], 1.0f);
}

TEST(XBuffer, MoveSemantics) {
    XBuffer<int> a(5, 42);
    XBuffer<int> b = std::move(a);
    EXPECT_TRUE(a.empty());
    EXPECT_EQ(b.size(), 5u);
    EXPECT_EQ(b[0], 42);
}

TEST(XBuffer, SharedCopy) {
    XBuffer<int> a(3, 7);
    XBuffer<int> b = a;  // shallow copy
    EXPECT_EQ(b.data(), a.data());
    b[0] = 99;
    EXPECT_EQ(a[0], 99);  // shared memory
}

TEST(XBuffer, Clear) {
    XBuffer<int> buf(10);
    buf.clear();
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.data(), nullptr);
}
