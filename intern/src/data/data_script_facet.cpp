
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
        m_ScriptFile.clear();
    }

    // -----------------------------------------------------------------------------

    void CScriptFacet::SetScriptFile(const std::string& _rScriptFile)
    {
        m_ScriptFile = _rScriptFile;
    }

    // -----------------------------------------------------------------------------

    const std::string& CScriptFacet::GetScriptFile()
    {
        return m_ScriptFile;
    }
} // namespace Dt