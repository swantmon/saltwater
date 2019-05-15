
#pragma once

#include <array>
#include <regex>

namespace Dt
{
    class CEntity;
}

namespace Edit
{
    class CAsset
    {
    public:

        enum EType
        {
            Model,
            Material,
            Texture,
            Scene,
            Record,
            NumberOfTypes,
            Undefined = -1,
        };

    public:

        static std::array<std::regex, NumberOfTypes> s_Filter;

    public:

        CAsset(const std::string& _rFilename);

    public:

        const std::string& GetPathToFile() const;
        EType GetType() const;

    protected:

        std::string m_PathToFile;
        EType m_Type;

    private:

        void Identify();
    };

    
} // namespace Edit

namespace Edit
{
namespace AssetHelper
{
    Dt::CEntity* LoadPrefabFromModel(const CAsset& _rAsset);
} // namespace AssetHelper
} // namespace Edit
