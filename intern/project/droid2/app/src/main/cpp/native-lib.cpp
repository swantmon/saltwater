#include <jni.h>
#include <string>

#include "app_droid/app_application.h"

extern "C"
JNIEXPORT jstring

JNICALL
Java_de_tu_1ilmenau_saltwater_MainActivity_stringFromJNI(JNIEnv *env, jobject /* this */)
{
    App::Application::OnStart(0);


    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
