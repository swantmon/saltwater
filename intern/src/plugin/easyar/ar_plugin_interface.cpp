
#include "plugin/easyar/ar_precompiled.h"

#include "base/base_include_glm.h"

#include "engine/core/core_asset_manager.h"
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
    auto loadFromImage(std::shared_ptr<easyar::ImageTracker> tracker, const std::string& path)
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

        tracker->loadTarget(target, [](std::shared_ptr<easyar::Target> target, bool status) 
        {
            std::printf("load target (%d): %s (%d)\n", status, target->name().c_str(), target->runtimeID());
        });

        return target;
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
    CPluginInterface::CPluginInterface()
        : m_IsActive(false)
    {

    }

    // -----------------------------------------------------------------------------

    CPluginInterface::~CPluginInterface()
    {

    }

    // -----------------------------------------------------------------------------

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

        float CameraNear = Core::CProgramParameters::GetInstance().Get("mr:camera:projection:near", 0.1f);

        float CameraFar = Core::CProgramParameters::GetInstance().Get("mr:camera:projection:far", 100.0f);

        glm::mat4 CameraInitialPose = Core::CProgramParameters::GetInstance().Get("mr:camera:initial_pose", glm::mat4(1.0f));

        // -----------------------------------------------------------------------------
        // Engine
        // -----------------------------------------------------------------------------
        m_Engine = std::make_shared<easyar::Engine>();

        m_IsActive = m_Engine->initialize(Key);

        if (m_IsActive == false) return;

        // -----------------------------------------------------------------------------
        // Camera
        // -----------------------------------------------------------------------------
        m_Camera.m_Native = std::make_shared<easyar::CameraDevice>();

        // -----------------------------------------------------------------------------
        // Frame Streamer
        // -----------------------------------------------------------------------------
        m_CameraFrameStreamer = std::make_shared<easyar::CameraFrameStreamer>();

        m_CameraFrameStreamer->attachCamera(m_Camera.m_Native);

        // -----------------------------------------------------------------------------
        // Open camera
        // -----------------------------------------------------------------------------
        m_Camera.m_Native->open(static_cast<int>(CameraDeviceType));

        // -----------------------------------------------------------------------------
        // Set settings to camera
        // -----------------------------------------------------------------------------
        m_Camera.m_Native->setFocusMode(CameraFocusMode);
        m_Camera.m_Native->setSize(easyar::Vec2I{ { m_CameraSize[0], m_CameraSize[1] } });
        m_Camera.m_Native->setHorizontalFlip(CameraFlipHorizontal);
        m_Camera.m_Native->setFrameRate(CameraFPS);

        m_Camera.m_ViewMatrix    = CameraInitialPose;
        m_Camera.m_Near          = CameraNear;
        m_Camera.m_Far           = CameraFar;
        m_Camera.m_TrackingState = CInternCamera::Tracking;

        auto Projection = m_Camera.m_Native->projectionGL(m_Camera.m_Near, m_Camera.m_Far);

        Base::CMemory::Copy(glm::value_ptr(m_Camera.m_ProjectionMatrix), &Projection.data[0], sizeof(easyar::Matrix44F));

        // -----------------------------------------------------------------------------
        // Set default marker
        // -----------------------------------------------------------------------------
        CInternTarget* pTarget = static_cast<CInternTarget*>(AcquireNewTarget("/marker/namecard.jpg"));

        // -----------------------------------------------------------------------------
        // Start everything
        // -----------------------------------------------------------------------------
        m_Camera.m_Native->start();
        m_CameraFrameStreamer->start();

        for (auto&& rrTracker : m_ImageTrackers)
        {
            rrTracker->start();
        }

        // -----------------------------------------------------------------------------
        // Check data
        // -----------------------------------------------------------------------------
        m_CameraSize[0] = m_Camera.m_Native->size().data[0];
        m_CameraSize[1] = m_Camera.m_Native->size().data[1];

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
            if(rrTracker) rrTracker->stop();
        }

        if(m_Camera.m_Native) m_Camera.m_Native->stop();
        if(m_CameraFrameStreamer) m_CameraFrameStreamer->stop();

        m_BackgroundTexturePtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
        if (m_IsActive == false) return;

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
                PossibleLostTargets.erase(ImageTarget->runtimeID());

                auto InternImageTarget = m_TrackedTargets[ImageTarget->runtimeID()];

                InternImageTarget.m_Native = ImageTarget;

                InternImageTarget.m_TrackingState = CInternTarget::Tracking;

                auto Pose = rrTargetInstance->poseGL();

                Base::CMemory::Copy(glm::value_ptr(m_Camera.m_ViewMatrix), &Pose.data[0], sizeof(easyar::Matrix44F));
            }
        }

        for (auto LostTarget : PossibleLostTargets)
        {
            auto ImageTarget = LostTarget.second;

            ImageTarget.m_TrackingState = CInternTarget::Lost;
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
        m_Camera.m_TrackingState = CCamera::Paused;

        m_Engine->onPause();

        m_IsActive = false;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        m_Engine->onResume();

        m_Camera.m_TrackingState = CCamera::Tracking;

        m_IsActive = true;
    }

    // -----------------------------------------------------------------------------

    const CCamera& CPluginInterface::GetCamera()
    {
        return m_Camera;
    }

    // -----------------------------------------------------------------------------

    CTarget* CPluginInterface::AcquireNewTarget(const std::string& _rPathToFile)
    {
        auto ImageTracker = std::make_shared<easyar::ImageTracker>();

        ImageTracker->attachStreamer(m_CameraFrameStreamer);

        auto Target = loadFromImage(ImageTracker, Core::AssetManager::GetPathToAssets() + _rPathToFile);

        m_ImageTrackers.push_back(ImageTracker);

        m_TrackedTargets[Target->runtimeID()].m_TrackingState = CInternTarget::Lost;

        return &m_TrackedTargets[Target->runtimeID()];
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::ReleaseTarget(CTarget* _pTarget)
    {

    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CPluginInterface::GetBackgroundTexture()
    {
        return m_BackgroundTexturePtr;
    }
} // namespace AR

extern "C" CORE_PLUGIN_API_EXPORT const AR::CCamera* GetCamera()
{
    return &static_cast<AR::CPluginInterface&>(GetInstance()).GetCamera();
}

extern "C" CORE_PLUGIN_API_EXPORT AR::CTarget* AcquireNewMarker(const std::string _rPathToFile)
{
    return static_cast<AR::CPluginInterface&>(GetInstance()).AcquireNewTarget(_rPathToFile);
}

extern "C" CORE_PLUGIN_API_EXPORT void ReleaseMarker(AR::CTarget* _pTarget)
{
    return static_cast<AR::CPluginInterface&>(GetInstance()).ReleaseTarget(_pTarget);
}

extern "C" CORE_PLUGIN_API_EXPORT Gfx::CTexturePtr GetBackgroundTexture()
{
    return static_cast<AR::CPluginInterface&>(GetInstance()).GetBackgroundTexture();
}