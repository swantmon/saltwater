
#include "core/core_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector2.h"

#include "core/core_jni_interface.h"

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

        int GetDeviceRotation();

        const Base::Int2& GetDeviceDimension();

        bool CheckPermission(const std::string& _rPermission);

        void AcquirePermissions(const std::string* _pPermissions, unsigned int _NumberOfPermissions);

    public:

        CJNIInterface();
        ~CJNIInterface();

    private:

        jobject m_pContext;
        JavaVM* m_pCurrentJavaVM;
        jint m_CurrentJavaVersion;
        jobject m_GameActivityThiz;
        jclass m_GameActivityID;
        jclass m_PermissionHelperID;
        jclass m_JavaStringClass;
        jobject m_GlobalClassLoader;
        jmethodID m_FindClassMethod;
        jmethodID m_GetDeviceRotationMethod;
        jmethodID m_GetDeviceDimensionWidthMethod;
        jmethodID m_GetDeviceDimensionHeightMethod;
        jmethodID m_CheckPermissionMethod;
        jmethodID m_AcquirePermissionMethod;

        Base::Int2 m_Dimension;
    };
}

namespace
{
    CJNIInterface::CJNIInterface()
        : m_pContext                      (0)
        , m_pCurrentJavaVM                (0)
        , m_CurrentJavaVersion            (0)
        , m_GameActivityThiz              (0)
        , m_GameActivityID                (0)
        , m_PermissionHelperID            (0)
        , m_JavaStringClass               (0)
        , m_GlobalClassLoader             (0)
        , m_FindClassMethod               (0)
        , m_GetDeviceRotationMethod       (0)
        , m_GetDeviceDimensionWidthMethod (0)
        , m_GetDeviceDimensionHeightMethod(0)
        , m_Dimension                     (0)
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

            jclass MainClass        = pEnvironment->FindClass("de/tuilmenau/saltwater/GameActivity");
            jclass ClassClass       = pEnvironment->FindClass("java/lang/Class");
            jclass ClassLoaderClass = pEnvironment->FindClass("java/lang/ClassLoader");

            jmethodID GetClassLoaderMethod = pEnvironment->GetMethodID(ClassClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

            jobject LocalClassLoader = pEnvironment->CallObjectMethod(MainClass, GetClassLoaderMethod);

            m_GlobalClassLoader = pEnvironment->NewGlobalRef(LocalClassLoader);
            m_FindClassMethod   = pEnvironment->GetMethodID(ClassLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");

            m_PermissionHelperID = (jclass)pEnvironment->NewGlobalRef(pEnvironment->FindClass("de/tuilmenau/saltwater/PermissionHelper"));

            m_CheckPermissionMethod = pEnvironment->GetStaticMethodID(m_PermissionHelperID, "CheckPermission", "(Ljava/lang/String;)Z");

            m_AcquirePermissionMethod = pEnvironment->GetStaticMethodID(m_PermissionHelperID, "AcquirePermissions", "([Ljava/lang/String;)V");

            jclass LocalStringClass = pEnvironment->FindClass("java/lang/String");

            m_JavaStringClass = (jclass)pEnvironment->NewGlobalRef(LocalStringClass);
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

        jclass LocalGameActivityClass = pEnvironment->FindClass("de/tuilmenau/saltwater/GameActivity");

        m_GameActivityID = (jclass)pEnvironment->NewGlobalRef(LocalGameActivityClass);

        m_GetDeviceRotationMethod = pEnvironment->GetMethodID(m_GameActivityID, "GetDeviceRotation", "()I");

        m_GetDeviceDimensionWidthMethod = pEnvironment->GetMethodID(m_GameActivityID, "GetDeviceDimensionWidth", "()I");

        m_GetDeviceDimensionHeightMethod = pEnvironment->GetMethodID(m_GameActivityID, "GetDeviceDimensionHeight", "()I");
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

    int CJNIInterface::GetDeviceRotation()
    {
        JNIEnv* pEnvironment = GetJavaEnvironment();

        jint DeviceRotation = (jint)pEnvironment->CallIntMethod(m_GameActivityThiz, m_GetDeviceRotationMethod);

        return DeviceRotation;
    }

    // -----------------------------------------------------------------------------

    const Base::Int2& CJNIInterface::GetDeviceDimension()
    {
        JNIEnv* pEnvironment = GetJavaEnvironment();

        jint Width  = (jint)pEnvironment->CallIntMethod(m_GameActivityThiz, m_GetDeviceDimensionWidthMethod);
        jint Height = (jint)pEnvironment->CallIntMethod(m_GameActivityThiz, m_GetDeviceDimensionHeightMethod);

        m_Dimension[0] = Width;
        m_Dimension[1] = Height;

        return m_Dimension;
    }

    // -----------------------------------------------------------------------------

    bool CJNIInterface::CheckPermission(const std::string& _rPermission)
    {
        JNIEnv* pEnvironment = GetJavaEnvironment();

        jstring Argument = pEnvironment->NewStringUTF(_rPermission.c_str());

        bool Result = pEnvironment->CallStaticBooleanMethod(m_PermissionHelperID, m_CheckPermissionMethod, Argument);

        pEnvironment->DeleteLocalRef(Argument);

        return Result;
    }

    // -----------------------------------------------------------------------------

    void CJNIInterface::AcquirePermissions(const std::string* _pPermissions, unsigned int _NumberOfPermissions)
    {
        JNIEnv* pEnvironment = GetJavaEnvironment();

        jobjectArray PermissionsArray = (jobjectArray)pEnvironment->NewObjectArray(_NumberOfPermissions, m_JavaStringClass, 0);

        for (int i = 0; i < _NumberOfPermissions; ++i)
        {
            jstring JavaString = pEnvironment->NewStringUTF(_pPermissions[i].c_str());

            pEnvironment->SetObjectArrayElement(PermissionsArray, i, JavaString);

            pEnvironment->DeleteLocalRef(JavaString);
        }

        pEnvironment->CallStaticVoidMethod(m_PermissionHelperID, m_AcquirePermissionMethod, PermissionsArray);

        pEnvironment->DeleteLocalRef(PermissionsArray);
    }
}

namespace Core
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

    int GetDeviceRotation()
    {
        return CJNIInterface::GetInstance().GetDeviceRotation();
    }

    // -----------------------------------------------------------------------------

    const Base::Int2& GetDeviceDimension()
    {
        return CJNIInterface::GetInstance().GetDeviceDimension();
    }

    // -----------------------------------------------------------------------------

    bool CheckPermission(const std::string& _rPermission)
    {
        return CJNIInterface::GetInstance().CheckPermission(_rPermission);
    }

    // -----------------------------------------------------------------------------

    void AcquirePermissions(const std::string* _pPermissions, unsigned int _NumberOfPermissions)
    {
        CJNIInterface::GetInstance().AcquirePermissions(_pPermissions, _NumberOfPermissions);
    }
} // namespace JNI
} // namespace Core

// -----------------------------------------------------------------------------
// Native interface from JAVA to C++
// -----------------------------------------------------------------------------
extern "C"
{
    JNIEXPORT void JNICALL Java_de_tuilmenau_saltwater_GameActivity_nativeInitializeInterface(JNIEnv* _pEnv, jobject _LocalThiz, jobject _Context)
    {
        CJNIInterface::GetInstance().SetActivity(_pEnv->NewGlobalRef(_LocalThiz));

        CJNIInterface::GetInstance().SetContext(_pEnv->NewGlobalRef(_Context));

        CJNIInterface::GetInstance().FindClassesAndMethods();
    }

    // -----------------------------------------------------------------------------

    JNIEXPORT void JNICALL Java_de_tuilmenau_saltwater_PermissionHelper_nativeOnAcquirePermissions(JNIEnv* _pEnv, jobject _LocalThiz, jobjectArray _Permissions, jintArray _GrantResults)
    {
        jint* pGrantResults = _pEnv->GetIntArrayElements(_GrantResults, NULL);

        _pEnv->ReleaseIntArrayElements(_GrantResults, pGrantResults, 0);
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