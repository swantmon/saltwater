
#include "app_droid/app_jni_interface.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------
#define JNI_CURRENT_VERSION JNI_VERSION_1_6

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------
namespace
{
    class CJNIInterface : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CJNIInterface)

    public:

        void SetContext(void* _pContext);
        void* GetContext();

    public:

        CJNIInterface();
        ~CJNIInterface();

    private:

        void* m_pContext;
    };
}

namespace
{
    CJNIInterface::CJNIInterface()
        : m_pContext(0)
    {

    };

    // -----------------------------------------------------------------------------

    CJNIInterface::~CJNIInterface()
    {

    };

    // -----------------------------------------------------------------------------

    void CJNIInterface::SetContext(void* _pContext)
    {
        m_pContext = _pContext;
    };

    // -----------------------------------------------------------------------------

    void* CJNIInterface::GetContext()
    {
        return m_pContext;
    };
}

namespace App
{
namespace JNI
{
    void SetContext(void* _pContext)
    {
        CJNIInterface::GetInstance().SetContext(_pContext);
    }

    // -----------------------------------------------------------------------------

    void* GetContext()
    {
        return CJNIInterface::GetInstance().GetContext();
    }
} // namespace JNI
} // namespace App

// -----------------------------------------------------------------------------
// Native interface from JAVA to C++
// -----------------------------------------------------------------------------
extern "C"
{
    JNIEXPORT void JNICALL Java_de_tu_1ilmenau_saltwater_GameActivity_nativeSetContext(JNIEnv* _pEnv, jobject _LocalThizz, jobject _Context)
    {
        CJNIInterface::GetInstance().SetContext(_Context);
    }
}; // extern "C"

// -----------------------------------------------------------------------------
// Startup function
// -----------------------------------------------------------------------------
JNIEXPORT jint JNI_OnLoad(JavaVM* _pJavaVM, void* _pReserved)
{
    // TODO:
    // Initialize interface and preload class loader + methods

    return JNI_CURRENT_VERSION;
}