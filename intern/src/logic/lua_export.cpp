
#include "logic/lua_export.h"
#include "logic/lua_state.h"

#include <assert.h>

using namespace LUA;

namespace
{
    class CInternStaticFunctionList : public CStaticFunctionList
    {
    public:

        void AddFunction(void* _pFunctionInfo);
        void RegisterFunctions(BState _State);
        void RegisterLibrary(BState _State, const Base::Char* _pLibraryName);
        void RegisterObject(BState _State, const Base::Char* _pObjectName);

    private:

        struct SFunctionInfo
        {
            const Base::Char* m_pFunctionName;
            FLuaCFunc         m_pLuaCFtr;
            const Base::Char* m_pDoc;
            SFunctionInfo*    m_pNext;
        };
    };
} // namespace

namespace
{
    void CInternStaticFunctionList::AddFunction(void* _pFunctionInfo)
    {
        SFunctionInfo* pFunctionInfo;

        assert(_pFunctionInfo != nullptr);

        pFunctionInfo = static_cast<SFunctionInfo*>(_pFunctionInfo);

        pFunctionInfo->m_pNext = static_cast<SFunctionInfo*>(m_pFirst);

        m_pFirst = pFunctionInfo;
    }

    // -----------------------------------------------------------------------------

    void CInternStaticFunctionList::RegisterFunctions(BState _State)
    {
        SFunctionInfo* pFunctionInfo;

        if ((_State != nullptr))
        {
            for (pFunctionInfo = static_cast<SFunctionInfo*>(m_pFirst); pFunctionInfo != nullptr; pFunctionInfo = pFunctionInfo->m_pNext)
            {
                State::RegisterFunction(_State, pFunctionInfo->m_pFunctionName, pFunctionInfo->m_pLuaCFtr);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CInternStaticFunctionList::RegisterLibrary(BState _State, const Base::Char* _pLibraryName)
    {
        SFunctionInfo* pFunctionInfo;

        if ((_State != nullptr))
        {
            for (pFunctionInfo = static_cast<SFunctionInfo*>(m_pFirst); pFunctionInfo != nullptr; pFunctionInfo = pFunctionInfo->m_pNext)
            {
                State::RegisterLibrary(_State, _pLibraryName, pFunctionInfo->m_pFunctionName, pFunctionInfo->m_pLuaCFtr);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CInternStaticFunctionList::RegisterObject(BState _State, const Base::Char* _pObjectName)
    {
        SFunctionInfo* pFunctionInfo;

        if ((_State != nullptr))
        {
            for (pFunctionInfo = static_cast<SFunctionInfo*>(m_pFirst); pFunctionInfo != nullptr; pFunctionInfo = pFunctionInfo->m_pNext)
            {
                State::RegisterObject(_State, _pObjectName, pFunctionInfo->m_pFunctionName, pFunctionInfo->m_pLuaCFtr);
            }
        }
    }
} // namespace

namespace LUA
{
    CStaticFunctionList::CStaticFunctionList()
        : m_pFirst(nullptr)
    {
    }
} // namespace LUA

namespace LUA
{
namespace Private
{
    void RequireLibrary(BState _State, const Base::Char* _pLibraryName, FLuaCFunc _pFunction)
    {
        State::RequireLibrary(_State, _pLibraryName, _pFunction);
    }

    // -----------------------------------------------------------------------------

    void AddFunction(CStaticFunctionList& _rFunctionList, void* _pFunctionInfo)
    {
        CInternStaticFunctionList& rInternFunctionList = static_cast<CInternStaticFunctionList&>(_rFunctionList);

        rInternFunctionList.AddFunction(_pFunctionInfo);
    }

    // -----------------------------------------------------------------------------

    void RegisterFunctions(BState _State, CStaticFunctionList& _rFunctionList)
    {
        CInternStaticFunctionList& rInternFunctionList = static_cast<CInternStaticFunctionList&>(_rFunctionList);

        rInternFunctionList.RegisterFunctions(_State);
    }

    // -----------------------------------------------------------------------------

    void RegisterLibrary(BState _State, CStaticFunctionList& _rFunctionList, const Base::Char* _pLibraryName)
    {
        CInternStaticFunctionList& rInternFunctionList = static_cast<CInternStaticFunctionList&>(_rFunctionList);

        rInternFunctionList.RegisterLibrary(_State, _pLibraryName);
    }

    // -----------------------------------------------------------------------------

    void RegisterObject(BState _State, CStaticFunctionList& _rFunctionList, const Base::Char* _pObjectName)
    {
        CInternStaticFunctionList& rInternFunctionList = static_cast<CInternStaticFunctionList&>(_rFunctionList);

        rInternFunctionList.RegisterObject(_State, _pObjectName);
    }
} // namespace Private
} // namespace LUA