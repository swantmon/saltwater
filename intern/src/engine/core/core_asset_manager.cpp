
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/core/core_asset_manager.h"

#include <filesystem>

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

        if (!std::filesystem::exists(m_PathToFiles) || !std::filesystem::exists(GetPathToData()))
        {
            BASE_THROWV("The root folder ('%s') is incorrect or path to data is not available.", m_PathToFiles.c_str());
        }

        auto PathToAssets = GetPathToAssets();

        if (!std::filesystem::exists(PathToAssets))
        {
            std::filesystem::create_directory(PathToAssets);

            ENGINE_CONSOLE_WARNINGV("The folder %s does not exists. The engine has created an empty one.", PathToAssets.c_str());
        }
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