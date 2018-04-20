
#include "plugin/easyar/ar_precompiled.h"

#include "base/base_include_glm.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"

#include "engine/graphic/gfx_texture_manager.h"

#include "plugin/easyar/ar_plugin_interface.h"

#include "easyar/imagetarget.hpp"
#include "easyar/imagetracker.hpp"
#include "easyar/vector.hpp"

CORE_PLUGIN_INFO(AR::CPluginInterface, "EasyAR", "2.2.0", "This plugin enables the possibility to have AR on desktop computer.")

namespace 
{
    void loadFromImage(std::shared_ptr<easyar::ImageTracker> tracker, const std::string& path)
    {
        auto target = std::make_shared<easyar::ImageTarget>();
        std::string jstr = "{\n"
            "  \"images\" :\n"
            "  [\n"
            "    {\n"
            "      \"image\" : \"" + path + "\",\n"
            "      \"name\" : \"" + path.substr(0, path.find_first_of(".")) + "\"\n"
            "    }\n"
            "  ]\n"
            "}";
        target->setup(jstr.c_str(), static_cast<int>(easyar::StorageType::Assets) | static_cast<int>(easyar::StorageType::Json), "");
        tracker->loadTarget(target, [](std::shared_ptr<easyar::Target> target, bool status) {
            std::printf("load target (%d): %s (%d)\n", status, target->name().c_str(), target->runtimeID());
        });
    }

    void loadFromJsonFile(std::shared_ptr<easyar::ImageTracker> tracker, const std::string& path, const std::string& targetname)
    {
        auto target = std::make_shared<easyar::ImageTarget>();
        target->setup(path, static_cast<int>(easyar::StorageType::Assets), targetname);
        tracker->loadTarget(target, [](std::shared_ptr<easyar::Target> target, bool status) {
            std::printf("load target (%d): %s (%d)\n", status, target->name().c_str(), target->runtimeID());
        });
    }

    void loadAllFromJsonFile(std::shared_ptr<easyar::ImageTracker> tracker, const std::string& path)
    {
        for (auto && target : easyar::ImageTarget::setupAll(path, static_cast<int>(easyar::StorageType::Assets))) {
            tracker->loadTarget(target, [](std::shared_ptr<easyar::Target> target, bool status) {
                std::printf("load target (%d): %s (%d)\n", status, target->name().c_str(), target->runtimeID());
            });
        }
    }
} // namespace 

namespace AR
{
    void CPluginInterface::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Parameters
        // -----------------------------------------------------------------------------
        std::string Key = Core::CProgramParameters::GetInstance().Get("mr:plugin:easyar:key", "<INSERT YOUR EASYAR KEY HERE>");

        easyar::CameraDeviceFocusMode CameraFocusMode = Core::CProgramParameters::GetInstance().Get("mr:camera:focus_mode", easyar::CameraDeviceFocusMode::Normal);

        easyar::CameraDeviceType CameraDeviceType = Core::CProgramParameters::GetInstance().Get("mr:camera:device_type", easyar::CameraDeviceType::Default);

        bool CameraFlipHorizontal = Core::CProgramParameters::GetInstance().Get("mr:camera:flip_horizontal", false);

        float CameraFPS = Core::CProgramParameters::GetInstance().Get("mr:camera:FPS", 30.0f);

        m_CameraSize = Core::CProgramParameters::GetInstance().Get("mr:camera:size", glm::ivec2(1280, 720));

        // -----------------------------------------------------------------------------
        // Engine
        // -----------------------------------------------------------------------------
        m_Engine = std::make_shared<easyar::Engine>();

        m_Engine->initialize(Key);

        // -----------------------------------------------------------------------------
        // Camera
        // -----------------------------------------------------------------------------
        m_Camera = std::make_shared<easyar::CameraDevice>();

        // -----------------------------------------------------------------------------
        // Frame Streamer
        // -----------------------------------------------------------------------------
        m_CameraFrameStreamer = std::make_shared<easyar::CameraFrameStreamer>();

        m_CameraFrameStreamer->attachCamera(m_Camera);

        // -----------------------------------------------------------------------------
        // Open camera
        // -----------------------------------------------------------------------------
        m_Camera->open(static_cast<int>(CameraDeviceType));

        // -----------------------------------------------------------------------------
        // Set settings to camera
        // -----------------------------------------------------------------------------
        m_Camera->setFocusMode(CameraFocusMode);
        m_Camera->setSize(easyar::Vec2I{ { m_CameraSize[0], m_CameraSize[1] } });
        m_Camera->setHorizontalFlip(CameraFlipHorizontal);
        m_Camera->setFrameRate(CameraFPS);

        // -----------------------------------------------------------------------------
        // Image tracker + default targets
        // -----------------------------------------------------------------------------
        auto ImageTracker = std::make_shared<easyar::ImageTracker>();

        ImageTracker->attachStreamer(m_CameraFrameStreamer);

        loadFromImage(ImageTracker, "../data/plugins/easyar/namecard.jpg");

        m_ImageTrackers.push_back(ImageTracker);

        // -----------------------------------------------------------------------------
        // Start everything
        // -----------------------------------------------------------------------------
        m_Camera->start();
        m_CameraFrameStreamer->start();

        for (auto&& rrTracker : m_ImageTrackers)
        {
            rrTracker->start();
        }

        // -----------------------------------------------------------------------------
        // Check data
        // -----------------------------------------------------------------------------
        m_CameraSize[0] = m_Camera->size().data[0];
        m_CameraSize[1] = m_Camera->size().data[1];

        // -----------------------------------------------------------------------------
        // Texture
        // -----------------------------------------------------------------------------
        Gfx::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = m_CameraSize[0];
        TextureDescriptor.m_NumberOfPixelsV  = m_CameraSize[1];
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource;
        TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
        TextureDescriptor.m_Format           = Gfx::CTexture::Unknown;
        TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
        TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = Gfx::CTexture::B8G8R8_UBYTE;

        m_BackgroundTexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

        Gfx::TextureManager::SetTextureLabel(m_BackgroundTexturePtr, "EasyAR camera texture");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        for (auto&& rrTracker : m_ImageTrackers)
        {
            rrTracker->stop();
        }

        m_Camera->stop();
        m_CameraFrameStreamer->stop();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
        // -----------------------------------------------------------------------------
        // Frame
        // -----------------------------------------------------------------------------
        auto Frame = m_CameraFrameStreamer->peek();

        // -----------------------------------------------------------------------------
        // Targets
        // -----------------------------------------------------------------------------
        auto PossibleLostTargets = m_TrackedTargets;

        for (auto&& rrTargetInstance : Frame->targetInstances())
        {
            auto ImageTarget = std::dynamic_pointer_cast<easyar::ImageTarget>(rrTargetInstance->target());

            if (ImageTarget == nullptr) continue;

            auto TargetStatus = rrTargetInstance->status();

            if (TargetStatus == easyar::TargetStatus::Tracked)
            {
                if (m_TrackedTargets.count(ImageTarget->runtimeID()) == 0)
                {
                    m_TrackedTargets[ImageTarget->runtimeID()] = ImageTarget;
                }

                PossibleLostTargets.erase(ImageTarget->runtimeID());

                auto Pose = rrTargetInstance->poseGL();

                // ...
            }
        }

        for (auto LostTarget : PossibleLostTargets)
        {
            auto ImageTarget = LostTarget.second;

            if (m_TrackedTargets.count(ImageTarget->runtimeID()) > 0)
            {
                m_TrackedTargets.erase(ImageTarget->runtimeID());
            }
        }

        // -----------------------------------------------------------------------------
        // Background image
        // -----------------------------------------------------------------------------
        for (auto Image : Frame->images())
        {
            Gfx::TextureManager::CopyToTexture2D(m_BackgroundTexturePtr, Base::AABB2UInt(glm::uvec2(0.0f, 0.0f), glm::uvec2(m_CameraSize)), 0, Image->data());
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        m_Engine->onPause();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        m_Engine->onResume();
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CPluginInterface::GetBackgroundTexture()
    {
        return m_BackgroundTexturePtr;
    }
} // namespace AR

extern "C" CORE_PLUGIN_API_EXPORT Gfx::CTexturePtr GetBackgroundTexture()
{
    return static_cast<AR::CPluginInterface&>(GetInstance()).GetBackgroundTexture();
}