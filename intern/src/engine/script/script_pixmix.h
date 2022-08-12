
#pragma once

#include "base/base_compression.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"
#include "engine/core/core_program_parameters.h"

#include "engine/engine.h"

#include "engine/script/script_script.h"

#include <zmq.hpp>

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

            InpaintWithPixMix = (InpaintWithPixMixFunc)(Core::PluginManager::GetPluginFunction("PixMix", "InpaintWithMask"));

            m_Threshold = Core::CProgramParameters::GetInstance().Get("mr:pixmix_server:color_threshold", 0);
            m_AlphaThreshold = Core::CProgramParameters::GetInstance().Get("mr:pixmix_server:alpha_threshold", 0);

            // -----------------------------------------------------------------------------
            // Setup ZMQ
            // -----------------------------------------------------------------------------
            int Port = Core::CProgramParameters::GetInstance().Get("mr:pixmix_server:network_port", 12346);

            int32_t msgHeader[8];
            msgHeader[0] = 0;
            msgHeader[1] = 0;
            
            zmq::context_t context;

            inSocket = zmq::socket_t(context, zmq::socket_type::pull);
            outSocket = zmq::socket_t(context, zmq::socket_type::push);

            inSocket.connect("tcp://localhost:" + std::to_string(Port));
            outSocket.connect("tcp://localhost:" + std::to_string(Port + 1));
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            zmq::message_t Message;
            inSocket.recv(Message);

            HandleMessage(Message);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }

        // -----------------------------------------------------------------------------

        void HandleMessage(const zmq::message_t& _rMessage)
        {
            auto pData = static_cast<const char*>(_rMessage.data());

            auto MsgType = *reinterpret_cast<const int*>(pData);
            
            if (MsgType == 0)
            {
                auto TextureSize = *reinterpret_cast<const glm::ivec2*>(pData + sizeof(int));
                auto PixelData = pData + sizeof(int) + sizeof(glm::ivec2);

                std::vector<glm::u8vec4> RawData(TextureSize.x * TextureSize.y);

                std::memcpy(RawData.data(), PixelData, sizeof(RawData[0]) * RawData.size());

                if (m_AlphaThreshold > 0)
                {
                    for (auto& Pixel : RawData)
                    {
                        if (Pixel.a > m_AlphaThreshold)
                        {
                            Pixel.a = 255;
                        }
                        else
                        {
                            Pixel.a = 0;
                        }
                    }
                }

                std::vector<glm::u8vec4> InpaintedImage(TextureSize.x * TextureSize.y);

                InpaintWithPixMix(TextureSize, RawData, InpaintedImage);

                //std::vector<char> Payload(InpaintedImage.size() * sizeof(InpaintedImage[0]) + sizeof(glm::ivec2));
                std::vector<char> Payload(_rMessage.size());

                auto pMsgType = reinterpret_cast<int*>(Payload.data());
                auto pTextureSize = reinterpret_cast<glm::ivec2*>(pMsgType + sizeof(*pMsgType));
                auto pPixelData = reinterpret_cast<char*>(pTextureSize + sizeof(*pTextureSize));

                *pMsgType = 0;
                *pTextureSize = TextureSize;
                std::memcpy(pPixelData, InpaintedImage.data(), InpaintedImage.size() * sizeof(InpaintedImage[0]));

                outSocket.send(zmq::buffer(Payload));
            }
            else if (MsgType == 1)
            {
                int Alpha;
                std::memcpy(&Alpha, pData + sizeof(int), sizeof(Alpha));
                m_AlphaThreshold = Alpha;
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

        int m_Threshold;
        int m_AlphaThreshold;

        zmq::socket_t inSocket;
        zmq::socket_t outSocket;
    };
} // namespace Scpt