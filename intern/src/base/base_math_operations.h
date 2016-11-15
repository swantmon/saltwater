//
//  base_operations.h
//  base
//
//  Created by Tobias Schwandt on 24/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_defines.h"
#include "base/base_math_constants.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace MATH
{
    inline float Sqrt(float _Radians)
    {
        return std::sqrt(_Radians);
    }
    
    // -----------------------------------------------------------------------------
    
    inline double Sqrt(double _Radians)
    {
        return std::sqrt(_Radians);
    }
    
    // -----------------------------------------------------------------------------
    
    inline float Sin(float _Radians)
    {
        return std::sin(_Radians);
    }
    
    // -----------------------------------------------------------------------------
    
    inline float Cos(float _Radians)
    {
        return std::cos(_Radians);
    }
    
    // -----------------------------------------------------------------------------
    
    inline float Tan(float _Radians)
    {
        return std::tan(_Radians);
    }

    // -----------------------------------------------------------------------------

    inline float Log(float _Value)
    {
        return std::logf(_Value);
    }

    // -----------------------------------------------------------------------------

    inline float Log2(float _Value)
    {
        return std::log2f(_Value);
    }
    
    // -----------------------------------------------------------------------------
    
    inline float DegreesToRadians(float _Degree)
    {
        return _Degree / 180.0f * SConstants<float>::s_Pi;
    }
    
    // -----------------------------------------------------------------------------
    
    inline float RadiansToDegree(float _Degree)
    {
        return _Degree * 180.0f / SConstants<float>::s_Pi;
    }
    
    // -----------------------------------------------------------------------------
    
    inline float Modulo(float _Dividend, float _Divisor)
    {
        return ::fmodf(_Dividend, _Divisor);
    }
    
    // -----------------------------------------------------------------------------
    
    inline double Modulo(double _Dividend, double _Divisor)
    {
        return ::fmod(_Dividend, _Divisor);
    }
    
    // -----------------------------------------------------------------------------
    
    inline int Abs(int _Value)
    {
        return ::abs(_Value);
    }
    
    // -----------------------------------------------------------------------------
    
    inline float Abs(float _Value)
    {
        return ::fabs(_Value);
    }

    // -----------------------------------------------------------------------------

    inline int Min(int _Value1, int _Value2)
    {
        return std::min(_Value1, _Value2);
    }
    
    // -----------------------------------------------------------------------------
    
    inline unsigned int Min(unsigned int _Value1, unsigned int _Value2)
    {
        return std::min(_Value1, _Value2);
    }

    // -----------------------------------------------------------------------------

    inline float Min(float _Value1, float _Value2)
    {
        return std::fmin(_Value1, _Value2);
    }

    // -----------------------------------------------------------------------------

    inline int Max(int _Value1, int _Value2)
    {
        return std::max(_Value1, _Value2);
    }
    
    // -----------------------------------------------------------------------------
    
    inline unsigned int Max(unsigned int _Value1, unsigned int _Value2)
    {
        return std::max(_Value1, _Value2);
    }

    // -----------------------------------------------------------------------------

    inline float Max(float _Value1, float _Value2)
    {
        return std::fmax(_Value1, _Value2);
    }
    
    // -----------------------------------------------------------------------------
    
    inline float IsEqual(float _Value1, float _Value2, float _Epsilon)
    {
        float Difference = _Value1 - _Value2;
        
        return ((Difference <= _Epsilon) && (Difference >= -_Epsilon));
    }

    // -----------------------------------------------------------------------------

    inline unsigned int Pow(unsigned int _X, unsigned int _Y)
    {
        return static_cast<unsigned int>(std::powf(static_cast<float>(_X), static_cast<float>(_Y)));
    }

    // -----------------------------------------------------------------------------

    inline float Pow(float _X, float _Y)
    {
        return std::powf(_X, _Y);
    }
    
    // -----------------------------------------------------------------------------
    
    inline float Lerp(float _Source, float _Destination, float _Blend)
    {
        return (1.0f - _Blend) * _Source + _Blend * _Destination;
    }

    // -----------------------------------------------------------------------------

    inline int Clamp(int _Source, int _Min, int _Max)
    {
        return std::max(_Min, std::min(_Source, _Max));
    }

    // -----------------------------------------------------------------------------

    inline unsigned int Clamp(unsigned int _Source, unsigned int _Min, unsigned int _Max)
    {
        return std::max(_Min, std::min(_Source, _Max));
    }

    // -----------------------------------------------------------------------------

    inline size_t Clamp(size_t _Source, size_t _Min, size_t _Max)
    {
        return std::max(_Min, std::min(_Source, _Max));
    }
    
    // -----------------------------------------------------------------------------
    
    inline float Clamp(float _Source, float _Min, float _Max)
    {
        return std::fmax(_Min, std::fmin(_Source, _Max));
    }
    
    // -----------------------------------------------------------------------------
    
    inline float Random()
    {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    inline void Swap(T& _rLeft, T& _rRight)
    {
        T Temp  = _rLeft;
        _rLeft  = _rRight;
        _rRight = Temp;
    }
} // namespace MATH
