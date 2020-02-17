
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
                
            }
            else if (_rMessage.m_MessageType == 2)
            {

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
    };
} // namespace Scpt