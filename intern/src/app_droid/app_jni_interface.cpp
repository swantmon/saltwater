
#include "app_droid/app_jni_interface.h"

#define JNI_CURRENT_VERSION JNI_VERSION_1_6

JNIEXPORT jint JNI_OnLoad(JavaVM* _pJavaVM, void* _pReserved)
{
    g_pJavaVM = _pJavaVM;

    return JNI_CURRENT_VERSION;
}