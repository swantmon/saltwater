
#pragma once

#include "base/base_coordinate_system.h"
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
    public:

        Dt::CEntity* m_pSkyEntity = nullptr;
        Dt::CSkyComponent* m_pSkyComponent = nullptr;

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
                 //(ARGetBackgroundTextureFunc)(Core::PluginManager::GetPluginFunction("SLAM", "GetBackgroundTexture"));

                Net::CMessageDelegate Lambda = [](const Net::CMessage& _rMessage, auto _Port)
                {
                    std::cout << "Received message with ID " << _rMessage.m_ID << " on port " << _Port << " with length " << _rMessage.m_CompressedSize << '\n';

                    std::stringstream StringStream;
                    StringStream << "Received message with ID " << _rMessage.m_ID << " on port " << _Port << " with length " << _rMessage.m_CompressedSize;
                    std::string String = StringStream.str();
                    std::vector<char> Data(String.length());
                    std::memcpy(Data.data(), String.c_str(), String.length());

                    Net::CNetworkManager::GetInstance().SendMessage(1, Data);
                };

                m_NetworkDelegate = std::shared_ptr<Net::CMessageDelegate>(new Net::CMessageDelegate(Lambda));

                Net::CNetworkManager::GetInstance().RegisterMessageHandler(0, m_NetworkDelegate);
                Net::CNetworkManager::GetInstance().RegisterMessageHandler(1, m_NetworkDelegate);
                Net::CNetworkManager::GetInstance().RegisterMessageHandler(2, m_NetworkDelegate);
                Net::CNetworkManager::GetInstance().RegisterMessageHandler(3, m_NetworkDelegate);
                Net::CNetworkManager::GetInstance().RegisterMessageHandler(4, m_NetworkDelegate);
                Net::CNetworkManager::GetInstance().RegisterMessageHandler(5, m_NetworkDelegate);
            }
            else
            {

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

        }

    };
} // namespace Scpt