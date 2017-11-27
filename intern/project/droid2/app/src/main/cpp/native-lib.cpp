#include <jni.h>
#include <string>

#include "base/base_timer.h"

extern "C"
JNIEXPORT jstring

JNICALL
Java_de_tu_1ilmenau_saltwater_MainActivity_stringFromJNI(JNIEnv *env, jobject /* this */)
{
    Base::CDefaultClock DefaultClock;

    double Test = DefaultClock.GetTime();


    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
