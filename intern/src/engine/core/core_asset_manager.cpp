
#include "engine/engine_precompiled.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/core/core_asset_manager.h"

namespace 
{
    class CCoreAssetManager : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CCoreAssetManager)

    public:

        CCoreAssetManager();
        ~CCoreAssetManager();

    public:

        void SetFilePath(const std::string& _rAssetPath);

        const std::string GetPathToFiles();
        const std::string GetPathToAssets();
        const std::string GetPathToData();

    private:

        std::string m_PathToFiles;
    };
} // namespace 

namespace 
{
    CCoreAssetManager::CCoreAssetManager()
        : m_PathToFiles(".")
    {

    }

    // -----------------------------------------------------------------------------

    CCoreAssetManager::~CCoreAssetManager()
    {

    }

    void CCoreAssetManager::SetFilePath(const std::string& _rPath)
    {
        m_PathToFiles = _rPath;
    }

    // -----------------------------------------------------------------------------

    const std::string CCoreAssetManager::GetPathToFiles()
    {
        return m_PathToFiles;
    }

    // -----------------------------------------------------------------------------

    const std::string CCoreAssetManager::GetPathToAssets()
    {
        return m_PathToFiles + "/assets";
    }

    // -----------------------------------------------------------------------------

    const std::string CCoreAssetManager::GetPathToData()
    {
        return m_PathToFiles + "/data";
    }
} // namespace 

namespace Core
{
namespace AssetManager
{
    void SetFilePath(const std::string& _rPath)
    {
        CCoreAssetManager::GetInstance().SetFilePath(_rPath);
    }

    // -----------------------------------------------------------------------------

    const std::string GetPathToFiles()
    {
        return  CCoreAssetManager::GetInstance().GetPathToFiles();
    }

    // -----------------------------------------------------------------------------

    const std::string GetPathToAssets()
    {
        return CCoreAssetManager::GetInstance().GetPathToAssets();
    }

    // -----------------------------------------------------------------------------

    const std::string GetPathToData()
    {
        return CCoreAssetManager::GetInstance().GetPathToData();
    }
} // namespace AssetManager
} // namespace Core