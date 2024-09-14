#ifndef __XBUFFER_H__
#define __XBUFFER_H__


#include <memory>

namespace memory
{
    
    template <typename T>
    class XBuffer {
    public:
        XBuffer();
        XBuffer(size_t n); // n denotes the number of elements
        ~XBuffer();

        XBuffer(const XBuffer& buffer);
        XBuffer& operator=(const XBuffer& buffer);

        T& operator[](size_t i);
        const T& operator[](size_t i) const;

        size_t sizeByByte() const;
        size_t sizeOfElements() const;

        T* get() const;

        void clear();
    
    private:
        std::shared_ptr<T>  mData;
        size_t              mSize; // number of elements
        T*                  mAddr;
    };

} // namespace memory

#include "xbuffer.impl.h"

#endif // __XBUFFER_H__