
#include "core/core_precompiled.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_asset_importer.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

using namespace Core;
using namespace Core::AssetImporter;

namespace 
{
    class CCoreAssetImporter : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CCoreAssetImporter)

    public:

        int ConvertGenerationFlags(int _Flags);

    private:

        CCoreAssetImporter();
        ~CCoreAssetImporter();
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

    int CCoreAssetImporter::ConvertGenerationFlags(int _Flags)
    {
        int ReturnFlag = 0;

        if ((_Flags & SGeneratorFlag::Nothing) == SGeneratorFlag::Nothing)
        {
            ReturnFlag |= aiProcess_Triangulate;
        }

        if ((_Flags & SGeneratorFlag::Default) == SGeneratorFlag::Default)
        {
            ReturnFlag |= aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices;
        }

        if ((_Flags & SGeneratorFlag::FlipUVs) == SGeneratorFlag::FlipUVs)
        {
            ReturnFlag |= aiProcess_FlipUVs;
        }

        if ((_Flags & SGeneratorFlag::RealtimeFast) == SGeneratorFlag::RealtimeFast)
        {
            ReturnFlag |= aiProcess_GenUVCoords | aiProcess_SortByPType;
        }

        return ReturnFlag;
    }
} // namespace 

namespace Core
{
namespace AssetImporter
{
    int ConvertGenerationFlags(int _Flags)
    {
        return CCoreAssetImporter::GetInstance().ConvertGenerationFlags(_Flags);
    }
} // namespace AssetImporter
} // namespace Core