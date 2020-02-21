
#pragma once

#include "base/base_compression.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"
#include "engine/core/core_program_parameters.h"

#include "engine/engine.h"

#include "engine/script/script_script.h"

#include "engine/network/core_network_manager.h"

namespace Scpt
{
    class CPixMixScript : public CScript<CPixMixScript>
    {
    public:
                
        void Start() override
        {
            // -----------------------------------------------------------------------------
            // Load PixMix plugin with
            // -----------------------------------------------------------------------------

            if (!Core::PluginManager::LoadPlugin("PixMix"))
            {
                BASE_THROWM("PixMix plugin was not loaded");
            }

            InpaintWithPixMix = (InpaintWithPixMixFunc)(Core::PluginManager::GetPluginFunction("PixMix", "Inpaint"));

            // -----------------------------------------------------------------------------
            // Create server
            // -----------------------------------------------------------------------------

            auto SLAMDelegate = std::bind(&CPixMixScript::OnNewMessage, this, std::placeholders::_1, std::placeholders::_2);

            int Port = Core::CProgramParameters::GetInstance().Get("mr:pixmix_server:network_port", 12346);
            m_Socket = Net::CNetworkManager::GetInstance().CreateServerSocket(Port);
            m_NetHandle = Net::CNetworkManager::GetInstance().RegisterMessageHandler(m_Socket, SLAMDelegate);

            m_Threshold = Core::CProgramParameters::GetInstance().Get("mr:pixmix_server:color_threshold", 50);
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

        // -----------------------------------------------------------------------------

        void OnNewMessage(const Net::CMessage& _rMessage, Net::SocketHandle _SocketHandle)
        {
            BASE_UNUSED(_SocketHandle);

            if (_rMessage.m_MessageType == 0)
            {
                std::vector<char> Decompressed(_rMessage.m_DecompressedSize);

                if (_rMessage.m_CompressedSize != _rMessage.m_DecompressedSize)
                {
                    try
                    {
                        Base::Decompress(_rMessage.m_Payload, Decompressed);
                    }
                    catch (...)
                    {
                        ENGINE_CONSOLE_ERRORV("Failed to decompress! Ignoring network message!");
                        return;
                    }
                }
                else
                {
                    std::memcpy(Decompressed.data(), _rMessage.m_Payload.data(), Decompressed.size());
                }
                
                glm::ivec2 Size = *reinterpret_cast<glm::ivec2*>(Decompressed.data());

                std::vector<glm::u8vec4> RawData(Size.x * Size.y);

                std::memcpy(RawData.data(), Decompressed.data() + sizeof(glm::ivec2), sizeof(RawData[0]) * RawData.size());

                for (auto& Pixel : RawData)
                {
                    if (Pixel.r <= m_Threshold && Pixel.g <= m_Threshold && Pixel.b <= m_Threshold)
                    {
                        Pixel = glm::u8vec4(255);
                    }
                }

                std::vector<glm::u8vec4> InpaintedImage(Size.x * Size.y);

                InpaintWithPixMix(Size, RawData, InpaintedImage);

                std::vector<char> Payload(InpaintedImage.size() * sizeof(InpaintedImage[0]));
                std::memcpy(Payload.data(), InpaintedImage.data(), Payload.size());

                Net::CMessage Message;
                Message.m_Category = 0;
                Message.m_CompressedSize = static_cast<int>(Payload.size());
                Message.m_DecompressedSize = static_cast<int>(Payload.size());
                Message.m_MessageType = 0;
                Message.m_Payload = Payload;

                Net::CNetworkManager::GetInstance().SendMessage(m_Socket, Message);
            }
        }
        
    public:

        inline IComponent* Allocate() override
        {
            return new CPixMixScript();
        }

    private:

        using InpaintWithPixMixFunc = void(*)(const glm::ivec2&, const std::vector<glm::u8vec4>&, std::vector<glm::u8vec4>&);
        InpaintWithPixMixFunc InpaintWithPixMix;

        Net::SocketHandle m_Socket;
        Net::CNetworkManager::CMessageDelegate::HandleType m_NetHandle;

        int m_Threshold;
    };
} // namespace Scpt