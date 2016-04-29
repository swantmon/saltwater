
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "logic/lua_main.h"
#include "logic/lua_state.h"

using namespace LUA;

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

namespace LUA
{
namespace Main
{
    BState GetMainState()
    {
        return CLuaMain::GetInstance().GetMainState();
    }
} // namespace Main
} // namespace LUA