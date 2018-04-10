
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"
#include "base/base_memory.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_console.h"

#include "data/data_entity.h"
#include "data/data_entity_folder.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::Map;

namespace
{
    class CDtLvlMap : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtLvlMap);
        
    public:

        CDtLvlMap();
       ~CDtLvlMap();

    public:

        void AllocateMap(Base::Size _NumberOfRegionsX, Base::Size _NumberOfRegionsY);
        void FreeMap();
        bool HasMap() const;

    public:

        CRegion* GetRegions() const;
        CRegion& GetRegionByID(unsigned int _RegionID) const;
        CRegion& GetRegionByPosition(float _AxisX, float _AxisY) const;
        CRegion& GetRegionByPosition(const glm::vec3& _rPosition) const;

        Base::Size GetNumberOfRegions() const;
        Base::Size GetNumberOfRegionsX() const;
        Base::Size GetNumberOfRegionsY() const;

    public:

        Base::Size GetNumberOfSquareMeters() const;
        Base::Size GetNumberOfMetersX() const;
        Base::Size GetNumberOfMetersY() const;

    public:

        void AddEntity(CEntity& _rEntity);
        void MoveEntity(CEntity& _rEntity);
        void RemoveEntity(CEntity& _rEntity);

        CEntityIterator EntitiesBegin() const;
        CEntityIterator EntitiesBegin(unsigned int _Category) const;
        CEntityIterator EntitiesBegin(const Base::AABB3Float& _rAABB) const;
        CEntityIterator EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB) const;
        CEntityIterator EntitiesBegin(const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const;
        CEntityIterator EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const;
        CEntityIterator EntitiesEnd() const;
        
    public:
        
        CEntity* GetFirstEntity() const;
        CEntity* GetFirstEntity(unsigned int _Category) const;
        CEntity* GetFirstEntity(const Base::AABB3Float& _rAABB) const;
        CEntity* GetFirstEntity(unsigned int _Category, const Base::AABB3Float& _rAABB) const;
        CEntity* GetFirstEntity(const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const;
        CEntity* GetFirstEntity(unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const;
        CEntity* GetNextEntity(CEntity* _pEntity) const;
        CEntity* GetNextEntity(CEntity* _pEntity, unsigned int _Category) const;
        CEntity* GetNextEntity(CEntity* _pEntity, const Base::AABB3Float& _rAABB) const;
        CEntity* GetNextEntity(CEntity* _pEntity, unsigned int _Category, const Base::AABB3Float& _rAABB) const;
        CEntity* GetNextEntity(CEntity* _pEntity, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const;
        CEntity* GetNextEntity(CEntity* _pEntity, unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const;

    private:

        class CInternEntityIterator : public CEntityIterator
        {
            public:

                CInternEntityIterator(CEntity* _pEntity);
                CInternEntityIterator(CEntityIterator _Iterator);
        };

    private:

        bool                       m_HasMap;
        Base::Size                 m_NumberOfEntities;
        Base::Size                 m_NumberOfRegions;
        Base::Size                 m_NumberOfRegionsX;
        Base::Size                 m_NumberOfRegionsY;
        CRegion*                   m_pRegions;
        Base::Size                 m_NumberOfMetersX;
        Base::Size                 m_NumberOfMetersY;
    
    private:

        bool IsValid(const Base::AABB3Float& _rAABB) const;
    };
} // namespace

namespace
{
    CDtLvlMap::CDtLvlMap()
        : m_HasMap                  (false)
        , m_NumberOfEntities        (0)
        , m_NumberOfRegions         (0)
        , m_NumberOfRegionsX        (0)
        , m_NumberOfRegionsY        (0)
        , m_pRegions                (nullptr)
        , m_NumberOfMetersX         (0)
        , m_NumberOfMetersY         (0)
    {
        
    }

    // -----------------------------------------------------------------------------

    CDtLvlMap::~CDtLvlMap()
    {
        assert(m_pRegions == nullptr);
    }
   
    // -----------------------------------------------------------------------------

    void CDtLvlMap::AllocateMap(Base::Size _NumberOfRegionsX, Base::Size _NumberOfRegionsY)
    {
        int        Category;
        Base::Size IndexOfRegion;

        // -----------------------------------------------------------------------------
        // Check if the map was not already allocated before.
        // -----------------------------------------------------------------------------
        assert(m_pRegions        == nullptr);
        assert(_NumberOfRegionsX <= s_MaxNumberOfRegionsX);
        assert(_NumberOfRegionsY <= s_MaxNumberOfRegionsY);

        // -----------------------------------------------------------------------------
        // Take care of exception safety.
        // -----------------------------------------------------------------------------
        try
        {
            ENGINE_CONSOLE_STREAMINFO("Data> Create new map with regions: x=" << _NumberOfRegionsX << ", y=" << _NumberOfRegionsY);

            m_NumberOfRegions  = _NumberOfRegionsX * _NumberOfRegionsY;
            m_NumberOfRegionsX = _NumberOfRegionsX;
            m_NumberOfRegionsY = _NumberOfRegionsY;
            m_NumberOfMetersX  = _NumberOfRegionsX * CRegion::s_NumberOfMetersX;
            m_NumberOfMetersY  = _NumberOfRegionsY * CRegion::s_NumberOfMetersY;

            // -----------------------------------------------------------------------------
            // Create the region array.
            // -----------------------------------------------------------------------------
            m_pRegions = static_cast<CRegion*>(Base::CMemory::Allocate(m_NumberOfRegions * sizeof(*m_pRegions)));

            // -----------------------------------------------------------------------------
            // Setup the regions.
            // -----------------------------------------------------------------------------
            for (IndexOfRegion = 0; IndexOfRegion < m_NumberOfRegions; ++ IndexOfRegion)
            {
                CRegion& rRegion = m_pRegions[IndexOfRegion];

                Base::CMemory::ConstructObject<CRegion>(&rRegion);

                rRegion.m_ID          = static_cast<CRegion::BID>(IndexOfRegion);
                rRegion.m_Position[0] = static_cast<int>(IndexOfRegion % _NumberOfRegionsX * CRegion::s_NumberOfMetersX);
                rRegion.m_Position[1] = static_cast<int>(IndexOfRegion / _NumberOfRegionsX * CRegion::s_NumberOfMetersY);
                rRegion.m_AABB[0][0]  = static_cast<float>(rRegion.m_Position[0]);
                rRegion.m_AABB[0][1]  = static_cast<float>(rRegion.m_Position[1]);
                rRegion.m_AABB[0][2]  = 0.0f;
                rRegion.m_AABB[1][0]  = rRegion.m_AABB[0][0] + static_cast<float>(CRegion::s_NumberOfMetersX);
                rRegion.m_AABB[1][1]  = rRegion.m_AABB[0][1] + static_cast<float>(CRegion::s_NumberOfMetersY);
                rRegion.m_AABB[1][2]  = 0.0f;

                for (Category = 0; Category < Dt::SEntityCategory::NumberOfCategories; ++ Category)
                {
                    CEntityFolder& rFolder = rRegion.m_EntityFolders[Category];

                    rFolder.m_Category  = static_cast<unsigned int>(Category);
                    rFolder.m_pRegion   = &rRegion;
                    rFolder.m_pEntities = nullptr;
                }
            }
        }
        catch (...)
        {
            FreeMap();

            BASE_THROWV("Map of size x=%i, y=%i can't be created!", _NumberOfRegionsX, _NumberOfRegionsY);
        }
        
        m_HasMap = true;
    }

    // -----------------------------------------------------------------------------

    void CDtLvlMap::FreeMap()
    {
        ENGINE_CONSOLE_STREAMINFO("Data> Free current map.");

        Base::Size IndexOfRegion;

        // -----------------------------------------------------------------------------
        // Iterate throw every entity and remove from map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin();
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities;)
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Re-link entity
            // -----------------------------------------------------------------------------
            assert(rCurrentEntity.GetFolder() != nullptr);

            if (rCurrentEntity.GetPrevious() != nullptr)
            {
                rCurrentEntity.GetPrevious()->SetNext(rCurrentEntity.GetNext());
            }
            else
            {
                rCurrentEntity.GetFolder()->m_pEntities = rCurrentEntity.GetNext();
            }

            if (rCurrentEntity.GetNext() != nullptr)
            {
                rCurrentEntity.GetNext()->SetPrevious(rCurrentEntity.GetPrevious());
            }

            // -----------------------------------------------------------------------------
            // Decrease entity counter
            // -----------------------------------------------------------------------------
            --m_NumberOfEntities;

            // -----------------------------------------------------------------------------
            // Set next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next();
        }
        
        // -----------------------------------------------------------------------------
        // Only if the regions have to be freed.
        // -----------------------------------------------------------------------------
        if (m_pRegions != nullptr)
        {
            for (IndexOfRegion = 0; IndexOfRegion < m_NumberOfRegions; ++ IndexOfRegion)
            {
                CRegion& rRegion = m_pRegions[IndexOfRegion];

                Base::CMemory::DestructObject(&rRegion);
            }

            Base::CMemory::Free(m_pRegions);

            m_NumberOfRegionsX = 0;
            m_NumberOfRegionsY = 0;
            m_NumberOfRegions  = 0;
            m_pRegions         = nullptr;
        }

        // -----------------------------------------------------------------------------
        // Only if the meters have to be freed.
        // -----------------------------------------------------------------------------
        m_NumberOfMetersX = 0;
        m_NumberOfMetersY = 0;
        
        // -----------------------------------------------------------------------------
        // Reset all entities in map
        // -----------------------------------------------------------------------------
        m_NumberOfEntities = 0;

        m_HasMap = false;
    }

    // -----------------------------------------------------------------------------

    bool CDtLvlMap::HasMap() const
    {
        return m_HasMap;
    }

    // -----------------------------------------------------------------------------

    CRegion* CDtLvlMap::GetRegions() const
    {
        assert(m_pRegions != nullptr);

        return m_pRegions;
    }

    // -----------------------------------------------------------------------------

    CRegion& CDtLvlMap::GetRegionByID(unsigned int _RegionID) const
    {
        assert(m_pRegions != nullptr);

        return m_pRegions[_RegionID];
    }

    // -----------------------------------------------------------------------------

    CRegion& CDtLvlMap::GetRegionByPosition(float _AxisX, float _AxisY) const
    {
        Base::Size RegionX;
        Base::Size RegionY;
        Base::Size RegionID;

        assert(m_pRegions != nullptr);
        assert(m_NumberOfRegions > 0);

        RegionX  = static_cast<Base::Size>(_AxisX) / CRegion::s_NumberOfMetersX;
        RegionY  = static_cast<Base::Size>(_AxisY) / CRegion::s_NumberOfMetersY;
        RegionID = RegionX + RegionY * m_NumberOfRegionsX;

        RegionID = glm::clamp(static_cast<unsigned int>(RegionID), 0u, static_cast<unsigned int>(m_NumberOfRegionsX * m_NumberOfRegionsY - 1));

        return m_pRegions[RegionID];
    }

    // -----------------------------------------------------------------------------

    CRegion& CDtLvlMap::GetRegionByPosition(const glm::vec3& _rPosition) const
    {
        return GetRegionByPosition(_rPosition[0], _rPosition[1]);
    }

    // -----------------------------------------------------------------------------

    Base::Size CDtLvlMap::GetNumberOfRegions() const
    {
        return m_NumberOfRegions;
    }

    // -----------------------------------------------------------------------------

    Base::Size CDtLvlMap::GetNumberOfRegionsX() const
    {
        return m_NumberOfRegionsX;
    }

    // -----------------------------------------------------------------------------

    Base::Size CDtLvlMap::GetNumberOfRegionsY() const
    {
        return m_NumberOfRegionsY;
    }

    // -----------------------------------------------------------------------------

    Base::Size CDtLvlMap::GetNumberOfSquareMeters() const
    {
        return m_NumberOfMetersX * m_NumberOfMetersY;
    }

    // -----------------------------------------------------------------------------

    Base::Size CDtLvlMap::GetNumberOfMetersX() const
    {
        return m_NumberOfMetersX;
    }

    // -----------------------------------------------------------------------------

    Base::Size CDtLvlMap::GetNumberOfMetersY() const
    {
        return m_NumberOfMetersY;
    }

    // -----------------------------------------------------------------------------

    void CDtLvlMap::AddEntity(CEntity& _rEntity)
    {
        // -----------------------------------------------------------------------------
        // Get the region where to add the entity.
        // -----------------------------------------------------------------------------
        CRegion& rRegion = GetRegionByPosition(_rEntity.GetWorldPosition());

        // -----------------------------------------------------------------------------
        // Get the folder where to add the entity.
        // -----------------------------------------------------------------------------
        CEntityFolder& rFolder = rRegion.m_EntityFolders[_rEntity.GetCategory()];

        // -----------------------------------------------------------------------------
        // Push the entity to the front of the folder.
        // -----------------------------------------------------------------------------
        _rEntity.SetFolder  (&rFolder);
        _rEntity.SetPrevious(nullptr);
        _rEntity.SetNext    (rFolder.m_pEntities);

        if (rFolder.m_pEntities != nullptr)
        {
            rFolder.m_pEntities->SetPrevious(&_rEntity);
        }

        rFolder.m_pEntities = &_rEntity;
        
        // -----------------------------------------------------------------------------
        // Increase entity counter
        // -----------------------------------------------------------------------------
        ++ m_NumberOfEntities;
    }

    // -----------------------------------------------------------------------------

    void CDtLvlMap::MoveEntity(CEntity& _rEntity)
    {
        CRegion* pOldRegion;
        CRegion* pNewRegion;

        assert((_rEntity.GetFolder() != nullptr) && (_rEntity.GetFolder()->m_pRegion != nullptr));

        pOldRegion = _rEntity.GetFolder()->m_pRegion;
        pNewRegion = &GetRegionByPosition(_rEntity.GetWorldPosition());

        if (pNewRegion != pOldRegion)
        {
            // -----------------------------------------------------------------------------
            // Remove the entity from the old region.
            // -----------------------------------------------------------------------------
            if (_rEntity.GetPrevious() != nullptr)
            {
                _rEntity.GetPrevious()->SetNext(_rEntity.GetNext());
            }
            else
            {
                _rEntity.GetFolder()->m_pEntities = _rEntity.GetNext();
            }

            if (_rEntity.GetNext() != nullptr)
            {
                _rEntity.GetNext()->SetPrevious(_rEntity.GetPrevious());
            }

            // -----------------------------------------------------------------------------
            // Add the entity to the new region.
            // -----------------------------------------------------------------------------
            CEntityFolder& rFolder = pNewRegion->m_EntityFolders[_rEntity.GetCategory()];

            // -----------------------------------------------------------------------------
            // Push the entity to the front of the folder.
            // -----------------------------------------------------------------------------
            _rEntity.SetFolder  (&rFolder);
            _rEntity.SetPrevious(nullptr);
            _rEntity.SetNext    (rFolder.m_pEntities);

            if (rFolder.m_pEntities != nullptr)
            {
                rFolder.m_pEntities->SetPrevious(&_rEntity);
            }

            rFolder.m_pEntities = &_rEntity;
        }
    }

    // -----------------------------------------------------------------------------

    void CDtLvlMap::RemoveEntity(CEntity& _rEntity)
    {
        assert(_rEntity.GetFolder() != nullptr);

        if (_rEntity.GetPrevious() != nullptr)
        {
            _rEntity.GetPrevious()->SetNext(_rEntity.GetNext());
        }
        else
        {
            _rEntity.GetFolder()->m_pEntities = _rEntity.GetNext();
        }

        if (_rEntity.GetNext() != nullptr)
        {
            _rEntity.GetNext()->SetPrevious(_rEntity.GetPrevious());
        }

        _rEntity.SetFolder(0);
        
        // -----------------------------------------------------------------------------
        // Decrease entity counter
        // -----------------------------------------------------------------------------
        -- m_NumberOfEntities;
    }

    // -----------------------------------------------------------------------------

    CEntityIterator CDtLvlMap::EntitiesBegin() const
    {
        return CInternEntityIterator(GetFirstEntity());
    }

    // -----------------------------------------------------------------------------

    CEntityIterator CDtLvlMap::EntitiesBegin(unsigned int _Category) const
    {
        return CInternEntityIterator(GetFirstEntity(_Category));
    }

    // -----------------------------------------------------------------------------

    CEntityIterator CDtLvlMap::EntitiesBegin(const Base::AABB3Float& _rAABB) const
    {
        return CInternEntityIterator(GetFirstEntity(_rAABB));
    }

    // -----------------------------------------------------------------------------

    CEntityIterator CDtLvlMap::EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB) const
    {
        return CInternEntityIterator(GetFirstEntity(_Category, _rAABB));
    }

    // -----------------------------------------------------------------------------

    CEntityIterator CDtLvlMap::EntitiesBegin(const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const
    {
        return CInternEntityIterator(GetFirstEntity(_rAABB, _rPosition, _SquareRadius));
    }

    // -----------------------------------------------------------------------------

    CEntityIterator CDtLvlMap::EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const
    {
        return CInternEntityIterator(GetFirstEntity(_Category, _rAABB, _rPosition, _SquareRadius));
    }

    // -----------------------------------------------------------------------------

    CEntityIterator CDtLvlMap::EntitiesEnd() const
    {
        return CInternEntityIterator(nullptr);
    }
    
    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetFirstEntity() const
    {
        int      Category;
        CRegion* pRegion;
        
        // -----------------------------------------------------------------------------
        // We have to find the first non-empty folder.
        // -----------------------------------------------------------------------------
        const CRegion* const pRegionEnd = m_pRegions + m_NumberOfRegions;

        for (pRegion = m_pRegions; pRegion < pRegionEnd; ++ pRegion)
        {
            for (Category = 0; Category < Dt::SEntityCategory::NumberOfCategories; ++ Category)
            {
                CEntityFolder& rFolder = pRegion->m_EntityFolders[Category];

                if (rFolder.m_pEntities != nullptr)
                {
                    return rFolder.m_pEntities;
                }
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetFirstEntity(unsigned int _Category) const
    {
        CRegion* pRegion;

        // -----------------------------------------------------------------------------
        // We have to find the first non-empty folder with the passed category.
        // -----------------------------------------------------------------------------
        const CRegion* const pRegionEnd = m_pRegions + m_NumberOfRegions;

        for (pRegion = m_pRegions; pRegion < pRegionEnd; ++ pRegion)
        {
            CEntityFolder& rFolder = pRegion->m_EntityFolders[_Category];

            if (rFolder.m_pEntities != nullptr)
            {
                return rFolder.m_pEntities;
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetFirstEntity(const Base::AABB3Float& _rAABB) const
    {
        int        Category;
        Base::Size RegionX;
        Base::Size RegionY;
        Base::Size RegionID;
        CRegion*   pRegion;

        assert((m_pRegions != nullptr) && (IsValid(_rAABB)));

        // -----------------------------------------------------------------------------
        // We have to find the first non-empty folder within the passed AABB.
        // -----------------------------------------------------------------------------
        pRegion = &GetRegionByPosition(_rAABB[0]);

        for (;;)
        {
            assert((pRegion != nullptr) && (pRegion->m_ID < m_NumberOfRegions));

            for (Category = 0; Category < Dt::SEntityCategory::NumberOfCategories; ++ Category)
            {
                CEntityFolder& rFolder = pRegion->m_EntityFolders[Category];

                if (rFolder.m_pEntities != nullptr)
                {
                    return rFolder.m_pEntities;
                }
            }

            // -----------------------------------------------------------------------------
            // Search in the next region.
            // -----------------------------------------------------------------------------
            if ((pRegion->m_Position[0] + CRegion::s_NumberOfMetersX) < _rAABB[1][0])
            {
                // -----------------------------------------------------------------------------
                // The current row of the AABB is not finished so return the right region.
                // -----------------------------------------------------------------------------
                ++ pRegion;
            }
            else if ((pRegion->m_Position[1] + CRegion::s_NumberOfMetersY) < _rAABB[1][1])
            {
                // -----------------------------------------------------------------------------
                // The current row is finished so go to the beginning of the next upper row.
                // -----------------------------------------------------------------------------
                RegionX  = static_cast<Base::Size>(_rAABB[0][0]) / CRegion::s_NumberOfMetersX;
                RegionY  = pRegion->m_Position[1] / CRegion::s_NumberOfMetersY + 1;
                RegionID = RegionX + RegionY * m_NumberOfRegionsX;

                pRegion  = &m_pRegions[RegionID];
            }
            else
            {
                // -----------------------------------------------------------------------------
                // We have reached the end of the regions.
                // -----------------------------------------------------------------------------
                break;
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetFirstEntity(unsigned int _Category, const Base::AABB3Float& _rAABB) const
    {
        Base::Size RegionX;
        Base::Size RegionY;
        Base::Size RegionID;
        CRegion*   pRegion;

        assert((m_pRegions != nullptr) && (IsValid(_rAABB)));

        // -----------------------------------------------------------------------------
        // We have to find the first non-empty folder within the passed AABB.
        // -----------------------------------------------------------------------------
        pRegion = &GetRegionByPosition(_rAABB[0]);

        for (;;)
        {
            assert((pRegion != nullptr) && (pRegion->m_ID < m_NumberOfRegions));

            CEntityFolder& rFolder = pRegion->m_EntityFolders[_Category];

            if (rFolder.m_pEntities != nullptr)
            {
                return rFolder.m_pEntities;
            }

            // -----------------------------------------------------------------------------
            // Search in the next region.
            // -----------------------------------------------------------------------------
            if ((pRegion->m_Position[0] + CRegion::s_NumberOfMetersX) < _rAABB[1][0])
            {
                // -----------------------------------------------------------------------------
                // The current row of the AABB is not finished so return the right region.
                // -----------------------------------------------------------------------------
                ++ pRegion;
            }
            else if ((pRegion->m_Position[1] + CRegion::s_NumberOfMetersY) < _rAABB[1][1])
            {
                // -----------------------------------------------------------------------------
                // The current row is finished so go to the beginning of the next upper row.
                // -----------------------------------------------------------------------------
                RegionX  = static_cast<Base::Size>(_rAABB[0][0]) / CRegion::s_NumberOfMetersX;
                RegionY  = pRegion->m_Position[1] / CRegion::s_NumberOfMetersY + 1;
                RegionID = RegionX + RegionY * m_NumberOfRegionsX;

                pRegion  = &m_pRegions[RegionID];
            }
            else
            {
                // -----------------------------------------------------------------------------
                // We have reached the end of the regions.
                // -----------------------------------------------------------------------------
                break;
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetFirstEntity(const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const
    {
        const CEntityIterator EndEntity = EntitiesEnd();

        for (CEntityIterator FirstEntity = EntitiesBegin(_rAABB); FirstEntity != EndEntity; FirstEntity = FirstEntity.Next(_rAABB))
        {
            glm::vec3 Delta = FirstEntity->GetWorldPosition() - _rPosition;

            if (Delta.length() * Delta.length() <= _SquareRadius)
            {
                return &(*FirstEntity);
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetFirstEntity(unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const
    {
        const CEntityIterator EndEntity = EntitiesEnd();

        for (CEntityIterator FirstEntity = EntitiesBegin(_Category, _rAABB); FirstEntity != EndEntity; FirstEntity = FirstEntity.Next(_Category, _rAABB))
        {
            glm::vec3 Delta = FirstEntity->GetWorldPosition() - _rPosition;

            if (Delta.length() * Delta.length() <= _SquareRadius)
            {
                return &(*FirstEntity);
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetNextEntity(CEntity* _pEntity) const
    {
        int      Category;
        CRegion* pRegion;

        assert((m_pRegions != nullptr) && (_pEntity != nullptr));

        // -----------------------------------------------------------------------------
        // Check if we have reached the end of the current folder.
        // -----------------------------------------------------------------------------
        if (_pEntity->GetNext() != nullptr)
        {
            return _pEntity->GetNext();
        }

        // -----------------------------------------------------------------------------
        // Check if there is a non-empty folder in the current region.
        // -----------------------------------------------------------------------------
        assert((_pEntity->GetFolder() != nullptr) && (_pEntity->GetFolder()->m_pRegion != nullptr));

        pRegion = _pEntity->GetFolder()->m_pRegion;

        for (Category = _pEntity->GetFolder()->m_Category + 1; Category < Dt::SEntityCategory::NumberOfCategories; ++ Category)
        {
            CEntityFolder& rFolder = pRegion->m_EntityFolders[Category];

            if (rFolder.m_pEntities != nullptr)
            {
                return rFolder.m_pEntities;
            }
        }

        // -----------------------------------------------------------------------------
        // We have to search the next region.
        // -----------------------------------------------------------------------------
        const CRegion* const pRegionEnd = m_pRegions + m_NumberOfRegions;

        for (pRegion = pRegion + 1; pRegion < pRegionEnd; ++ pRegion)
        {
            for (Category = 0; Category < Dt::SEntityCategory::NumberOfCategories; ++ Category)
            {
                CEntityFolder& rFolder = pRegion->m_EntityFolders[Category];

                if (rFolder.m_pEntities != nullptr)
                {
                    return rFolder.m_pEntities;
                }
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetNextEntity(CEntity* _pEntity, unsigned int _Category) const
    {
        CRegion* pRegion;

        assert((m_pRegions != nullptr) && (_pEntity != nullptr));

        // -----------------------------------------------------------------------------
        // Check if we have reached the end of the current folder.
        // -----------------------------------------------------------------------------
        if (_pEntity->GetNext() != nullptr)
        {
            return _pEntity->GetNext();
        }

        // -----------------------------------------------------------------------------
        // We have to search the next region.
        // -----------------------------------------------------------------------------
        assert((_pEntity->GetFolder() != nullptr) && (_pEntity->GetFolder()->m_pRegion != nullptr));

        const CRegion* const pRegionEnd = m_pRegions + m_NumberOfRegions;

        for (pRegion = _pEntity->GetFolder()->m_pRegion + 1; pRegion < pRegionEnd; ++ pRegion)
        {
            CEntityFolder& rFolder = pRegion->m_EntityFolders[_Category];

            if (rFolder.m_pEntities != nullptr)
            {
                return rFolder.m_pEntities;
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetNextEntity(CEntity* _pEntity, const Base::AABB3Float& _rAABB) const
    {
        int        Category;
        Base::Size RegionX;
        Base::Size RegionY;
        Base::Size RegionID;
        CRegion*   pRegion;

        assert((m_pRegions != nullptr) && (_pEntity != nullptr) && IsValid(_rAABB));

        // -----------------------------------------------------------------------------
        // Check if we have reached the end of the current folder.
        // -----------------------------------------------------------------------------
        if (_pEntity->GetNext() != nullptr)
        {
            return _pEntity->GetNext();
        }

        // -----------------------------------------------------------------------------
        // Check if there is a non-empty folder in the current region.
        // -----------------------------------------------------------------------------
        assert((_pEntity->GetFolder() != nullptr) && (_pEntity->GetFolder()->m_pRegion != nullptr));

        pRegion = _pEntity->GetFolder()->m_pRegion;

        for (Category = _pEntity->GetFolder()->m_Category + 1; Category < Dt::SEntityCategory::NumberOfCategories; ++ Category)
        {
            CEntityFolder& rFolder = pRegion->m_EntityFolders[Category];

            if (rFolder.m_pEntities != nullptr)
            {
                return rFolder.m_pEntities;
            }
        }

        // -----------------------------------------------------------------------------
        // We have to search the next region.
        // -----------------------------------------------------------------------------
        for (;;)
        {
            if ((pRegion->m_Position[0] + CRegion::s_NumberOfMetersX) < _rAABB[1][0])
            {
                // -----------------------------------------------------------------------------
                // The current row of the AABB is not finished so return the right region.
                // -----------------------------------------------------------------------------
                ++ pRegion;
            }
            else if ((pRegion->m_Position[1] + CRegion::s_NumberOfMetersY) < _rAABB[1][1])
            {
                // -----------------------------------------------------------------------------
                // The current row is finished so go to the beginning of the next upper row.
                // -----------------------------------------------------------------------------
                RegionX  = static_cast<Base::Size>(_rAABB[0][0]) / CRegion::s_NumberOfMetersX;
                RegionY  = pRegion->m_Position[1] / CRegion::s_NumberOfMetersY + 1;
                RegionID = RegionX + RegionY * m_NumberOfRegionsX;

                pRegion  = &m_pRegions[RegionID];
            }
            else
            {
                // -----------------------------------------------------------------------------
                // We have reached the end of the regions.
                // -----------------------------------------------------------------------------
                break;
            }

            // -----------------------------------------------------------------------------
            // Search in the next region.
            // -----------------------------------------------------------------------------
            assert((pRegion != nullptr) && (pRegion->m_ID < m_NumberOfRegions));

            for (Category = 0; Category < Dt::SEntityCategory::NumberOfCategories; ++ Category)
            {
                CEntityFolder& rFolder = pRegion->m_EntityFolders[Category];

                if (rFolder.m_pEntities != nullptr)
                {
                    return rFolder.m_pEntities;
                }
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetNextEntity(CEntity* _pEntity, unsigned int _Category, const Base::AABB3Float& _rAABB) const
    {
        Base::Size RegionX;
        Base::Size RegionY;
        Base::Size RegionID;
        CRegion*   pRegion;

        assert((m_pRegions != nullptr) && (_pEntity != nullptr) && IsValid(_rAABB));

        // -----------------------------------------------------------------------------
        // Check if we have reached the end of the current folder.
        // -----------------------------------------------------------------------------
        if (_pEntity->GetNext() != nullptr)
        {
            return _pEntity->GetNext();
        }

        // -----------------------------------------------------------------------------
        // We have to search the next region.
        // -----------------------------------------------------------------------------
        assert((_pEntity->GetFolder() != nullptr) && (_pEntity->GetFolder()->m_pRegion != nullptr));

        pRegion = _pEntity->GetFolder()->m_pRegion;

        for (;;)
        {
            if ((pRegion->m_Position[0] + CRegion::s_NumberOfMetersX) < _rAABB[1][0])
            {
                // -----------------------------------------------------------------------------
                // The current row of the AABB is not finished so return the right region.
                // -----------------------------------------------------------------------------
                ++ pRegion;
            }
            else if ((pRegion->m_Position[1] + CRegion::s_NumberOfMetersY) < _rAABB[1][1])
            {
                // -----------------------------------------------------------------------------
                // The current row is finished so go to the beginning of the next upper row.
                // -----------------------------------------------------------------------------
                RegionX  = static_cast<Base::Size>(_rAABB[0][0]) / CRegion::s_NumberOfMetersX;
                RegionY  = pRegion->m_Position[1] / CRegion::s_NumberOfMetersY + 1;
                RegionID = RegionX + RegionY * m_NumberOfRegionsX;

                pRegion  = &m_pRegions[RegionID];
            }
            else
            {
                // -----------------------------------------------------------------------------
                // We have reached the end of the regions.
                // -----------------------------------------------------------------------------
                break;
            }

            // -----------------------------------------------------------------------------
            // Search in the next region.
            // -----------------------------------------------------------------------------
            assert((pRegion != nullptr) && (pRegion->m_ID < m_NumberOfRegions));

            CEntityFolder& rFolder = pRegion->m_EntityFolders[_Category];

            if (rFolder.m_pEntities != nullptr)
            {
                return rFolder.m_pEntities;
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetNextEntity(CEntity* _pEntity, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const
    {
        const CInternEntityIterator EntityEndIterator = EntitiesEnd();

        for (CInternEntityIterator EntityIterator = GetNextEntity(_pEntity, _rAABB); EntityIterator != EntityEndIterator; EntityIterator = EntityIterator.Next(_rAABB))
        {
            glm::vec3 Delta = EntityIterator->GetWorldPosition() - _rPosition;

            if (Delta.length() * Delta.length() <= _SquareRadius)
            {
                return &(*EntityIterator);
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CEntity* CDtLvlMap::GetNextEntity(CEntity* _pEntity, unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius) const
    {
        const CInternEntityIterator EntityEndIterator = EntitiesEnd();

        for (CInternEntityIterator EntityIterator = GetNextEntity(_pEntity, _Category, _rAABB); EntityIterator != EntityEndIterator; EntityIterator = EntityIterator.Next(_Category, _rAABB))
        {
            glm::vec3 Delta = EntityIterator->GetWorldPosition() - _rPosition;

            if (Delta.length() * Delta.length() <= _SquareRadius)
            {
                return &(*EntityIterator);
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    bool CDtLvlMap::IsValid(const Base::AABB3Float& _rAABB) const
    {
        // -----------------------------------------------------------------------------
        // Check if the given position is valid
        // -----------------------------------------------------------------------------
        if (_rAABB[0][0] < 0.0f             ) return false;
        if (_rAABB[0][1] < 0.0f             ) return false;
        if (_rAABB[1][0] > m_NumberOfMetersX) return false;
        if (_rAABB[1][1] > m_NumberOfMetersY) return false;

        return true;
    }
} // namespace

namespace
{
    CDtLvlMap::CInternEntityIterator::CInternEntityIterator(CEntity* _pEntity)
        : CEntityIterator(_pEntity)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtLvlMap::CInternEntityIterator::CInternEntityIterator(CEntityIterator _Iterator)
        : CEntityIterator(_Iterator)
    {
    }
} // namespace

namespace Dt
{
namespace Map
{
    CEntityIterator::CEntityIterator()
        : m_pCurrentEntity(nullptr)
    {
    }

    // ----------------------------------------------------------------------------

    CEntityIterator::CEntityIterator(CEntity* _pEntity)
        : m_pCurrentEntity(_pEntity)
    {
    }

    // -----------------------------------------------------------------------------

    CEntity& CEntityIterator::operator * () const
    {
        assert(m_pCurrentEntity != nullptr);

        return *m_pCurrentEntity;
    }

    // -----------------------------------------------------------------------------

    CEntity* CEntityIterator::operator -> () const
    {
        return m_pCurrentEntity;
    }

    // ----------------------------------------------------------------------------

    CEntityIterator& CEntityIterator::Next()
    {
        m_pCurrentEntity = CDtLvlMap::GetInstance().GetNextEntity(m_pCurrentEntity);

        return *this;
    }

    // -----------------------------------------------------------------------------

    CEntityIterator& CEntityIterator::Next(unsigned int _Category)
    {
        m_pCurrentEntity = CDtLvlMap::GetInstance().GetNextEntity(m_pCurrentEntity, _Category);

        return *this;
    }

    // -----------------------------------------------------------------------------

    CEntityIterator& CEntityIterator::Next(const Base::AABB3Float& _rAABB)
    {
        m_pCurrentEntity = CDtLvlMap::GetInstance().GetNextEntity(m_pCurrentEntity, _rAABB);

        return *this;
    }

    // -----------------------------------------------------------------------------

    CEntityIterator& CEntityIterator::Next(unsigned int _Category, const Base::AABB3Float& _rAABB)
    {
        m_pCurrentEntity = CDtLvlMap::GetInstance().GetNextEntity(m_pCurrentEntity, _Category, _rAABB);

        return *this;
    }

    // -----------------------------------------------------------------------------

    CEntityIterator& CEntityIterator::Next(const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius)
    {
        m_pCurrentEntity = CDtLvlMap::GetInstance().GetNextEntity(m_pCurrentEntity, _rAABB, _rPosition, _SquareRadius);

        return *this;
    }

    // -----------------------------------------------------------------------------

    CEntityIterator& CEntityIterator::Next(unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius)
    {
        m_pCurrentEntity = CDtLvlMap::GetInstance().GetNextEntity(m_pCurrentEntity, _Category, _rAABB, _rPosition, _SquareRadius);

        return *this;
    }

    // -----------------------------------------------------------------------------

    bool CEntityIterator::operator == (const CEntityIterator& _rOther) const
    {
        return m_pCurrentEntity == _rOther.m_pCurrentEntity;
    }

    // -----------------------------------------------------------------------------

    bool CEntityIterator::operator != (const CEntityIterator& _rOther) const
    {
        return m_pCurrentEntity != _rOther.m_pCurrentEntity;
    }
} // namespace Map
} // namespace Dt

namespace Dt
{
namespace Map
{
    void AllocateMap(Base::Size _NumberOfRegionsX, Base::Size _NumberOfRegionsY)
    {
        return CDtLvlMap::GetInstance().AllocateMap(_NumberOfRegionsX, _NumberOfRegionsY);
    }

    // -----------------------------------------------------------------------------

    void FreeMap()
    {
        CDtLvlMap::GetInstance().FreeMap();
    }

    // -----------------------------------------------------------------------------

    bool HasMap()
    {
        return CDtLvlMap::GetInstance().HasMap();
    }

    // -----------------------------------------------------------------------------

    CRegion* GetRegions()
    {
        return CDtLvlMap::GetInstance().GetRegions();
    }

    // -----------------------------------------------------------------------------

    CRegion& GetRegionByID(unsigned int _RegionID)
    {
        return CDtLvlMap::GetInstance().GetRegionByID(_RegionID);
    }

    // -----------------------------------------------------------------------------

    CRegion& GetRegionByPosition(float _AxisX, float _AxisY)
    {
        return CDtLvlMap::GetInstance().GetRegionByPosition(_AxisX, _AxisY);
    }

    // -----------------------------------------------------------------------------

    CRegion& GetRegionByPosition(const glm::vec3& _rPosition)
    {
        return CDtLvlMap::GetInstance().GetRegionByPosition(_rPosition);
    }

    // -----------------------------------------------------------------------------

    Base::Size GetNumberOfRegions()
    {
        return CDtLvlMap::GetInstance().GetNumberOfRegions();
    }

    // -----------------------------------------------------------------------------

    Base::Size GetNumberOfRegionsX()
    {
        return CDtLvlMap::GetInstance().GetNumberOfRegionsX();
    }

    // -----------------------------------------------------------------------------

    Base::Size GetNumberOfRegionsY()
    {
        return CDtLvlMap::GetInstance().GetNumberOfRegionsY();
    }

    // -----------------------------------------------------------------------------

    Base::Size GetNumberOfSquareMeters()
    {
        return CDtLvlMap::GetInstance().GetNumberOfSquareMeters();
    }

    // -----------------------------------------------------------------------------

    Base::Size GetNumberOfMetersX()
    {
        return CDtLvlMap::GetInstance().GetNumberOfMetersX();
    }

    // -----------------------------------------------------------------------------

    Base::Size GetNumberOfMetersY()
    {
        return CDtLvlMap::GetInstance().GetNumberOfMetersY();
    }

    // -----------------------------------------------------------------------------

    CEntityIterator EntitiesBegin()
    {
        return CDtLvlMap::GetInstance().EntitiesBegin();
    }

    // -----------------------------------------------------------------------------

    CEntityIterator EntitiesBegin(unsigned int _Category)
    {
        return CDtLvlMap::GetInstance().EntitiesBegin(_Category);
    }

    // -----------------------------------------------------------------------------

    CEntityIterator EntitiesBegin(const Base::AABB3Float& _rAABB)
    {
        return CDtLvlMap::GetInstance().EntitiesBegin(_rAABB);
    }

    // -----------------------------------------------------------------------------

    CEntityIterator EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB)
    {
        return CDtLvlMap::GetInstance().EntitiesBegin(_Category, _rAABB);
    }

    // -----------------------------------------------------------------------------

    CEntityIterator EntitiesBegin(const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius)
    {
        return CDtLvlMap::GetInstance().EntitiesBegin(_rAABB, _rPosition, _SquareRadius);
    }

    // -----------------------------------------------------------------------------

    CEntityIterator EntitiesBegin(unsigned int _Category, const Base::AABB3Float& _rAABB, const glm::vec3& _rPosition, float _SquareRadius)
    {
        return CDtLvlMap::GetInstance().EntitiesBegin(_Category, _rAABB, _rPosition, _SquareRadius);
    }

    // -----------------------------------------------------------------------------

    CEntityIterator EntitiesEnd()
    {
        return CDtLvlMap::GetInstance().EntitiesEnd();
    }

    // -----------------------------------------------------------------------------

    void AddEntity(CEntity& _rEntity)
    {
        CDtLvlMap::GetInstance().AddEntity(_rEntity);
    }
    
    // -----------------------------------------------------------------------------
    
    void MoveEntity(CEntity& _rEntity)
    {
        CDtLvlMap::GetInstance().MoveEntity(_rEntity);
    }
    
    // -----------------------------------------------------------------------------
    
    void RemoveEntity(CEntity& _rEntity)
    {
        CDtLvlMap::GetInstance().RemoveEntity(_rEntity);
    }
} // namespace Map
} // namespace Dt
