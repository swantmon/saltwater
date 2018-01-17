
#include "app_droid/app_jni_interface.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include <jni.h>

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

        void InitializeJNI(JavaVM* _pJavaVM, jint _Version);

        void FindClassesAndMethods();

        JNIEnv* GetJavaEnvironment();

        void SetActivity(jobject _Activity);

        void SetContext(jobject _pContext);
        jobject GetContext();

        bool HasCameraPermission();

    public:

        CJNIInterface();
        ~CJNIInterface();

    private:

        jobject m_pContext;
        JavaVM* m_pCurrentJavaVM;
        jint m_CurrentJavaVersion;
        jobject m_GameActivityThiz;
        jclass m_GameActivityID;
        jobject m_GlobalClassLoader;
        jmethodID m_FindClassMethod;
        jmethodID m_HasCameraPermissionMethod;
    };
}

namespace
{
    CJNIInterface::CJNIInterface()
        : m_pContext                 (0)
        , m_pCurrentJavaVM           (0)
        , m_CurrentJavaVersion       (0)
        , m_GameActivityThiz         (0)
        , m_GameActivityID           (0)
        , m_GlobalClassLoader        (0)
        , m_FindClassMethod          (0)
        , m_HasCameraPermissionMethod(0)
    {

    };

    // -----------------------------------------------------------------------------

    CJNIInterface::~CJNIInterface()
    {

    };

    // -----------------------------------------------------------------------------

    void CJNIInterface::InitializeJNI(JavaVM* _pJavaVM, jint _Version)
    {
        if (m_pCurrentJavaVM == 0)
        {
            m_pCurrentJavaVM     = _pJavaVM;
            m_CurrentJavaVersion = _Version;

            JNIEnv* pEnvironment = GetJavaEnvironment();

            jclass MainClass        = pEnvironment->FindClass("de/tu_ilmenau/saltwater/GameActivity");
            jclass ClassClass       = pEnvironment->FindClass("java/lang/Class");
            jclass ClassLoaderClass = pEnvironment->FindClass("java/lang/ClassLoader");

            jmethodID GetClassLoaderMethod = pEnvironment->GetMethodID(ClassClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

            jobject LocalClassLoader = pEnvironment->CallObjectMethod(MainClass, GetClassLoaderMethod);

            m_GlobalClassLoader = pEnvironment->NewGlobalRef(LocalClassLoader);
            m_FindClassMethod   = pEnvironment->GetMethodID(ClassLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        }
    }

    // -----------------------------------------------------------------------------

    void CJNIInterface::FindClassesAndMethods()
    {
        // -----------------------------------------------------------------------------
        // Signature	                Java Type
        // V	                        void
        // Z	                        boolean
        // B	                        byte
        // C        	                char
        // S        	                short
        // I        	                int
        // J        	                long
        // F        	                float
        // D        	                double
        // L fully-qualified-class ;	fully-qualified-class
        // [ type	                    type[]
        // ( arg-types ) ret-type	    method type
        //
        // Example: (Ljava/lang/String;)Ljava/lang/String;
        // -----------------------------------------------------------------------------

        JNIEnv* pEnvironment = GetJavaEnvironment();

        jclass LocalGameActivityClass = pEnvironment->FindClass("de/tu_ilmenau/saltwater/GameActivity");

        m_GameActivityID = (jclass)pEnvironment->NewGlobalRef(LocalGameActivityClass);

        m_HasCameraPermissionMethod = pEnvironment->GetMethodID(m_GameActivityID, "HasCameraPermission", "()Z");
    }

    // -----------------------------------------------------------------------------

    JNIEnv* CJNIInterface::GetJavaEnvironment()
    {
        JNIEnv* pEnvironment = nullptr;

        jint GetResult = m_pCurrentJavaVM->GetEnv((void **)&pEnvironment, JNI_CURRENT_VERSION);

        if (GetResult == JNI_EDETACHED)
        {
            jint AttachResult = m_pCurrentJavaVM->AttachCurrentThread(&pEnvironment, NULL);

            if (AttachResult == JNI_ERR)
            {
                BASE_CONSOLE_ERROR("Failed to attach thread to get the JNI environment!");

                return nullptr;
            }
        }
        else if (GetResult != JNI_OK)
        {
            BASE_CONSOLE_ERRORV("Failed to get the JNI environment! Result = %d", GetResult);

            return nullptr;
        }

        return pEnvironment;
    }

    // -----------------------------------------------------------------------------

    void CJNIInterface::SetContext(jobject _pContext)
    {
        m_pContext = _pContext;
    };

    // -----------------------------------------------------------------------------

    jobject CJNIInterface::GetContext()
    {
        return m_pContext;
    };

    // -----------------------------------------------------------------------------

    void CJNIInterface::SetActivity(jobject _Activity)
    {
        m_GameActivityThiz = _Activity;
    }

    // -----------------------------------------------------------------------------

    bool CJNIInterface::HasCameraPermission()
    {
        JNIEnv* pEnvironment = GetJavaEnvironment();

        jboolean HasCameraPermission = (jboolean)pEnvironment->CallBooleanMethod(m_GameActivityThiz, m_HasCameraPermissionMethod);

        return HasCameraPermission == JNI_TRUE;
    }
}

namespace App
{
namespace JNI
{
    void* GetJavaEnvironment()
    {
        return CJNIInterface::GetInstance().GetJavaEnvironment();
    }

    // -----------------------------------------------------------------------------

    void* GetContext()
    {
        return CJNIInterface::GetInstance().GetContext();
    }

    // -----------------------------------------------------------------------------

    bool HasCameraPermission()
    {
        return CJNIInterface::GetInstance().HasCameraPermission();
    }
} // namespace JNI
} // namespace App

// -----------------------------------------------------------------------------
// Native interface from JAVA to C++
// -----------------------------------------------------------------------------
extern "C"
{
    JNIEXPORT void JNICALL Java_de_tu_1ilmenau_saltwater_GameActivity_native_1InitializeInterface(JNIEnv* _pEnv, jobject _LocalThiz, jobject _Context)
    {
        CJNIInterface::GetInstance().SetActivity(_pEnv->NewGlobalRef(_LocalThiz));

        CJNIInterface::GetInstance().SetContext(_pEnv->NewGlobalRef(_Context));

        CJNIInterface::GetInstance().FindClassesAndMethods();
    }
}; // extern "C"

// -----------------------------------------------------------------------------
// Startup function
// -----------------------------------------------------------------------------
JNIEXPORT jint JNI_OnLoad(JavaVM* _pJavaVM, void* _pReserved)
{
    CJNIInterface::GetInstance().InitializeJNI(_pJavaVM, JNI_CURRENT_VERSION);

    return JNI_CURRENT_VERSION;
}