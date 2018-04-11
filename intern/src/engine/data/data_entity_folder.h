
#pragma once

namespace Dt
{
    class CEntity;
    class CRegion;
} // namespace Dt


namespace Dt
{
    class CEntityFolder
    {
    public:

        inline CEntityFolder();
        inline ~CEntityFolder();


    public:

        unsigned int m_Category;
        CRegion*     m_pRegion;
        CEntity*     m_pEntities;
        unsigned int m_LastModification;

    public:

        CEntityFolder* m_pNext;
    };
} // namespace Dt

namespace Dt
{
	inline CEntityFolder::CEntityFolder()
        : m_Category        (0)
        , m_pRegion         (0)
        , m_pEntities       (0)
        , m_LastModification(0)
        , m_pNext           (0)
    {

    }

    // -----------------------------------------------------------------------------

    inline CEntityFolder::~CEntityFolder()
    {
        m_pRegion          = 0;
        m_pEntities        = 0;
        m_LastModification = 0;
        m_pNext            = 0;
    }
} // namespace Dt