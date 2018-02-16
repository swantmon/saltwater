
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
        // -----------------------------------------------------------------------------
        // Read
        // -----------------------------------------------------------------------------
        char TextureName[256];

        _rMessage.GetString(TextureName, 256);

        // -----------------------------------------------------------------------------
        // Load
        // Check if texture is really created or only loaded!
        // -----------------------------------------------------------------------------
        Dt::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_Format           = Dt::STextureDescriptor::s_FormatFromSource;
        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDescriptor.m_Binding          = Dt::CTextureBase::ShaderResource;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = TextureName;
        TextureDescriptor.m_pIdentifier      = 0;

        Dt::CTextureBase* pLoadedTexture = Dt::TextureManager::CreateTexture(TextureDescriptor);

        // -----------------------------------------------------------------------------
        // Set hash
        // -----------------------------------------------------------------------------
        if (pLoadedTexture)
        {
            Dt::TextureManager::MarkTextureAsDirty(pLoadedTexture, Dt::CTextureBase::DirtyCreate);

            _rMessage.SetResult(pLoadedTexture->GetHash());
        }
        else
        {
            _rMessage.SetResult(-1);
        }
    }

    // -----------------------------------------------------------------------------

    void CTextureHelper::OnTextureInfo(Edit::CMessage& _rMessage)
    {
        unsigned int TextureHash = _rMessage.Get<int>();

        Dt::CTextureBase* pTexture = Dt::TextureManager::GetTextureByHash(static_cast<unsigned int>(TextureHash));

        if (pTexture != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pTexture->GetHash());
            NewMessage.Put(pTexture->GetDimension());
            NewMessage.Put(pTexture->GetFormat());
            NewMessage.Put(pTexture->GetSemantic());
            NewMessage.Put(pTexture->GetBinding());
            NewMessage.Put(pTexture->IsArray());
            NewMessage.Put(pTexture->IsCube());
            NewMessage.Put(pTexture->IsDummy());

            if (pTexture->GetFileName().length() > 0)
            {
                NewMessage.Put(true);

                NewMessage.PutString(pTexture->GetFileName().c_str());
            }
            else
            {
                NewMessage.Put(false);
            }

            if (pTexture->GetIdentifier().length() > 0)
            {
                NewMessage.Put(true);

                NewMessage.PutString(pTexture->GetIdentifier().c_str());
            }
            else
            {
                NewMessage.Put(false);
            }

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Texture_Info, NewMessage);

            _rMessage.SetResult(pTexture->GetHash());
        }
        else
        {
            _rMessage.SetResult(-1);
        }
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