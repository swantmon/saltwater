
#include "plugin/easyar/ar_precompiled.h"

#include "base/base_include_glm.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"

#include "engine/engine.h"

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
        : m_IsActive                (false)
        , m_FlipVertical            (false)
        , m_pCameraImageData        (0)
        , m_pCameraTempImageLineData(0)
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
        // Hooks
        // -----------------------------------------------------------------------------
        Engine::RegisterEventHandler(Engine::Gfx_OnUpdate, ENGINE_BIND_EVENT_METHOD(&CPluginInterface::Gfx_OnUpdate));

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

        bool ShowSupportedProperties = Core::CProgramParameters::GetInstance().Get("mr:camera:show_supported_properties", false);

        m_FirstTargetIsWorldCenter = Core::CProgramParameters::GetInstance().Get("mr:camera:first_marker_is_world_center", true);

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

        // -----------------------------------------------------------------------------
        // Camera data
        // -----------------------------------------------------------------------------
        m_pCameraImageData = Base::CMemory::Allocate(m_CameraSize[0] * m_CameraSize[1] * 3 * sizeof(Base::U8));

        m_pCameraTempImageLineData = Base::CMemory::Allocate(m_CameraSize[0] * 3 * sizeof(Base::U8));

        // -----------------------------------------------------------------------------
        // Check supported properties
        // -----------------------------------------------------------------------------
        if (ShowSupportedProperties)
        {
            int NumberOfElements = m_Camera.m_Native->supportedFrameRateCount();

            if (NumberOfElements == 0)
            {
                ENGINE_CONSOLE_INFOV("Supported FPS data is not available.");
            }
            else
            {
                ENGINE_CONSOLE_INFOV("Supported FPS (count=%i):", NumberOfElements);

                for (int i = 0; i < NumberOfElements; ++i)
                {
                    ENGINE_CONSOLE_INFOV(" [%i]> %f FPS", i, m_Camera.m_Native->supportedFrameRate(i));
                }
            }

            // -----------------------------------------------------------------------------

            NumberOfElements = m_Camera.m_Native->supportedSizeCount();

            if (NumberOfElements == 0)
            {
                ENGINE_CONSOLE_INFOV("Supported sizes data is not available.");
            }
            else
            {
                ENGINE_CONSOLE_INFOV("Supported sizes (count=%i):", NumberOfElements);

                for (int i = 0; i < NumberOfElements; ++i)
                {
                    ENGINE_CONSOLE_INFOV(" [%i]> %i x %i", i, m_Camera.m_Native->supportedSize(i).data[0], m_Camera.m_Native->supportedSize(i).data[1]);
                }
            }
        }
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

        Base::CMemory::Free(m_pCameraImageData);

        Base::CMemory::Free(m_pCameraTempImageLineData);
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
        // Targets + camera
        // -----------------------------------------------------------------------------
        int NumberOfFoundTargets = 0;

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

                if (m_FirstTargetIsWorldCenter && NumberOfFoundTargets == 0)
                {
                    m_Camera.m_TrackingState = CInternCamera::Tracking;

                    Base::CMemory::Copy(glm::value_ptr(m_Camera.m_ViewMatrix), &Pose.data[0], sizeof(easyar::Matrix44F));

                    InternImageTarget.m_ModelMatrix = glm::mat4(1.0f);
                }
                else
                {
                    Base::CMemory::Copy(glm::value_ptr(InternImageTarget.m_ModelMatrix), &Pose.data[0], sizeof(easyar::Matrix44F));
                }

                ++ NumberOfFoundTargets;
            }
        }

        if (m_FirstTargetIsWorldCenter && NumberOfFoundTargets == 0)
        {
            m_Camera.m_TrackingState = CInternCamera::Undefined;
        }

        for (auto LostTarget : PossibleLostTargets)
        {
            auto ImageTarget = LostTarget.second;

            ImageTarget.m_TrackingState = CInternTarget::Lost;
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

        ImageTracker->start();

        m_ImageTrackers.push_back(ImageTracker);

        m_TrackedTargets[Target->runtimeID()].m_TrackingState = CInternTarget::Lost;

        return &m_TrackedTargets[Target->runtimeID()];
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::ReleaseTarget(CTarget* _pTarget)
    {
        BASE_UNUSED(_pTarget);
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CPluginInterface::GetBackgroundTexture()
    {
        return m_BackgroundTexturePtr;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::SetFlipVertical(bool _Flag)
    {
        m_FlipVertical = _Flag;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Gfx_OnUpdate()
    {
        // -----------------------------------------------------------------------------
        // Frame
        // -----------------------------------------------------------------------------
        auto Frame = m_CameraFrameStreamer->peek();

        for (auto Image : Frame->images())
        {
            Base::CMemory::Copy(m_pCameraImageData, Image->data(), m_CameraSize[0] * m_CameraSize[1] * 3 * sizeof(Base::U8));

            if (m_FlipVertical)
            {
                int NumberOfBytesPerLine = m_CameraSize[0] * 3 * sizeof(Base::U8);

                for (int y = 0; y < m_CameraSize[1] / 2; ++y)
                {
                    void* pFirstLine = (Base::U8*)m_pCameraImageData + (y * m_CameraSize[0] * 3);
                    void* pLastLine = (Base::U8*)m_pCameraImageData + ((m_CameraSize[1] - y - 1) * m_CameraSize[0] * 3);

                    Base::CMemory::Copy(m_pCameraTempImageLineData, pFirstLine, NumberOfBytesPerLine);

                    Base::CMemory::Copy(pFirstLine, pLastLine, NumberOfBytesPerLine);

                    Base::CMemory::Copy(pLastLine, m_pCameraTempImageLineData, NumberOfBytesPerLine);
                }
            }

            Gfx::TextureManager::CopyToTexture2D(m_BackgroundTexturePtr, Base::AABB2UInt(glm::uvec2(0.0f, 0.0f), glm::uvec2(m_CameraSize)), 0, m_pCameraImageData);
        }
    }
} // namespace AR

extern "C" CORE_PLUGIN_API_EXPORT const AR::CCamera* GetCamera()
{
    return &static_cast<AR::CPluginInterface&>(GetInstance()).GetCamera();
}

extern "C" CORE_PLUGIN_API_EXPORT AR::CTarget* AcquireNewTarget(const std::string _rPathToFile)
{
    return static_cast<AR::CPluginInterface&>(GetInstance()).AcquireNewTarget(_rPathToFile);
}

extern "C" CORE_PLUGIN_API_EXPORT void ReleaseTarget(AR::CTarget* _pTarget)
{
    return static_cast<AR::CPluginInterface&>(GetInstance()).ReleaseTarget(_pTarget);
}

extern "C" CORE_PLUGIN_API_EXPORT Gfx::CTexturePtr GetBackgroundTexture()
{
    return static_cast<AR::CPluginInterface&>(GetInstance()).GetBackgroundTexture();
}

extern "C" CORE_PLUGIN_API_EXPORT void SetFlipVertical(bool _Flag)
{
    static_cast<AR::CPluginInterface&>(GetInstance()).SetFlipVertical(_Flag);
}