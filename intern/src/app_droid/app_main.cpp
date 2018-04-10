
#include "app_droid/app_application.h"

#include "base/base_exception.h"

#include "core/core_console.h"

#include <android_native_app_glue.h>

void android_main(android_app* _pAndroidApp)
{
    try
    {
        App::Application::OnStart(_pAndroidApp);

        App::Application::OnRun();
    }
    catch (const Base::CException& _rException)
    {
        ENGINE_CONSOLE_ERROR("An Exception stops application");
        ENGINE_CONSOLE_INFOV(" > Reason:   %s", _rException.GetText());
        ENGINE_CONSOLE_INFOV(" > Code:     %i", _rException.GetCode());
        ENGINE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        ENGINE_CONSOLE_ERROR("An undefined exception stops application");
    }

    try
    {
        App::Application::OnExit();
    }
    catch (const Base::CException& _rException)
    {
        ENGINE_CONSOLE_ERROR("An Exception stops application");
        ENGINE_CONSOLE_INFOV(" > Reason:   %s", _rException.GetText());
        ENGINE_CONSOLE_INFOV(" > Code:     %i", _rException.GetCode());
        ENGINE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        ENGINE_CONSOLE_ERROR("An undefined exception stops application");
    }
}