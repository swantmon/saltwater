
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_texture_manager.h"

#include "editor/edit_texture_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

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