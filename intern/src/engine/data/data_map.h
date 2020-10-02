
#pragma once

#include "engine/engine_config.h"

#include "base/base_aabb3.h"
#include "base/base_include_glm.h"
#include "base/base_typedef.h"

#include "engine/data/data_region.h"

namespace Dt
{
    class CEntity;
} // namespace Dt

namespace Dt
{
namespace Map
{
    class ENGINE_API CEntityIterator
    {
    public:

        CEntityIterator();

    public:

        CEntity& operator *  () const;
        CEntity* operator -> () const;

    public:

        CEntityIterator& Next();
        CEntityIterator& Next(unsigned int _Category);
        CEntityIterator& Next(const Base::AABB3Float& _rAABB);
        CEntityIterator& Next(unsigned int _Category, const Base::AABB3Float& _rAABB);
        CEntityIterator& Next(const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius);
        CEntityIterator& Next(unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius);

    public:

        bool operator == (const CEntityIterator& _rOther) const;
        bool operator != (const CEntityIterator& _rOther) const;

    protected:

        CEntityIterator(CEntity* _pEntity);

    private:

        CEntity* m_pCurrentEntity;
    };
} // namespace Map
} // namespace Dt

namespace Dt
{
namespace Map
{
    static const Base::Size s_MaxNumberOfRegionsX     = 128;
    static const Base::Size s_MaxNumberOfRegionsY     = 128;
    static const Base::Size s_MaxNumberOfRegions      = s_MaxNumberOfRegionsX * s_MaxNumberOfRegionsY;
    static const Base::Size s_MaxNumberOfMetersX      = s_MaxNumberOfRegionsX * CRegion::s_NumberOfMetersX;
    static const Base::Size s_MaxNumberOfMetersY      = s_MaxNumberOfRegionsY * CRegion::s_NumberOfMetersY;
    static const Base::Size s_MaxNumberOfSquareMeters = s_MaxNumberOfMetersX  * s_MaxNumberOfMetersY;
} // namespace Map
} // namespace Dt

namespace Dt
{
namespace Map
{
    ENGINE_API void AllocateMap(Base::Size _NumberOfRegionsX, Base::Size _NumberOfRegionsY);
    ENGINE_API void FreeMap();
    ENGINE_API bool HasMap();

    ENGINE_API CRegion* GetRegions();                                                                                                                                          ///< Returns a pointer to the region array.
    ENGINE_API CRegion& GetRegionByID(unsigned int _RegionID);                                                                                                                 ///< Returns the regions with the passed ID.
    ENGINE_API CRegion& GetRegionByPosition(float _AxisX, float _AxisY);                                                                                                       ///< Returns the region that covers the given world coordinate.
    ENGINE_API CRegion& GetRegionByPosition(const glm::vec3& _rPosition);                                                                                                      ///< Returns the region that covers the given world coordinate.
    
    ENGINE_API Base::Size GetNumberOfRegions();                                                                                                                                ///< Returns the number of regions in the map.
    ENGINE_API Base::Size GetNumberOfRegionsX();                                                                                                                               ///< Returns the number of regions in one row of the map.
    ENGINE_API Base::Size GetNumberOfRegionsY();                                                                                                                               ///< Returns the number of regions in one column of the map.
    
    ENGINE_API Base::Size GetNumberOfSquareMeters();                                                                                                                           ///< Returns the number of tiles in the map.
    ENGINE_API Base::Size GetNumberOfMetersX();                                                                                                                                ///< Returns the number of tiles in one row of the map.
    ENGINE_API Base::Size GetNumberOfMetersY();                                                                                                                                ///< Returns the number of tiles in one column of the map.

    ENGINE_API CEntityIterator EntitiesBegin();                                                                                                                                ///< Returns an iterator to the first entity in the map.
    ENGINE_API CEntityIterator EntitiesBegin(unsigned int _Category);                                                                                                          ///< Returns an iterator to the first entity of the given category.
    ENGINE_API CEntityIterator EntitiesBegin(const Base::AABB3Float& _rAABB);                                                                                                  ///< Returns an iterator to the first entity within the AABB.
    ENGINE_API CEntityIterator EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB);                                                                          ///< Returns an iterator to the first entity of the given category within the AABB.
    ENGINE_API CEntityIterator EntitiesBegin(const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius);                                                ///< Returns an iterator to the first entity within the given radius.
    ENGINE_API CEntityIterator EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius);                        ///< Returns an iterator to the first entity of the given category within the given radius.
    ENGINE_API CEntityIterator EntitiesEnd();                                                                                                                                  ///< Returns an iterator with a null pointer.
    
    ENGINE_API void AddEntity(CEntity& _rEntity);
    ENGINE_API void RemoveEntity(CEntity& _rEntity);
    ENGINE_API void MoveEntity(CEntity& _rEntity);

    ENGINE_API void Read(CSceneReader& _rCodec);
    ENGINE_API void Write(CSceneWriter& _rCodec);
} // namespace Map
} // namespace Dt
