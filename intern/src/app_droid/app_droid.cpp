#include <jni.h>
#include <string>

#include "app_droid/app_application.h"

#include "base/base_console.h"
#include "base/base_exception.h"

#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "app_droid/android_native_app_glue.h"

static android_app s_AndroidApp;

extern "C" {
    JNIEXPORT void JNICALL Java_de_tu_1ilmenau_saltwater_MainActivity_nativeOnStart(JNIEnv *_pJavaEnv, jobject _Obj)
    {
        try
        {
            App::Application::OnStart(&s_AndroidApp);
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
    }

    JNIEXPORT void JNICALL Java_de_tu_1ilmenau_saltwater_MainActivity_nativeOnResume(JNIEnv *_pJavaEnv, jobject _Obj)
    {
    }

    JNIEXPORT void JNICALL Java_de_tu_1ilmenau_saltwater_MainActivity_nativeOnPause(JNIEnv *_pJavaEnv, jobject _Obj)
    {
    }

    JNIEXPORT void JNICALL Java_de_tu_1ilmenau_saltwater_MainActivity_nativeOnStop(JNIEnv *_pJavaEnv, jobject _Obj)
    {
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
    }

    JNIEXPORT void JNICALL Java_de_tu_1ilmenau_saltwater_MainActivity_nativeSetSurface(JNIEnv *_pJavaEnv, jobject _Obj, jobject _Surface)
    {
        if (_Surface != 0)
        {
            s_AndroidApp.window = ANativeWindow_fromSurface(_pJavaEnv, _Surface);
        }
        else
        {
            ANativeWindow_release(s_AndroidApp.window);
        }
    }
}
