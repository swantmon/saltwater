
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_lua_main.h"
#include "core/core_lua_state.h"

using namespace Core::Lua;

namespace 
{
    class CLuaMain : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLuaMain)

    public:

        CLuaMain();
        ~CLuaMain();

    public:

        BState GetMainState();

    private:

        BState m_MainState;
    };
} // namespace 

namespace 
{
    CLuaMain::CLuaMain()
    {
        m_MainState = State::CreateState("MainState");
    }

    // -----------------------------------------------------------------------------

    CLuaMain::~CLuaMain()
    {
        State::DeleteState(m_MainState);
    }

    // -----------------------------------------------------------------------------

    BState CLuaMain::GetMainState()
    {
        return m_MainState;
    }
} // namespace

namespace Core
{
namespace Lua
{
namespace Main
{
    BState GetMainState()
    {
        return CLuaMain::GetInstance().GetMainState();
    }
} // namespace Main
} // namespace Lua
} // namespace Core