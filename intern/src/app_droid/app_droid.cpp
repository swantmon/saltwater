#include <jni.h>
#include <string>

#include "app_droid/app_application.h"

#include "base/base_console.h"
#include "base/base_exception.h"

extern "C"
JNIEXPORT jstring

JNICALL
Java_de_tu_1ilmenau_saltwater_MainActivity_stringFromJNI(JNIEnv *env, jobject /* this */)
{
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

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
