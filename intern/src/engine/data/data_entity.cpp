
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_hierarchy_facet.h"
#include "engine/data/data_transformation_facet.h"

namespace Dt
{
    CEntity::CEntity()
        : m_pNextNeighbor       (this)
        , m_pPreviousNeighbor   (this)
        , m_pFolder             (nullptr)
        , m_pHierarchyFacet     (nullptr)
        , m_pTransformationFacet(nullptr)
        , m_pComponentsFacet    (nullptr)
        , m_ID                  (s_InvalidID)
        , m_Name                ()
        , m_WorldAABB           ()
        , m_WorldPosition       ()
        , m_Flags               ()
    {
        m_Flags.m_Key   = 0;
        m_Flags.m_Layer = SEntityLayer::Default;

        m_Flags.m_IsDynamic    = true;
        m_Flags.m_IsSelectable = true;
        m_Flags.m_IsActive     = true;
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

    void CEntity::SetActive(bool _Flag)
    {
        m_Flags.m_IsActive = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CEntity::IsActive() const
    {
        return m_Flags.m_IsActive == true;
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

    void CEntity::SetWorldPosition(const glm::vec3& _rPosition)
    {
        m_WorldPosition = _rPosition;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CEntity::GetWorldPosition()
    {
        return m_WorldPosition;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CEntity::GetWorldPosition() const
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

    void CEntity::SetComponentFacet(CComponentFacet* _pFacet)
    {
        m_pComponentsFacet = _pFacet;
    }

    // -----------------------------------------------------------------------------

    CComponentFacet* CEntity::GetComponentFacet()
    {
        return m_pComponentsFacet;
    }

    // -----------------------------------------------------------------------------

    const CComponentFacet* CEntity::GetComponentFacet() const
    {
        return m_pComponentsFacet;
    }

    // -----------------------------------------------------------------------------

    void CEntity::Attach(CEntity& _rEntity)
    {
        auto pChildHierarchyFacet = _rEntity.GetHierarchyFacet();

        if (m_pHierarchyFacet == nullptr || pChildHierarchyFacet == nullptr)
        {
            return;
        }

        pChildHierarchyFacet->SetParent(this);

        auto pFirstChild = m_pHierarchyFacet->GetFirstChild();

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

        glm::mat4 NewMatrix;

        glm::vec3 Translation;
        glm::quat Rotation;
        glm::vec3 Scale;
        glm::vec3 Skew;
        glm::vec4 Perspective;

        NewMatrix = glm::inverse(m_pTransformationFacet->GetWorldMatrix()) * pChildTransformationFacet->GetWorldMatrix();

        glm::decompose(NewMatrix, Scale, Rotation, Translation, Skew, Perspective);

        pChildTransformationFacet->SetPosition(Translation);
        pChildTransformationFacet->SetRotation(glm::eulerAngles(Rotation));
        pChildTransformationFacet->SetScale(Scale);
    }

    // -----------------------------------------------------------------------------

    void CEntity::Detach()
    {
        auto pParent = m_pHierarchyFacet->GetParent();

        if (pParent == nullptr) return;

        auto pParentHierarchyFacet = pParent->GetHierarchyFacet();

        pParentHierarchyFacet->SetFirstChild(m_pHierarchyFacet->GetSibling());

        m_pHierarchyFacet->SetParent(nullptr);

        m_pHierarchyFacet->SetSibling(nullptr);

        // -----------------------------------------------------------------------------
        // Transformation
        // -----------------------------------------------------------------------------
        auto pParentTransformationFacet = pParent->GetTransformationFacet();

        if (m_pTransformationFacet == nullptr || pParentTransformationFacet == nullptr) return;

        glm::vec3 Translation;
        glm::quat Rotation;
        glm::vec3 Scale;
        glm::vec3 Skew;
        glm::vec4 Perspective;

        glm::mat4 NewMatrix = m_pTransformationFacet->GetWorldMatrix();

        glm::decompose(NewMatrix, Scale, Rotation, Translation, Skew, Perspective);

        glm::mat4 RotationMatrix = glm::toMat4(Rotation);

        NewMatrix = glm::inverse(RotationMatrix) * NewMatrix;

        glm::decompose(NewMatrix, Scale, Rotation, Translation, Skew, Perspective);

        m_pTransformationFacet->SetPosition(Translation);
        m_pTransformationFacet->SetRotation(glm::eulerAngles(Rotation));
        m_pTransformationFacet->SetScale(Scale);
    }
    
    // -----------------------------------------------------------------------------

    void CEntity::AttachComponent(Dt::IComponent* _pComponent)
    {
        if (_pComponent == nullptr || _pComponent->GetHostEntity() != nullptr) return;

        _pComponent->m_pHostEntity = this;

        m_pComponentsFacet->AddComponent(_pComponent);
    }

    // -----------------------------------------------------------------------------

    void CEntity::DetachComponent(Dt::IComponent* _pComponent)
    {
        if (_pComponent == nullptr || _pComponent->GetHostEntity() != this) return;

        _pComponent->m_pHostEntity = nullptr;

        m_pComponentsFacet->RemoveComponent(_pComponent);
    }

    // -----------------------------------------------------------------------------

    void CEntity::Read(CSceneReader& _rCodec)
    {
        Base::Serialize(_rCodec, m_Name);

        _rCodec >> m_ID;
        _rCodec >> m_WorldAABB[0][0];
        _rCodec >> m_WorldAABB[0][1];
        _rCodec >> m_WorldAABB[0][2];
        _rCodec >> m_WorldAABB[1][0];
        _rCodec >> m_WorldAABB[1][1];
        _rCodec >> m_WorldAABB[1][2];

        Base::Serialize(_rCodec, m_WorldPosition);

        _rCodec >> m_Flags.m_Key;
    }

    // -----------------------------------------------------------------------------

    void CEntity::Write(CSceneWriter& _rCodec)
    {
        Base::Serialize(_rCodec, m_Name);

        _rCodec << m_ID;
        _rCodec << m_WorldAABB[0][0];
        _rCodec << m_WorldAABB[0][1];
        _rCodec << m_WorldAABB[0][2];
        _rCodec << m_WorldAABB[1][0];
        _rCodec << m_WorldAABB[1][1];
        _rCodec << m_WorldAABB[1][2];

        Base::Serialize(_rCodec, m_WorldPosition);

        _rCodec << m_Flags.m_Key;
    }

} // namespace Dt