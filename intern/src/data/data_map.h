//
//  data_map.h
//
//  Created by Tobias Schwandt.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_aabb3.h"
#include "base/base_typedef.h"
#include "base/base_vector2.h"

#include "data/data_region.h"

namespace Base
{
namespace Ser
{
    class CTextReader;
    class CTextWriter;
} // namespace Ser
} // namespace Base

namespace Dt
{
    class CEntity;
} // namespace Dt

namespace Dt
{
namespace Map
{
    class CEntityIterator
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
        CEntityIterator& Next(const Base::AABB3Float& _rAABB, const Base::Float3& _rPosition, float _SquareRadius);
        CEntityIterator& Next(unsigned int _Category, const Base::AABB3Float& _rAABB, const Base::Float3& _rPosition, float _SquareRadius);

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
    void AllocateMap(Base::Size _NumberOfRegionsX, Base::Size _NumberOfRegionsY);
    void FreeMap();
    bool HasMap();

    CRegion* GetRegions();                                                                                                                                          ///< Returns a pointer to the region array.
    CRegion& GetRegionByID(unsigned int _RegionID);                                                                                                                 ///< Returns the regions with the passed ID.
    CRegion& GetRegionByPosition(float _X, float _Y);                                                                                                               ///< Returns the region that covers the given world coordinate.
    CRegion& GetRegionByPosition(const Base::Float3& _rPosition);                                                                                                   ///< Returns the region that covers the given world coordinate.
    
    Base::Size GetNumberOfRegions();                                                                                                                                ///< Returns the number of regions in the map.
    Base::Size GetNumberOfRegionsX();                                                                                                                               ///< Returns the number of regions in one row of the map.
    Base::Size GetNumberOfRegionsY();                                                                                                                               ///< Returns the number of regions in one column of the map.
    
    Base::Size GetNumberOfSquareMeters();                                                                                                                           ///< Returns the number of tiles in the map.
    Base::Size GetNumberOfMetersX();                                                                                                                                ///< Returns the number of tiles in one row of the map.
    Base::Size GetNumberOfMetersY();                                                                                                                                ///< Returns the number of tiles in one column of the map.

    CEntityIterator EntitiesBegin();                                                                                                                                ///< Returns an iterator to the first entity in the map.
    CEntityIterator EntitiesBegin(unsigned int _Category);                                                                                              ///< Returns an iterator to the first entity of the given category.
    CEntityIterator EntitiesBegin(const Base::AABB3Float& _rAABB);                                                                                                  ///< Returns an iterator to the first entity within the AABB.
    CEntityIterator EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB);                                                              ///< Returns an iterator to the first entity of the given category within the AABB.
    CEntityIterator EntitiesBegin(const Base::AABB3Float& _rAABB, const Base::Float3& _rPosition, float _SquareRadius);                                             ///< Returns an iterator to the first entity within the given radius.
    CEntityIterator EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB, const Base::Float3& _rPosition, float _SquareRadius);         ///< Returns an iterator to the first entity of the given category within the given radius.
    CEntityIterator EntitiesEnd();                                                                                                                                  ///< Returns an iterator with a null pointer.
    
    void AddEntity(CEntity& _rEntity);
    void RemoveEntity(CEntity& _rEntity);
    void MoveEntity(CEntity& _rEntity);
} // namespace Map
} // namespace Dt
