
#pragma once

#include "engine/script/script_script.h"

namespace Scpt
{
    class CARSettingsScript : public CScript<CARSettingsScript>
    {
    public:

        struct SPlaneFindingMode
        {
            enum Enum
            {
                Disabled,
                Horizontal,
                Vertical,
                HorizontalAndVertical,
            };
        };

        struct SLightingEstimation
        {
            enum Enum
            {
                Disabled,
                AmbientIntensity,
                HDREnvironment,
            };
        };

        SPlaneFindingMode::Enum m_PlaneFindingMode = SPlaneFindingMode::HorizontalAndVertical;
        SLightingEstimation::Enum m_LightEstimation = SLightingEstimation::HDREnvironment;

        bool m_HidePlanesAndPointsOnFirstMarker = true;
        bool m_RenderPlanes = true;
        bool m_RenderPoints = true;

    public:

        void Start() override
        {

        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {

        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent&) override
        {
        }

    public:

        inline void Read(CSceneReader& _rCodec) override
        {
            CComponent::Read(_rCodec);
        }

        inline void Write(CSceneWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);
        }

        inline IComponent* Allocate() override
        {
            return new CARSettingsScript();
        }
    };
} // namespace Scpt