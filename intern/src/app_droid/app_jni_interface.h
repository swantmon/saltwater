
#pragma once

#include <jni.h>

class CJNIInterface
{
public:
    static CJNIInterface& GetInstance()
    {
        static CJNIInterface s_Singleton;

        return s_Singleton;
    }

public:
    void* s_pContext = 0;
};