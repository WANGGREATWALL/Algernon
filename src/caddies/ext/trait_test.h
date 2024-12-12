#ifndef __TRAIT_TEST_H__
#define __TRAIT_TEST_H__

#include <type_traits>

template <class T, T v>
struct ConstantBase {
    static const T value = v;
    typedef T value_type;
    typedef ConstantBase<T, v> type;
    operator value_type() { return value; }
};

#endif // __TRAIT_TEST_H__