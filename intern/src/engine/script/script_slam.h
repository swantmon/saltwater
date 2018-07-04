
#pragma once

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
                
                m_NetworkDelegate = std::shared_ptr<Net::CMessageDelegate>(new Net::CMessageDelegate(std::bind(&CSLAMScript::OnNewMessage, this, std::placeholders::_1, std::placeholders::_2)));

                Net::CNetworkManager::GetInstance().RegisterMessageHandler(0, m_NetworkDelegate);
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

        }

    private:

        void OnNewMessage(const Net::CMessage& _rMessage, int _Port)
        {
            std::cout << "Received message with ID " << _rMessage.m_Category << " on port " << _Port << " with length " << _rMessage.m_CompressedSize << '\n';

            std::stringstream StringStream;
            StringStream << "Received message with ID " << _rMessage.m_Category << " on port " << _Port << " with length " << _rMessage.m_CompressedSize;
            std::string String = StringStream.str();
            std::vector<char> Data(String.length());
            std::memcpy(Data.data(), String.c_str(), String.length());

            Net::CNetworkManager::GetInstance().SendMessage(0, Data);
        }
    };
} // namespace Scpt