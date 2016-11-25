
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_texture_manager.h"

#include "editor/edit_texture_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <windows.h>
#undef SendMessage

namespace
{
    class CTextureHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CTextureHelper)

    public:

        CTextureHelper();
        ~CTextureHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        void OnNewTexture(Edit::CMessage& _rMessage);

        void OnRequestTextureInfo(Edit::CMessage& _rMessage);

        void OnTextureInfo(Edit::CMessage& _rMessage);

        void OnDirtyTexture(Dt::CTextureBase* _pTexture);

        std::string CopyFileToAssets(const char* _pAssetFolder, const char* _pPathToFile);
    };
} // namespace

namespace
{
    CTextureHelper::CTextureHelper()
    {
        
    }

    // -----------------------------------------------------------------------------

    CTextureHelper::~CTextureHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Entity
        // -----------------------------------------------------------------------------
        Dt::TextureManager::RegisterDirtyTextureHandler(DATA_DIRTY_TEXTURE_METHOD(&CTextureHelper::OnDirtyTexture));

        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
//         Edit::MessageManager::Register(Edit::SGUIMessageType::NewMaterial, EDIT_RECEIVE_MESSAGE(&CTextureHelper::OnNewTexture));
// 
//         Edit::MessageManager::Register(Edit::SGUIMessageType::RequestMaterialInfo, EDIT_RECEIVE_MESSAGE(&CTextureHelper::OnRequestTextureInfo));
// 
//         Edit::MessageManager::Register(Edit::SGUIMessageType::MaterialInfo, EDIT_RECEIVE_MESSAGE(&CTextureHelper::OnTextureInfo));
    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnNewTexture(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnRequestTextureInfo(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnTextureInfo(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnDirtyTexture(Dt::CTextureBase* _pTexture)
    {
        assert(_pTexture != nullptr);

        if ((_pTexture->GetDirtyFlags() & Dt::CTextureBase::DirtyCreate) != 0)
        {
            
        }
    }

    // -----------------------------------------------------------------------------

    std::string CTextureHelper::CopyFileToAssets(const char* _pAssetFolder, const char* _pPathToFile)
    {
        char pDrive[4];
        char pDirectory[512];
        char pFilename[32];
        char pExtension[12];

        std::string FileExtension;
        std::string RelativePathToModel;

        _splitpath_s(_pPathToFile, pDrive, 4, pDirectory, 512, pFilename, 32, pExtension, 12);

        FileExtension = std::string(pFilename) + std::string(pExtension);
        RelativePathToModel = std::string(_pAssetFolder) + FileExtension;

        CopyFileA(_pPathToFile, RelativePathToModel.c_str(), true);

        return FileExtension.c_str();
    };
} // namespace

namespace Edit
{
namespace Helper
{
namespace Texture
{
    void OnStart()
    {
        CTextureHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CTextureHelper::GetInstance().OnExit();
    }
} // namespace Texture
} // namespace Helper
} // namespace Edit