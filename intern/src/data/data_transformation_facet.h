
#pragma once

#include "base/base_vector3.h"
#include "base/base_matrix4x4.h"

namespace Dt
{
    class CTransformationFacet
    {
    public:

        void SetPosition(const Base::Float3& _rPosition);
        Base::Float3& GetPosition();
        const Base::Float3& GetPosition() const;

        void SetRotation(const Base::Float3& _rRotation);
        Base::Float3& GetRotation();
        const Base::Float3& GetRotation() const;

        void SetScale(const Base::Float3& _rScale);
        Base::Float3& GetScale();
        const Base::Float3& GetScale() const;

        void SetWorldMatrix(Base::Float4x4& _rMatrix);
        Base::Float4x4& GetWorldMatrix();
        const Base::Float4x4& GetWorldMatrix() const;

    public:

        CTransformationFacet();
        ~CTransformationFacet();

    private:

        Base::Float4x4 m_WorldMatrix;           //< Absolute world matrix of the entity inside the map
        Base::Float3   m_Position;              //< Position of the entity to its parent
        Base::Float3   m_Rotation;              //< Rotation of the entity to its parent
        Base::Float3   m_Scale;                 //< Scale of the entity to its parent
    };
} // namespace Dt

