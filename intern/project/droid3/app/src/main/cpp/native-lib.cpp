#include <jni.h>
#include <string>


#include "app_droid/app_application.h"

#include "base/base_console.h"
#include "base/base_exception.h"


//#include "app_droid/app_start_state.h"

//#include "graphic/gfx_application_interface.h"

#include "core/core_time.h"

extern "C"
JNIEXPORT jstring

JNICALL
Java_de_tu_1ilmenau_saltwater_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {


    try
    {
        App::Application::OnStart(0);

        App::Application::OnRun();
    }
    catch (const Base::CException& _rException)
    {
        BASE_CONSOLE_ERROR("An Exception stops application");
        BASE_CONSOLE_INFOV(" > Reason:   %s", _rException.GetText());
        BASE_CONSOLE_INFOV(" > Code:     %i", _rException.GetCode());
        BASE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        BASE_CONSOLE_ERROR("An undefined exception stops application");
    }

    try
    {
        App::Application::OnExit();
    }
    catch (const Base::CException& _rException)
    {
        BASE_CONSOLE_ERROR("An Exception stops application");
        BASE_CONSOLE_INFOV(" > Reason:   %s", _rException.GetText());
        BASE_CONSOLE_INFOV(" > Code:     %i", _rException.GetCode());
        BASE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        BASE_CONSOLE_ERROR("An undefined exception stops application");
    }

    //unsigned int WindowID = Gfx::App::RegisterWindow(0);

    //App::CStartState::GetInstance().OnEnter();
    //App::CStartState::GetInstance().OnRun();
    //App::CStartState::GetInstance().OnLeave();

    Core::Time::Update();
    Core::Time::Update();
    Core::Time::Update();

    int Frame = Core::Time::GetNumberOfFrame();

    double D = Core::Time::GetDeltaTimeLastFrame();

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
