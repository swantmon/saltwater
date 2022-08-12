
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

            zmq::context_t context;

            inSocket = zmq::socket_t(context, zmq::socket_type::pull);
            outSocket = zmq::socket_t(context, zmq::socket_type::push);

            inSocket.bind("tcp://*:" + std::to_string(Port + 1));
            outSocket.bind("tcp://*:" + std::to_string(Port));

            for (;;)
            {
                Update(); // Why is this not called automatically?
            }
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            zmq::message_t Msg;
            inSocket.recv(Msg);

            auto pData = static_cast<const char*>(Msg.data());

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
                std::vector<char> ResultMsg(Msg.size());

                auto pMsgType = ResultMsg.data();
                auto pTextureSize = pMsgType + sizeof(*pMsgType);
                auto pPixelData = pTextureSize + sizeof(*pTextureSize);

                *reinterpret_cast<int*>(pMsgType) = 0;
                *reinterpret_cast<glm::ivec2*>(pTextureSize) = TextureSize;
                std::memcpy(pPixelData, InpaintedImage.data(), InpaintedImage.size() * sizeof(InpaintedImage[0]));

                outSocket.send(zmq::buffer(ResultMsg));
            }
            else if (MsgType == 1)
            {
                int Alpha;
                std::memcpy(&Alpha, pData + sizeof(int), sizeof(Alpha));
                m_AlphaThreshold = Alpha;
            }
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
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