
#include "engine/engine_precompiled.h"

#include "base/base_crc.h"
#include "base/base_memory.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_asset_importer.h"
#include "core/core_asset_generator.h"
#include "core/core_console.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "tinyxml2.h"

#include <map>

using namespace Core;
using namespace Core::AssetImporter;
using namespace Core::AssetGenerator;

namespace 
{
    class CCoreAssetImporter : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CCoreAssetImporter)

    public:

        const void* AllocateAssimpImporter(const std::string& _rFile, int _GeneratorFlag);

        const void* AllocateTinyXMLImporter(const std::string& _rFile);

        void* GetNativeAccessFromImporter(const void* _pImporter);

        void ReleaseImporter(const void* _pImporter);

    private:

        class CImporterInfo
        {
        public:

            enum EType
            {
                Assimp,
                TinyXML,
                NumberOfImporters
            };

            EType       m_Type;
            void*       m_pAccess;
            int         m_AllocateCount;
            Base::BHash m_Hash;

        public:

            CImporterInfo()
                : m_Type         (Assimp)
                , m_pAccess      (0)
                , m_AllocateCount(0)
                , m_Hash         (0)
            { 
            }
        };

    private:

        typedef std::map<Base::BHash, CImporterInfo> CImporterInfos;

    private:

        CImporterInfos m_ImporterInfos;

    private:

        CCoreAssetImporter();
        ~CCoreAssetImporter();

    private:

        int ConvertGenerationFlags(int _Flags);
    };
} // namespace 

namespace 
{
    CCoreAssetImporter::CCoreAssetImporter()
    {

    }

    // -----------------------------------------------------------------------------

    CCoreAssetImporter::~CCoreAssetImporter()
    {

    }

    // -----------------------------------------------------------------------------

    const void* CCoreAssetImporter::AllocateAssimpImporter(const std::string& _rFile, int _GeneratorFlag)
    {
        // -----------------------------------------------------------------------------
        // Create hash of file and test if importer is available
        // -----------------------------------------------------------------------------
        Base::BHash Hash = 0;
        
        Hash = Base::CRC32(Hash, _rFile.c_str(), static_cast<unsigned int>(_rFile.length()));

        Hash = Base::CRC32(Hash, &_GeneratorFlag, sizeof(_GeneratorFlag));

        if (m_ImporterInfos.find(Hash) != m_ImporterInfos.end())
        {
            auto& rInfo = m_ImporterInfos.at(Hash);

            ++rInfo.m_AllocateCount;

            return &rInfo;
        }

        // -----------------------------------------------------------------------------
        // Generate object / importer
        // -----------------------------------------------------------------------------
        Assimp::Importer* pImporter;

#if  PLATFORM_ANDROID
        pImporter = new Assimp::Importer();
#else
        pImporter = Base::CMemory::NewObject<Assimp::Importer>();
#endif

        int GeneratorFlag = ConvertGenerationFlags(_GeneratorFlag);

        const aiScene* pScene = pImporter->ReadFile(_rFile.c_str(), GeneratorFlag);

        if (!pScene)
        {
            ENGINE_CONSOLE_STREAMWARNING("Loading file '" << _rFile << "' with assimp failed (Code: " << pImporter->GetErrorString() << ").");

            return nullptr;
        }

        ENGINE_CONSOLE_STREAMINFO("Loading file '" << _rFile << "' with assimp succeeded.");

        // -----------------------------------------------------------------------------
        // Generate new info
        // -----------------------------------------------------------------------------
        auto& Info = m_ImporterInfos[Hash];

        Info.m_Type          = CImporterInfo::Assimp;
        Info.m_pAccess       = pImporter;
        Info.m_AllocateCount = 1;
        Info.m_Hash          = Hash;

        // -----------------------------------------------------------------------------
        // Return importer
        // -----------------------------------------------------------------------------
        return &Info;
    }

    // -----------------------------------------------------------------------------

    const void* CCoreAssetImporter::AllocateTinyXMLImporter(const std::string& _rFile)
    {
        // -----------------------------------------------------------------------------
        // Create hash of file and test if importer is available
        // -----------------------------------------------------------------------------
        Base::BHash Hash = Base::CRC32(_rFile.c_str(), static_cast<unsigned int>(_rFile.length()));

        if (m_ImporterInfos.find(Hash) != m_ImporterInfos.end())
        {
            auto& rInfo = m_ImporterInfos.at(Hash);

            ++rInfo.m_AllocateCount;

            return &rInfo;
        }

        // -----------------------------------------------------------------------------
        // Generate object
        // -----------------------------------------------------------------------------
        tinyxml2::XMLDocument* pMaterialFile;

#if  PLATFORM_ANDROID
        pMaterialFile = new tinyxml2::XMLDocument();
#else
        pMaterialFile = Base::CMemory::NewObject<tinyxml2::XMLDocument>();
#endif

        int Error = pMaterialFile->LoadFile(_rFile.c_str());

        if (Error != tinyxml2::XML_SUCCESS)
        {
            ENGINE_CONSOLE_STREAMWARNING("Loading xml file '" << _rFile << "' failed.");

            return nullptr;
        }

        ENGINE_CONSOLE_STREAMINFO("Loading xml file '" << _rFile << "' succeeded.");

        // -----------------------------------------------------------------------------
        // Generate new info
        // -----------------------------------------------------------------------------
        auto& Info = m_ImporterInfos[Hash];

        Info.m_Type          = CImporterInfo::TinyXML;
        Info.m_pAccess       = pMaterialFile;
        Info.m_AllocateCount = 1;
        Info.m_Hash          = Hash;

        // -----------------------------------------------------------------------------
        // Return importer
        // -----------------------------------------------------------------------------
        return &Info;
    }

    // -----------------------------------------------------------------------------

    void* CCoreAssetImporter::GetNativeAccessFromImporter(const void* _pImporter)
    {
        if (_pImporter == nullptr) return nullptr;
        
        return static_cast<const CImporterInfo*>(_pImporter)->m_pAccess;
    }

    // -----------------------------------------------------------------------------

    void CCoreAssetImporter::ReleaseImporter(const void* _pImporter)
    {
        if (_pImporter == nullptr) return;
        
        // -----------------------------------------------------------------------------
        // Get info
        // -----------------------------------------------------------------------------
        const CImporterInfo* pConstInfo = static_cast<const CImporterInfo*>(_pImporter);

        Base::BHash Hash = pConstInfo->m_Hash;

        if (m_ImporterInfos.find(Hash) == m_ImporterInfos.end())
        {
            return;
        }

        CImporterInfo& rInfo = m_ImporterInfos.at(Hash);

        // -----------------------------------------------------------------------------
        // Decrease allocate count
        // -----------------------------------------------------------------------------
        -- rInfo.m_AllocateCount;

        // -----------------------------------------------------------------------------
        // Release importer if allocate count is zero
        // -----------------------------------------------------------------------------
        if (rInfo.m_AllocateCount == 0)
        {
#if PLATFORM_ANDROID
            delete rInfo.m_pAccess;
#else
            Base::CMemory::DeleteObject(rInfo.m_pAccess);
#endif

            m_ImporterInfos.erase(Hash);
        }
    }

    // -----------------------------------------------------------------------------

    int CCoreAssetImporter::ConvertGenerationFlags(int _Flags)
    {
        int ReturnFlag = 0;

        if ((_Flags & SGeneratorFlag::Nothing) == SGeneratorFlag::Nothing)
        {
            ReturnFlag |= aiProcess_Triangulate;
        }

        if ((_Flags & SGeneratorFlag::Default) == SGeneratorFlag::Default)
        {
            ReturnFlag |= aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_GenUVCoords;
        }

        if ((_Flags & SGeneratorFlag::FlipUVs) == SGeneratorFlag::FlipUVs)
        {
            ReturnFlag |= aiProcess_FlipUVs;
        }

        if ((_Flags & SGeneratorFlag::RealtimeFast) == SGeneratorFlag::RealtimeFast)
        {
            ReturnFlag |= aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
        }

        return ReturnFlag;
    }
} // namespace 

namespace Core
{
namespace AssetImporter
{
    const void* AllocateAssimpImporter(const std::string& _rFile, int _GeneratorFlag)
    {
        return CCoreAssetImporter::GetInstance().AllocateAssimpImporter(_rFile, _GeneratorFlag);
    }

    // -----------------------------------------------------------------------------

    const void* AllocateTinyXMLImporter(const std::string& _rFile)
    {
        return CCoreAssetImporter::GetInstance().AllocateTinyXMLImporter(_rFile);
    }

    // -----------------------------------------------------------------------------

    void* GetNativeAccessFromImporter(const void* _pImporter)
    {
        return CCoreAssetImporter::GetInstance().GetNativeAccessFromImporter(_pImporter);
    }

    // -----------------------------------------------------------------------------

    void ReleaseImporter(const void* _pImporter)
    {
        CCoreAssetImporter::GetInstance().ReleaseImporter(_pImporter);
    }
} // namespace AssetImporter
} // namespace Core