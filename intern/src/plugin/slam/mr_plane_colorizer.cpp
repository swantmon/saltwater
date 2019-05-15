
#include "plugin/slam/slam_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include "plugin/slam/mr_plane_colorizer.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include <gl/glew.h>

using namespace MR;
using namespace Gfx;

namespace
{
    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

} // namespace

namespace MR
{
    // -----------------------------------------------------------------------------

    void CPlaneColorizer::UpdatePlane(int _PlaneID)
    {
        auto& rPlaneMap = m_pReconstructor->GetPlanes();

        auto Iter = rPlaneMap.find(_PlaneID);

        if (Iter != rPlaneMap.end())
        {
            auto& rPlane = Iter->second;

            UpdatePlane(rPlane);
        }
    }

    // -----------------------------------------------------------------------------

    void CPlaneColorizer::UpdatePlane(CSLAMReconstructor::SPlane& _rPlane)
    {
        if (_rPlane.m_TexturePtr == nullptr)
        {
            _rPlane.m_TexturePtr = m_DummyTexturePtr;
        }
    }
    
    // -----------------------------------------------------------------------------

    void CPlaneColorizer::ColorizeAllPlanes()
    {
        auto& rPlaneMap = m_pReconstructor->GetPlanes();

        std::vector<unsigned char> Pixels;

        for (int i = 0; i < m_PlaneTextureSize; ++ i)
        {
            for (int j = 0; j < m_PlaneTextureSize; ++ j)
            {
                Pixels.push_back(255);
                Pixels.push_back(20);
                Pixels.push_back(147);
                Pixels.push_back(255);
            }
        }

        for (auto& Iter : rPlaneMap)
        {
            auto& rPlane = Iter.second;

            if (rPlane.m_MeshPtr != nullptr)
            {
                if (rPlane.m_TexturePtr == m_DummyTexturePtr)
                {
                    STextureDescriptor TextureDescriptor = {};

                    TextureDescriptor.m_NumberOfPixelsU = m_PlaneTextureSize;
                    TextureDescriptor.m_NumberOfPixelsV = m_PlaneTextureSize;
                    TextureDescriptor.m_NumberOfPixelsW = 1;
                    TextureDescriptor.m_NumberOfMipMaps = 1;
                    TextureDescriptor.m_NumberOfTextures = 1;
                    TextureDescriptor.m_Binding = CTexture::ShaderResource;
                    TextureDescriptor.m_Access = CTexture::EAccess::CPURead;
                    TextureDescriptor.m_Usage = CTexture::EUsage::GPUReadWrite;
                    TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
                    TextureDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;
                    TextureDescriptor.m_pPixels = Pixels.data();

                    rPlane.m_TexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);
                }

                ColorizePlane(rPlane);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CPlaneColorizer::ColorizePlane(CSLAMReconstructor::SPlane& _rPlane)
    {

    }

    // -----------------------------------------------------------------------------

    CPlaneColorizer::CPlaneColorizer(MR::CSLAMReconstructor* _pReconstructor)
        : m_pReconstructor(_pReconstructor)
    {
        assert(_pReconstructor != nullptr);

        m_PlaneTextureSize = Core::CProgramParameters::GetInstance().Get("mr:plane_texture_size", 512);

        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = STextureDescriptor::s_NumberOfTexturesFromSource;
        TextureDescriptor.m_Binding = CTexture::ShaderResource;
        TextureDescriptor.m_Access = CTexture::EAccess::CPURead;
        TextureDescriptor.m_Usage = CTexture::EUsage::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;
        TextureDescriptor.m_pFileName = "textures/lines_d.png";

        m_DummyTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);
    }

    // -----------------------------------------------------------------------------

    CPlaneColorizer::~CPlaneColorizer()
    {

    }

} // namespace MR
