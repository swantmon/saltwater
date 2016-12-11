
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

        void OnTextureNew(Edit::CMessage& _rMessage);

        void OnTextureLoad(Edit::CMessage& _rMessage);

        void OnTextureInfo(Edit::CMessage& _rMessage);

        void OnTextureUpdate(Edit::CMessage& _rMessage);

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
        Edit::MessageManager::Register(Edit::SGUIMessageType::Texture_New, EDIT_RECEIVE_MESSAGE(&CTextureHelper::OnTextureNew));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Texture_Load, EDIT_RECEIVE_MESSAGE(&CTextureHelper::OnTextureLoad));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Texture_Info, EDIT_RECEIVE_MESSAGE(&CTextureHelper::OnTextureInfo));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Texture_Update, EDIT_RECEIVE_MESSAGE(&CTextureHelper::OnTextureUpdate));
    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnTextureNew(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnTextureLoad(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnTextureInfo(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnTextureUpdate(Edit::CMessage& _rMessage)
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