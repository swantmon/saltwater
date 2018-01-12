
#include "app_droid/app_application.h"
#include "app_droid/app_jni_interface.h"

#include "base/base_console.h"
#include "base/base_exception.h"

#include <assert.h>

#define JNI_CURRENT_VERSION JNI_VERSION_1_6

JNIEXPORT jint JNI_OnLoad(JavaVM* _pJavaVM, void* _pReserved)
{
    BASE_CONSOLE_INFO("YES");
    int a = 64;

    assert(_pJavaVM == 0);

    return JNI_CURRENT_VERSION;
}