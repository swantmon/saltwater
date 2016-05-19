
#include "data/data_precompiled.h"

#include "data/data_script_facet.h"

namespace Dt
{
    CScriptFacet::CScriptFacet()
        : m_ScriptFile()
    {

    }

    // -----------------------------------------------------------------------------

    CScriptFacet::~CScriptFacet()
    {
        m_ScriptFile.Clear();
    }

    // -----------------------------------------------------------------------------

    void CScriptFacet::SetScriptFile(const Base::Char* _pScriptFile)
    {
        m_ScriptFile = _pScriptFile;
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CScriptFacet::GetScriptFile()
    {
        return m_ScriptFile.GetConst();
    }
} // namespace Dt