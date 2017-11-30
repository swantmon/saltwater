#include <jni.h>
#include <string>

#include "core/core_time.h"

extern "C"
JNIEXPORT jstring

JNICALL
Java_de_tu_1ilmenau_saltwater_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

    Core::Time::Update();
    Core::Time::Update();
    Core::Time::Update();

    int Frame = Core::Time::GetNumberOfFrame();

    double D = Core::Time::GetDeltaTimeLastFrame();

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
