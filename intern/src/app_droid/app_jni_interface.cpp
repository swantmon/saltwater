
#include "app_droid/app_jni_interface.h"

#include <assert.h>

#define JNI_CURRENT_VERSION JNI_VERSION_1_6

extern "C"
{

JNIEXPORT void JNICALL Java_de_tu_1ilmenau_saltwater_GameActivity_nativeSetContext(JNIEnv* _pEnv, jobject _LocalThizz, jobject _Context)
{
    CJNIInterface::GetInstance().s_pContext = (void*)_Context;
}

}; // extern "C"

JNIEXPORT jint JNI_OnLoad(JavaVM* _pJavaVM, void* _pReserved)
{
    // TODO:
    // Initialize interface and preload class loader + methods

    return JNI_CURRENT_VERSION;
}