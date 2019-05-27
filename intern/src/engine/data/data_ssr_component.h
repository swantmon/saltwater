
#pragma once

#include "engine/engine_config.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CSSRComponent : public CComponent<CSSRComponent>
    {
    public:

        void SetIntensity(float _Intensity);
        float GetIntensity() const;

        void SetRoughnessMask(float _RoughnessMask);
        float GetRoughnessMask() const;

        void SetDistance(float _Distance);
        float GetDistance() const;

        void SetUseLastFrame(bool _Flag);
        bool GetUseLastFrame() const;

        void UpdateEffect();

    public:

        CSSRComponent();
        ~CSSRComponent();

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);

            _rCodec >> m_Intensity;
            _rCodec >> m_RoughnessMask;
            _rCodec >> m_Distance;
            _rCodec >> m_UseLastFrame
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);

            _rCodec << m_Intensity;
            _rCodec << m_RoughnessMask;
            _rCodec << m_Distance;
            _rCodec << m_UseLastFrame
        }

        inline IComponent* Allocate() override
        {
            return new CSSRComponent();
        }

    private:

        float m_Intensity;              //> Intensity of SSR on final image (Default: 1.0f)
        float m_RoughnessMask;          //> Identifier of the roughness depending surfaces. (Default: -6.66f)
        float m_Distance;               //> Distance in units between objects to detect reflection (equal to ray length)
        bool  m_UseLastFrame;           //> Either use double reflections of not (Default: true)

    private:

        friend class CSSRComponentGUI;
    };
} // namespace Dt