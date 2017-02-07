
#include "data/data_precompiled.h"

#include "data/data_entity.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_transformation_facet.h"

namespace Dt
{
    CEntity::CEntity()
        : m_pNextNeighbor       (this)
        , m_pPreviousNeighbor   (this)
        , m_pFolder             (nullptr)
        , m_pHierarchyFacet     (nullptr)
        , m_pTransformationFacet(nullptr)
        , m_ID                  (s_InvalidID)
        , m_Name                ()
        , m_WorldAABB           ()
        , m_WorldPosition       ()
        , m_Flags               ()
    {
        m_pDetailFacets[SFacetCategory::Data]    = 0;
        m_pDetailFacets[SFacetCategory::Graphic] = 0;
        m_pDetailFacets[SFacetCategory::Script]  = 0;

        m_Flags.m_Key = 0;
    }

    // -----------------------------------------------------------------------------

    CEntity::~CEntity()
    {
        m_Name.clear();
    }

    // -----------------------------------------------------------------------------

    CEntity::BID CEntity::GetID() const
    {
        return m_ID;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetCategory(unsigned int _Category)
    {
        m_Flags.m_Category = _Category;
    }

    // -----------------------------------------------------------------------------

    unsigned int CEntity::GetCategory() const
    {
        return m_Flags.m_Category;
    }

    // -----------------------------------------------------------------------------


    void CEntity::SetName(const std::string& _rName)
    {
        m_Name = _rName;
    }

    // -----------------------------------------------------------------------------

    const std::string& CEntity::GetName() const
    {
        return m_Name;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetDynamic(bool _Flag)
    {
        m_Flags.m_IsDynamic = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CEntity::IsDynamic() const
    {
        return m_Flags.m_IsDynamic == true;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetSelectable(bool _Flag)
    {
        m_Flags.m_IsSelectable = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CEntity::IsSelectable() const
    {
        return m_Flags.m_IsSelectable == true;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetLayer(unsigned int _Layer)
    {
        m_Flags.m_Layer = _Layer;
    }

    // -----------------------------------------------------------------------------

    unsigned int CEntity::GetLayer() const
    {
        return m_Flags.m_Layer;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetDirtyFlags(unsigned int _Flags)
    {
        m_Flags.m_DirtyFlags = _Flags;
    }

    // -----------------------------------------------------------------------------

    unsigned int CEntity::GetDirtyFlags() const
    {
        return m_Flags.m_DirtyFlags;
    }

    // -----------------------------------------------------------------------------

    bool CEntity::IsInMap() const
    {
        return m_pFolder != nullptr;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetType(unsigned int _Type)
    {
        m_Flags.m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    unsigned int CEntity::GetType() const
    {
        return m_Flags.m_Type;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetWorldPosition(const Base::Float3& _rPosition)
    {
        m_WorldPosition = _rPosition;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CEntity::GetWorldPosition()
    {
        return m_WorldPosition;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CEntity::GetWorldPosition() const
    {
        return m_WorldPosition;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetWorldAABB(const Base::AABB3Float& _rAABB)
    {
        m_WorldAABB = _rAABB;
    }

    // -----------------------------------------------------------------------------

    Base::AABB3Float& CEntity::GetWorldAABB()
    {
        return m_WorldAABB;
    }

    // -----------------------------------------------------------------------------

    const Base::AABB3Float& CEntity::GetWorldAABB() const
    {
        return m_WorldAABB;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetFolder(CEntityFolder* _pFolder)
    {
        m_pFolder = _pFolder;
    }

    // -----------------------------------------------------------------------------

    CEntityFolder* CEntity::GetFolder()
    {
        return m_pFolder;
    }

    // -----------------------------------------------------------------------------

    const CEntityFolder* CEntity::GetFolder() const
    {
        return m_pFolder;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetNext(CEntity* _pLink)
    {
        m_pNextNeighbor = _pLink;
    }

    // -----------------------------------------------------------------------------

    CEntity* CEntity::GetNext()
    {
        return m_pNextNeighbor;
    }

    // -----------------------------------------------------------------------------

    const CEntity* CEntity::GetNext() const
    {
        return m_pNextNeighbor;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetPrevious(CEntity* _pLink)
    {
        m_pPreviousNeighbor = _pLink;
    }

    // -----------------------------------------------------------------------------

    CEntity* CEntity::GetPrevious()
    {
        return m_pPreviousNeighbor;
    }

    // -----------------------------------------------------------------------------

    const CEntity* CEntity::GetPrevious() const
    {
        return m_pPreviousNeighbor;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetHierarchyFacet(CHierarchyFacet* _pFacet)
    {
        m_pHierarchyFacet = _pFacet;
    }

    // -----------------------------------------------------------------------------

    CHierarchyFacet* CEntity::GetHierarchyFacet()
    {
        return m_pHierarchyFacet;
    }

    // -----------------------------------------------------------------------------

    const CHierarchyFacet* CEntity::GetHierarchyFacet() const
    {
        return m_pHierarchyFacet;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetTransformationFacet(CTransformationFacet* _pFacet)
    {
        m_pTransformationFacet = _pFacet;
    }

    // -----------------------------------------------------------------------------

    CTransformationFacet* CEntity::GetTransformationFacet()
    {
        return m_pTransformationFacet;
    }

    // -----------------------------------------------------------------------------

    const CTransformationFacet* CEntity::GetTransformationFacet() const
    {
        return m_pTransformationFacet;
    }

    // -----------------------------------------------------------------------------

    void CEntity::SetDetailFacet(unsigned int _Category, void* _pFacet)
    {
        m_pDetailFacets[_Category] = _pFacet;
    }

    // -----------------------------------------------------------------------------

    void* CEntity::GetDetailFacet(unsigned int _Category)
    {
        return m_pDetailFacets[_Category];
    }

    // -----------------------------------------------------------------------------

    const void* CEntity::GetDetailFacet(unsigned int _Category) const
    {
        return m_pDetailFacets[_Category];
    }

    // -----------------------------------------------------------------------------

    void CEntity::Attach(CEntity& _rEntity)
    {
        CEntity*         pFirstChild;
        CHierarchyFacet* pChildHierarchyFacet;

        pChildHierarchyFacet = _rEntity.GetHierarchyFacet();

        if (m_pHierarchyFacet == nullptr || pChildHierarchyFacet == nullptr)
        {
            return;
        }

        pChildHierarchyFacet->SetParent(this);

        pFirstChild = m_pHierarchyFacet->GetFirstChild();

        if (pFirstChild != nullptr)
        {
            pChildHierarchyFacet->SetSibling(pFirstChild);
        }

        m_pHierarchyFacet->SetFirstChild(&_rEntity);

        // -----------------------------------------------------------------------------
        // Transformation
        // -----------------------------------------------------------------------------
        Dt::CTransformationFacet* pChildTransformationFacet;

        pChildTransformationFacet = _rEntity.GetTransformationFacet();

        if (m_pTransformationFacet == nullptr || pChildTransformationFacet == nullptr)
        {
            return;
        }

        Base::Float4x4 NewMatrix;

        Base::Float3 Translation;
        Base::Float3 Rotation;
        Base::Float3 Scale;

        NewMatrix = m_pTransformationFacet->GetWorldMatrix().GetInverted() * pChildTransformationFacet->GetWorldMatrix();

        NewMatrix.GetTranslation(Translation);
        NewMatrix.GetRotation(Rotation);

        Rotation *= Base::Float3(-1, 1, 1);

        NewMatrix.GetScale(Scale);

        pChildTransformationFacet->SetPosition(Translation);
        pChildTransformationFacet->SetRotation(Rotation);
        pChildTransformationFacet->SetScale(Scale);
    }

    // -----------------------------------------------------------------------------

    void CEntity::Detach()
    {
        CEntity*         pParent;
        CHierarchyFacet* pParentHierarchyFacet;

        pParent = m_pHierarchyFacet->GetParent();

        if (pParent == nullptr)
        {
            return;
        }

        pParentHierarchyFacet = pParent->GetHierarchyFacet();

        pParentHierarchyFacet->SetFirstChild(m_pHierarchyFacet->GetSibling());

        m_pHierarchyFacet->SetParent(nullptr);

        m_pHierarchyFacet->SetSibling(nullptr);

        // -----------------------------------------------------------------------------
        // Transformation
        // -----------------------------------------------------------------------------
        Dt::CTransformationFacet* pParentTransformationFacet;

        pParentTransformationFacet = pParent->GetTransformationFacet();

        if (m_pTransformationFacet == nullptr || pParentTransformationFacet == nullptr)
        {
            return;
        }

        Base::Float4x4 NewMatrix;
        Base::Float4x4 RotationMatrix;

        Base::Float3 Translation;
        Base::Float3 Rotation;
        Base::Float3 Scale;

        NewMatrix = m_pTransformationFacet->GetWorldMatrix();

        NewMatrix.GetTranslation(Translation);

        NewMatrix.GetRotation(Rotation);

        // TODO by tschwandt
        // is it necessary to do this?
        Rotation *= Base::Float3(-1, 1, 1);

        RotationMatrix.SetRotation(Rotation[0], Rotation[1], Rotation[2]);

        NewMatrix = RotationMatrix.GetInverted() * NewMatrix;

        NewMatrix.GetScale(Scale);

        m_pTransformationFacet->SetPosition(Translation);
        m_pTransformationFacet->SetRotation(Rotation);
        m_pTransformationFacet->SetScale(Scale);
    }
} // namespace Dt