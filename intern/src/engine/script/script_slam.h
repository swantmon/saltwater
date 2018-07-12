
#pragma once

#include "base/base_compression.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"

#include "engine/script/script_script.h"

#include "engine/network/core_network_manager.h"

namespace Scpt
{
    class CSLAMScript : public CScript<CSLAMScript>
    {
    private:

        uint16_t* Buffer;

        glm::mat4 PoseMatrix;

        enum EMessageType
        {
            COMMAND,
            TRANSFORM,
            DEPTHFRAME,
            COLORFRAME
        };

        typedef void(*InitializeCallback)(void);
        typedef void(*DepthFrameCallback)(const uint16_t*, const char*, const glm::mat4*);
        typedef void(*SizeAndIntrinsicsCallback)(glm::vec4, glm::vec4);

        InitializeCallback OnInitializeReconstructor;
        DepthFrameCallback OnNewDepthFrame;
        SizeAndIntrinsicsCallback OnSetImageSizesAndIntrinsics;
        
    private:
        
        std::shared_ptr<Net::CMessageDelegate> m_NetworkDelegate;

    public:

        void Start() override
        {
            // -----------------------------------------------------------------------------
            // Input
            // -----------------------------------------------------------------------------
            if (Core::PluginManager::HasPlugin("SLAM"))
            {                
                m_NetworkDelegate = std::shared_ptr<Net::CMessageDelegate>(new Net::CMessageDelegate(std::bind(&CSLAMScript::OnNewMessage, this, std::placeholders::_1, std::placeholders::_2)));

                Net::CNetworkManager::GetInstance().RegisterMessageHandler(0, m_NetworkDelegate);
                
                OnNewDepthFrame = (DepthFrameCallback)(Core::PluginManager::GetPluginFunction("SLAM", "OnNewDepthFrame"));
                OnInitializeReconstructor = (InitializeCallback)(Core::PluginManager::GetPluginFunction("SLAM", "InitializeReconstructor"));
                OnSetImageSizesAndIntrinsics = (SizeAndIntrinsicsCallback)(Core::PluginManager::GetPluginFunction("SLAM", "SetImageSizesAndIntrinsicData"));

                Buffer = new uint16_t[640 * 480];
            }
            else
            {
                throw Base::CException(__FILE__, __LINE__, "SLAM plugin was not loaded");
            }
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

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }

    private:

        void OnNewMessage(const Net::CMessage& _rMessage, int _Port)
        {
            BASE_UNUSED(_Port);

            std::vector<char> Decompressed(_rMessage.m_DecompressedSize);

            if (_rMessage.m_CompressedSize != _rMessage.m_DecompressedSize)
            {   
                Base::Decompress(_rMessage.m_Payload, Decompressed);
            }
            else
            {
                std::memcpy(Decompressed.data(), _rMessage.m_Payload.data(), Decompressed.size());
            }

            int32_t MessageType = *reinterpret_cast<int32_t*>(Decompressed.data());
            
            if (MessageType == TRANSFORM)
            {
                PoseMatrix = *reinterpret_cast<glm::mat4*>(Decompressed.data() + sizeof(int32_t));
            }
            else if (MessageType == DEPTHFRAME)
            {
                int32_t Width = *reinterpret_cast<int32_t*>(Decompressed.data() + sizeof(int32_t));
                int32_t Height = *reinterpret_cast<int32_t*>(Decompressed.data() + 2 * sizeof(int32_t));
                
                const float* RawBuffer = reinterpret_cast<float*>(Decompressed.data() + 3 * sizeof(int32_t));

                for (int i = 0; i < Width; ++ i)
                {
                    for (int j = 0; j < Height; ++ j)
                    {
                        Buffer[i * Height + j] = static_cast<uint16_t>(RawBuffer[i * Height + j]);
                    }
                }

                OnNewDepthFrame(Buffer, nullptr, &PoseMatrix);
            }
        }
    };
} // namespace Scpt