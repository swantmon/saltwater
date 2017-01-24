
#pragma once

namespace MATH
{
    template <typename T>
    struct SLimits
    {
    public:

        typedef T X;

    public:

        static const X s_Min;
        static const X s_Max;
    };
} // namespace MATH

namespace MATH
{
    template <>
    const typename SLimits<char>::X SLimits<char>::s_Min = X(CHAR_MIN);
    template <>
    const typename SLimits<char>::X SLimits<char>::s_Max = X(CHAR_MAX);

    template <>
    const typename SLimits<signed char>::X SLimits<signed char>::s_Min = X(SCHAR_MIN);
    template <>
    const typename SLimits<signed char>::X SLimits<signed char>::s_Max = X(SCHAR_MAX);

    template <>
    const typename SLimits<unsigned char>::X SLimits<unsigned char>::s_Min = X(0);
    template <>
    const typename SLimits<unsigned char>::X SLimits<unsigned char>::s_Max = X(UCHAR_MAX);

    template <>
    const typename SLimits<wchar_t>::X SLimits<wchar_t>::s_Min = X(WCHAR_MIN);
    template <>
    const typename SLimits<wchar_t>::X SLimits<wchar_t>::s_Max = X(WCHAR_MAX);

    template <>
    const typename SLimits<short>::X SLimits<short>::s_Min = X(SHRT_MIN);
    template <>
    const typename SLimits<short>::X SLimits<short>::s_Max = X(SHRT_MAX);

    template <>
    const typename SLimits<unsigned short>::X SLimits<unsigned short>::s_Min = X(0);
    template <>
    const typename SLimits<unsigned short>::X SLimits<unsigned short>::s_Max = X(USHRT_MAX);

    template <>
    const typename SLimits<int>::X SLimits<int>::s_Min = X(INT_MIN);
    template <>
    const typename SLimits<int>::X SLimits<int>::s_Max = X(INT_MAX);

    template <>
    const typename SLimits<unsigned int>::X SLimits<unsigned int>::s_Min = X(0);
    template <>
    const typename SLimits<unsigned int>::X SLimits<unsigned int>::s_Max = X(UINT_MAX);
    
    template <>
    const typename SLimits<long>::X SLimits<long>::s_Min = X(LONG_MIN);
    template <>
    const typename SLimits<long>::X SLimits<long>::s_Max = X(LONG_MAX);

    template <>
    const typename SLimits<unsigned long>::X SLimits<unsigned long>::s_Min = X(0);
    template <>
    const typename SLimits<unsigned long>::X SLimits<unsigned long>::s_Max = X(ULONG_MAX);

    template <>
    const typename SLimits<long long>::X SLimits<long long>::s_Min = X(-_LLONG_MAX - _C2);
    template <>
    const typename SLimits<long long>::X SLimits<long long>::s_Max = X(_LLONG_MAX);
    
    template <>
    const typename SLimits<unsigned long long>::X SLimits<unsigned long long>::s_Min = X(0);
    template <>
    const typename SLimits<unsigned long long>::X SLimits<unsigned long long>::s_Max = X(_ULLONG_MAX);
    
    template <>
    const typename SLimits<float>::X SLimits<float>::s_Min = X(FLT_MIN);
    template <>
    const typename SLimits<float>::X SLimits<float>::s_Max = X(FLT_MAX);
    
    template <>
    const typename SLimits<double>::X SLimits<double>::s_Min = X(DBL_MIN);
    template <>
    const typename SLimits<double>::X SLimits<double>::s_Max = X(DBL_MAX);
    
    template <>
    const typename SLimits<long double>::X SLimits<long double>::s_Min = X(LDBL_MIN);
    template <>
    const typename SLimits<long double>::X SLimits<long double>::s_Max = X(LDBL_MAX);
} // namespace MATH