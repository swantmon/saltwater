
#pragma once

#include "base/base_defines.h"

namespace MATH
{
    template <typename T>
    struct SConstants
    {
    public:

        typedef T X;

    public:

        static const X s_Pi;
        static const X s_HalfPi;
        static const X s_TwoPi;
        static const X s_ReciprocalTwoPi;
        static const X s_SquareRootOfTwo;
        static const X s_Epsilon;
    };
} // namespace MATH

namespace MATH
{
    template <typename T>
    const typename SConstants<T>::X SConstants<T>::s_Pi = X(3.1415926535897932384626433832795);
    template <typename T>
    const typename SConstants<T>::X SConstants<T>::s_HalfPi = X(1.5707963267948966192313216916395);
    template <typename T>
    const typename SConstants<T>::X SConstants<T>::s_TwoPi = X(6.2831853071795864769252867665590);
    template <typename T>
    const typename SConstants<T>::X SConstants<T>::s_ReciprocalTwoPi = X(0.1591549430918953357688837633725);
    template <typename T>
    const typename SConstants<T>::X SConstants<T>::s_SquareRootOfTwo = X(1.4142135623730950488016887242097);
    template <typename T>
    const typename SConstants<T>::X SConstants<T>::s_Epsilon = X(0.000001);
} // namespace MATH