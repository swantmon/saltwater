
#pragma once

#include "engine/data/data_sky_component.h"

#include "editor_imgui/imgui/imgui.h"

#include "engine/graphic/gfx_texture_manager.h"

namespace Dt
{
    class CSkyComponentGUI : public CSkyComponent
    {
    public:

        void OnGUI()
        {
            // -----------------------------------------------------------------------------
            // Refresh mode
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Static", "Dynamic" };

                int Index = static_cast<int>(GetRefreshMode());

                ImGui::Combo("Refresh Mode", &Index, Text, 2);

                SetRefreshMode(static_cast<ERefreshMode>(Index));
            }

            // -----------------------------------------------------------------------------
            // Type
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Procedural", "Panorama", "Cubemap" };

                int Index = static_cast<int>(GetType());

                ImGui::Combo("Type", &Index, Text, 3);

                SetType(static_cast<EType>(Index));
            }

            // -----------------------------------------------------------------------------
            // Quality
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "64", "128", "256", "512", "1024", "2048" };

                int Index = static_cast<int>(GetQuality());

                ImGui::Combo("Quality", &Index, Text, 6);

                SetQuality(static_cast<EQuality>(Index));
            }

            // -----------------------------------------------------------------------------
            // Rest
            // -----------------------------------------------------------------------------
            static char PathToTexture[255] = {};

            if (ImGui::InputText("Path to Texture", PathToTexture, 255) && strlen(PathToTexture) > 0)
            {
                Gfx::STextureDescriptor TextureDescriptor; 

                TextureDescriptor.m_NumberOfPixelsU  = Gfx::STextureDescriptor::s_NumberOfPixelsFromSource; 
                TextureDescriptor.m_NumberOfPixelsV  = Gfx::STextureDescriptor::s_NumberOfPixelsFromSource; 
                TextureDescriptor.m_NumberOfPixelsW  = 1;
                TextureDescriptor.m_NumberOfMipMaps  = 1;
                TextureDescriptor.m_NumberOfTextures = 1;
                TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite; 
                TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead; 
                TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse; 
                TextureDescriptor.m_pFileName        = 0; 
                TextureDescriptor.m_pPixels          = 0; 
                TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource;
                TextureDescriptor.m_Format           = Gfx::STextureDescriptor::s_FormatFromSource;
                TextureDescriptor.m_pFileName        = PathToTexture;

                if (GetType() == Dt::CSkyComponent::Cubemap)
                {
                    TextureDescriptor.m_NumberOfTextures = 6;

                    SetTexture(Gfx::TextureManager::CreateCubeTexture(TextureDescriptor));
                }
                else
                {
                    SetTexture(Gfx::TextureManager::CreateTexture2D(TextureDescriptor));
                }
            }

            ImGui::DragFloat("Intensity", &m_Intensity);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Sky";
        }
    };
} // namespace Dt
