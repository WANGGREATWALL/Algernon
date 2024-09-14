#ifndef __XBUFFER_IMPL_H__
#define __XBUFFER_IMPL_H__

#include "xbuffer.h"
#include "logger.h"

namespace memory {

    template <typename T>
    std::shared_ptr<T> makeSharedArray(size_t n)
    {
        // default_delete is the default deleter of stl
        return std::shared_ptr<T>(new T[n](), std::default_delete<T[]>());
    }

    template <typename T>
    XBuffer<T>::XBuffer()
        : mSize(0), mAddr(nullptr)
    {
        ;
    }

    template <typename T>
    XBuffer<T>::XBuffer(size_t n)
        : mSize(0), mAddr(nullptr)
    {
        mData = makeSharedArray<T>(n);
        ASSERTER(mData);
        mSize = n;
        mAddr = mData.get();
    }

    template <typename T>
    XBuffer<T>::~XBuffer()
    {
        clear();
    }

    template <typename T>
    XBuffer<T>::XBuffer(const XBuffer& buffer)
    {
        mData = buffer.mData;
        mSize = buffer.mSize;
        mAddr = buffer.mAddr;
    }

    template <typename T>
    XBuffer<T>& XBuffer<T>::operator=(const XBuffer& buffer)
    {
        mData = buffer.mData;
        mSize = buffer.mSize;
        mAddr = buffer.mAddr;

        return *this;
    }

    template <typename T>
    size_t XBuffer<T>::sizeByByte() const
    {
        return mSize * sizeof(T);
    }

    template <typename T>
    size_t XBuffer<T>::sizeOfElements() const
    {
        return mSize;
    }

    template <typename T>
    T& XBuffer<T>::operator[](size_t i)
    {
        ASSERTER(i < mSize);
        return mAddr[i];
    }

    template <typename T>
    const T& XBuffer<T>::operator[](size_t i) const
    {
        ASSERTER(i < mSize);
        return mAddr[i];
    }

    template <typename T>
    T* XBuffer<T>::get() const
    {
        return mAddr;
    }

    template <typename T>
    void XBuffer<T>::clear()
    {
        mData.reset();
        mSize = 0;
        mAddr = nullptr;
    }

} // namespace memory

#endif // __XBUFFER_IMPL_H__