
#include "data/data_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_script_facet.h"
#include "data/data_script_manager.h"

#include <assert.h>
#include <vector>

using namespace Dt;
using namespace Dt::ScriptManager;

namespace
{
    class CDtScriptManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtScriptManager);
        
    public:
        
        CDtScriptManager();
        ~CDtScriptManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();

        CScriptFacet* CreateScript();

        void Update();
        
    private:
        
        class CInternScriptFacet : public CScriptFacet
        {
        private:
            friend class CDtScriptManager;
        };
        
    private:
        
        typedef Base::CPool<CInternScriptFacet, 2048> CScriptFacetPool;

    private:
        
        CScriptFacetPool m_ScriptFacets;
    };
} // namespace

namespace
{
    CDtScriptManager::CDtScriptManager()
        : m_ScriptFacets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtScriptManager::~CDtScriptManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtScriptManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtScriptManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtScriptManager::Clear()
    {
        m_ScriptFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    CScriptFacet* CDtScriptManager::CreateScript()
    {
        CInternScriptFacet& rFacet = m_ScriptFacets.Allocate();

        return &rFacet;
    }

    // -----------------------------------------------------------------------------

    void CDtScriptManager::Update()
    {

    }
} // namespace

namespace Dt
{
namespace ScriptManager
{
    void OnStart()
    {
        CDtScriptManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtScriptManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtScriptManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CScriptFacet* CreateScript()
    {
        return CDtScriptManager::GetInstance().CreateScript();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    	CDtScriptManager::GetInstance().Update();
    }
} // namespace ScriptManager
} // namespace Dt
