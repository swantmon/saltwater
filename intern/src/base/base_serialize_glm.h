
#pragma once

#include "base/base_defines.h"
#include "base/base_serialize_splitter.h"

#include "base/base_include_glm.h"

namespace SER
{
    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Write(TArchive & _rArchive, const glm::tvec2<T, P> & _rVec);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Read(TArchive & _rArchive, glm::tvec2<T, P> & _rVec);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Serialize(TArchive & _rArchive, glm::tvec3<T, P> & _rVec);
    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Write(TArchive& _rArchive, const glm::tvec2<T, P>& _rVec);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Read(TArchive& _rArchive, glm::tvec3<T, P>& _rVec);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Serialize(TArchive& _rArchive, glm::tvec3<T, P>& _rVec);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Write(TArchive & _rArchive, const glm::tvec4<T, P> & _rVec);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Read(TArchive & _rArchive, glm::tvec4<T, P> & _rVec);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Serialize(TArchive& _rArchive, glm::tvec4<T, P>& _rVec);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Write(TArchive & _rArchive, const glm::tquat<T, P> & _rQuat);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Read(TArchive & _rArchive, glm::tquat<T, P> & _rQuat);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Serialize(TArchive& _rArchive, glm::tquat<T, P>& _rQuat);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Write(TArchive & _rArchive, const glm::tmat3x3<T, P> & _rMat);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Read(TArchive & _rArchive, glm::tmat3x3<T, P> & _rMat);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Serialize(TArchive& _rArchive, glm::tmat3x3<T, P>& _rMat);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Write(TArchive & _rArchive, const glm::tmat4x4<T, P> & _rMat);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Read(TArchive & _rArchive, glm::tmat4x4<T, P> & _rMat);

    template<class TArchive, typename T, glm::precision P = glm::defaultp>
    inline void Serialize(TArchive & _rArchive, glm::tmat4x4<T, P> & _rMat);
} // namespace SER

namespace SER
{

    template<class TArchive, typename T, glm::precision P>
    inline void Write(TArchive& _rArchive, const glm::tvec2<T, P>& _rVec)
    {
        _rArchive << _rVec[0];
        _rArchive << _rVec[1];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Read(TArchive& _rArchive, glm::tvec2<T, P>& _rVec)
    {
        _rArchive >> _rVec[0];
        _rArchive >> _rVec[1];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Serialize(TArchive& _rArchive, glm::tvec2<T, P>& _rVec)
    {
        SER::SplitGlobalSerialize(_rArchive, _rVec);
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Write(TArchive& _rArchive, const glm::tvec3<T, P>& _rVec)
    {
        _rArchive << _rVec[0];
        _rArchive << _rVec[1];
        _rArchive << _rVec[2];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Read(TArchive& _rArchive, glm::tvec3<T, P>& _rVec)
    {
        _rArchive >> _rVec[0];
        _rArchive >> _rVec[1];
        _rArchive >> _rVec[2];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Serialize(TArchive& _rArchive, glm::tvec3<T, P>& _rVec)
    {
        SER::SplitGlobalSerialize(_rArchive, _rVec);
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Write(TArchive & _rArchive, const glm::tvec4<T, P> & _rVec)
    {
        _rArchive << _rVec[0];
        _rArchive << _rVec[1];
        _rArchive << _rVec[2];
        _rArchive << _rVec[3];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Read(TArchive & _rArchive, glm::tvec4<T, P> & _rVec)
    {
        _rArchive >> _rVec[0];
        _rArchive >> _rVec[1];
        _rArchive >> _rVec[2];
        _rArchive >> _rVec[3];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Serialize(TArchive & _rArchive, glm::tvec4<T, P> & _rVec)
    {
        SER::SplitGlobalSerialize(_rArchive, _rVec);
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Write(TArchive& _rArchive, const glm::tquat<T, P>& _rQuat)
    {
        _rArchive << _rQuat[0];
        _rArchive << _rQuat[1];
        _rArchive << _rQuat[2];
        _rArchive << _rQuat[3];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Read(TArchive& _rArchive, glm::tquat<T, P>& _rQuat)
    {
        _rArchive >> _rQuat[0];
        _rArchive >> _rQuat[1];
        _rArchive >> _rQuat[2];
        _rArchive >> _rQuat[3];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Serialize(TArchive& _rArchive, glm::tquat<T, P>& _rQuat)
    {
        SER::SplitGlobalSerialize(_rArchive, _rQuat);
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Write(TArchive& _rArchive, const glm::tmat3x3<T, P>& _rMat)
    {
        _rArchive << _rMat[0][0];
        _rArchive << _rMat[0][1];
        _rArchive << _rMat[0][2];

        _rArchive << _rMat[1][0];
        _rArchive << _rMat[1][1];
        _rArchive << _rMat[1][2];

        _rArchive << _rMat[2][0];
        _rArchive << _rMat[2][1];
        _rArchive << _rMat[2][2];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Read(TArchive& _rArchive, glm::tmat3x3<T, P>& _rMat)
    {
        _rArchive >> _rMat[0][0];
        _rArchive >> _rMat[0][1];
        _rArchive >> _rMat[0][2];

        _rArchive >> _rMat[1][0];
        _rArchive >> _rMat[1][1];
        _rArchive >> _rMat[1][2];

        _rArchive >> _rMat[2][0];
        _rArchive >> _rMat[2][1];
        _rArchive >> _rMat[2][2];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Serialize(TArchive& _rArchive, glm::tmat3x3<T, P>& _rMat)
    {
        SER::SplitGlobalSerialize(_rArchive, _rMat);
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Write(TArchive& _rArchive, const glm::tmat4x4<T, P>& _rMat)
    {
        _rArchive << _rMat[0][0];
        _rArchive << _rMat[0][1];
        _rArchive << _rMat[0][2];
        _rArchive << _rMat[0][3];

        _rArchive << _rMat[1][0];
        _rArchive << _rMat[1][1];
        _rArchive << _rMat[1][2];
        _rArchive << _rMat[1][3];

        _rArchive << _rMat[2][0];
        _rArchive << _rMat[2][1];
        _rArchive << _rMat[2][2];
        _rArchive << _rMat[2][3];

        _rArchive << _rMat[3][0];
        _rArchive << _rMat[3][1];
        _rArchive << _rMat[3][2];
        _rArchive << _rMat[3][3];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Read(TArchive& _rArchive, glm::tmat4x4<T, P>& _rMat)
    {
        _rArchive >> _rMat[0][0];
        _rArchive >> _rMat[0][1];
        _rArchive >> _rMat[0][2];
        _rArchive >> _rMat[0][3];

        _rArchive >> _rMat[1][0];
        _rArchive >> _rMat[1][1];
        _rArchive >> _rMat[1][2];
        _rArchive >> _rMat[1][3];

        _rArchive >> _rMat[2][0];
        _rArchive >> _rMat[2][1];
        _rArchive >> _rMat[2][2];
        _rArchive >> _rMat[2][3];

        _rArchive >> _rMat[3][0];
        _rArchive >> _rMat[3][1];
        _rArchive >> _rMat[3][2];
        _rArchive >> _rMat[3][3];
    }

    // -----------------------------------------------------------------------------

    template<class TArchive, typename T, glm::precision P>
    inline void Serialize(TArchive& _rArchive, glm::tmat4x4<T, P>& _rMat)
    {
        SER::SplitGlobalSerialize(_rArchive, _rMat);
    }
} // namespace SER