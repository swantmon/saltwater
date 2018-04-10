
#pragma once

#include "engine/engine_config.h"

#include "base/base_typedef.h"

#include "data/data_component.h"

namespace Dt
{
    class ENGINE_API CSkyComponent : public CComponent<CSkyComponent>
    {
    public:

        enum EType
        {
            Procedural,      //< Sky is a procedural generated HDR depending on settings
            Panorama,        //< Sky will be created from panorama image
            Cubemap,         //< Sky is generated from given cube map
            Texture,         //< Sky is generated from given texture
            TextureGeometry, //< Sky is generated from given texture by using a geometry
            TextureLUT,      //< Sky is generated from given texture by using a LUT
        };

        enum ERefreshMode
        {
            Static,         //< Sky will be updated at any time the settings has changed
            Dynamic,        //< Sky will be updated at every frame (can be used for dynamic images)
        };

    public:

        void SetRefreshMode(ERefreshMode _RefreshMode);
        ERefreshMode GetRefreshMode();

        void SetType(EType _Type);
        EType GetType() const;

        void SetTexture(const std::string& _rTexture2D);
        const std::string& GetTexture();

        bool GetHasTexture() const;

        void SetIntensity(float _Intensity);
        float GetIntensity() const;

    public:

        CSkyComponent();
        ~CSkyComponent();

    private:

        ERefreshMode m_RefreshMode;        //< Refresh mode of the sky
        EType        m_Type;               //< Type of the skybox for procedural panorama or cubemap
        bool         m_HasHDR;             //< Declares either the image consists of HDR values
        std::string  m_Texture;            //< Texture
        float        m_Intensity;          //< Intensity of sky that is freely adjustable by artist (multiplier on the image)
    };
} // namespace Dt