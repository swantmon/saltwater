
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"
#include "base/base_managed_pool.h"
#include "base/base_singleton.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"

#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_native_sampler.h"
#include "engine/graphic/gfx_native_sampler_set.h"
#include "engine/graphic/gfx_sampler_manager.h"

using namespace Gfx;

namespace
{
    class CGfxSamplerManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxSamplerManager)
        
    public:
        
        struct SSamplerDescriptor
        {
            CSampler::EFilter Filter;
            CSampler::EAddressMode AddressModeU;
            CSampler::EAddressMode AddressModeV;
            CSampler::EAddressMode AddressModeW;
            CSampler::EComparisonFunction ComparisonFunction;
            float MaxAnisotropy;
            float MinLOD;
            float MaxLOD;
            float Bias;
        };
        
    public:

        CGfxSamplerManager();
       ~CGfxSamplerManager();

    public:

        void OnStart();
        void OnExit();

    public:

        CSamplerPtr GetSampler(CSampler::ESampler _Sampler);

    public:

        CSamplerSetPtr CreateSamplerSet(CSamplerPtr* _pSamplerPtrs, unsigned int _NumberOfSamplers);

    public:

        void SetSamplerLabel(CSamplerPtr _SamplerPtr, const char* _pLabel);

    private:

        // -----------------------------------------------------------------------------
        // Represents a unique sampler.
        // -----------------------------------------------------------------------------
        class CInternSampler : public CNativeSampler
        {
            public:

                CInternSampler();
               ~CInternSampler();

            private:

                friend class CGfxSamplerManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a unique combination of up to 16 samplers.
        // -----------------------------------------------------------------------------
        class CInternSamplerSet : public CNativeSamplerSet
        {
            public:

                CInternSamplerSet();
               ~CInternSamplerSet();

            public:

                bool operator == (const CInternSamplerSet& _rSamplerSet) const;
                bool operator != (const CInternSamplerSet& _rSamplerSet) const;

            private:

                friend class CGfxSamplerManager;
        };

        // -----------------------------------------------------------------------------
        // The sampler pool is driven by the user policy because samplers cannot be
        // created from outside and the manager already holds a fixed size array of
        // smart pointers to the samplers.
        // -----------------------------------------------------------------------------
        typedef Base::CManagedPool<CInternSampler>    CSamplers;
        typedef Base::CManagedPool<CInternSamplerSet> CSamplerSets;
        
    private:
        
        static SSamplerDescriptor s_NativeSamplerDescriptors[CSampler::NumberOfSamplers];

    private:

        CSamplers          m_Samplers;
        CSamplers::CPtr    m_SamplerPtrs[CSampler::NumberOfSamplers];
        CSamplerSets       m_SamplerSets;
        CSamplerSets::CPtr m_SamplerSetPtrs[CSampler::NumberOfSamplers];
        GLuint             m_NativeSampler[CSampler::NumberOfSamplers];
        
    private:
        
        int ConvertAddressMode(CSampler::EAddressMode _AddressMode);
        int ConvertMagFilterMode(CSampler::EFilter _Filter);
        int ConvertMinFilterMode(CSampler::EFilter _Filter);
        int ConvertComparisonMode(CSampler::EComparisonFunction _ComparisonFunc);
        bool IsComparisonActive(CSampler::EFilter _Filter);
    };
} // namespace

namespace
{
    CGfxSamplerManager::SSamplerDescriptor CGfxSamplerManager::s_NativeSamplerDescriptors[CSampler::NumberOfSamplers] =
    {
        // -----------------------------------------------------------------------------
        // State             | Default Value          |
        // --------------------------------------------
        // Filter            | MIN_MAG_MIP_LINEAR     |
        // AddressU          | Clamp                  |
        // AddressV          | Clamp                  |
        // AddressW          | Clamp                  |
        // ComparisonFunc    | Never                  |
        // MaxAnisotropy     | 16                     |
        // Minimum LOD       | Minimum Float          |
        // Maximum LOD       | Maximum Float          |
        // BIAS              | 0.0f                   |
        // -----------------------------------------------------------------------------
       
        { CSampler::MinMagMipPoint        , CSampler::Clamp , CSampler::Clamp , CSampler::Clamp , CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagMipPoint        , CSampler::Wrap  , CSampler::Wrap  , CSampler::Wrap  , CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagMipPoint        , CSampler::Border, CSampler::Border, CSampler::Border, CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagMipPoint        , CSampler::Mirror, CSampler::Mirror, CSampler::Mirror, CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagLinearMipPoint  , CSampler::Clamp , CSampler::Clamp , CSampler::Clamp , CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagLinearMipPoint  , CSampler::Wrap  , CSampler::Wrap  , CSampler::Wrap  , CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagLinearMipPoint  , CSampler::Border, CSampler::Border, CSampler::Border, CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagLinearMipPoint  , CSampler::Mirror, CSampler::Mirror, CSampler::Mirror, CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagMipLinear       , CSampler::Clamp , CSampler::Clamp , CSampler::Clamp , CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagMipLinear       , CSampler::Wrap  , CSampler::Wrap  , CSampler::Wrap  , CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagMipLinear       , CSampler::Border, CSampler::Border, CSampler::Border, CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagMipLinear       , CSampler::Mirror, CSampler::Mirror, CSampler::Mirror, CSampler::Never      , 16.0f, -1000.0f, 1000.0f, 0.0f },
        { CSampler::MinMagMipLinearCompare, CSampler::Clamp , CSampler::Clamp , CSampler::Clamp , CSampler::LesserEqual, 16.0f, -1000.0f, 1000.0f, 0.0f },
    };
} // namespace

namespace
{
    CGfxSamplerManager::CGfxSamplerManager()
        : m_Samplers   ()
        , m_SamplerSets()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxSamplerManager::~CGfxSamplerManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxSamplerManager::OnStart()
    {
        unsigned int IndexOfSampler;

        // -----------------------------------------------------------------------------
        // Reset the interfaces so we are safe to release them in case of an exception.
        // -----------------------------------------------------------------------------
        for (IndexOfSampler = 0; IndexOfSampler < CSampler::NumberOfSamplers; ++ IndexOfSampler)
        {
            m_SamplerPtrs[IndexOfSampler] = nullptr;
        }

        try
        {
            // -----------------------------------------------------------------------------
            // Generate samplers
            // -----------------------------------------------------------------------------
            glGenSamplers(CSampler::NumberOfSamplers, m_NativeSampler);

            for (IndexOfSampler = 0; IndexOfSampler < CSampler::NumberOfSamplers; ++ IndexOfSampler)
            {
                // -----------------------------------------------------------------------------
                // Create sampler
                // -----------------------------------------------------------------------------
                m_SamplerPtrs[IndexOfSampler] = m_Samplers.Allocate();

                m_SamplerPtrs[IndexOfSampler]->m_Type               = static_cast<CSampler::ESampler>(IndexOfSampler);
                m_SamplerPtrs[IndexOfSampler]->m_AddressMode[0]     = s_NativeSamplerDescriptors[IndexOfSampler].AddressModeU;
                m_SamplerPtrs[IndexOfSampler]->m_AddressMode[1]     = s_NativeSamplerDescriptors[IndexOfSampler].AddressModeV;
                m_SamplerPtrs[IndexOfSampler]->m_AddressMode[2]     = s_NativeSamplerDescriptors[IndexOfSampler].AddressModeW;
                m_SamplerPtrs[IndexOfSampler]->m_Filter             = s_NativeSamplerDescriptors[IndexOfSampler].Filter;
                m_SamplerPtrs[IndexOfSampler]->m_ComparisonFunction = s_NativeSamplerDescriptors[IndexOfSampler].ComparisonFunction;

                // -----------------------------------------------------------------------------
                // Create native sampler
                // -----------------------------------------------------------------------------
                GLuint NativeSampler = m_NativeSampler[IndexOfSampler];

                int AdressModeU = ConvertAddressMode(s_NativeSamplerDescriptors[IndexOfSampler].AddressModeU);
                int AdressModeV = ConvertAddressMode(s_NativeSamplerDescriptors[IndexOfSampler].AddressModeV);
                int AdressModeW = ConvertAddressMode(s_NativeSamplerDescriptors[IndexOfSampler].AddressModeW);
                
                int MagFilter = ConvertMagFilterMode(s_NativeSamplerDescriptors[IndexOfSampler].Filter);
                int MinFilter = ConvertMinFilterMode(s_NativeSamplerDescriptors[IndexOfSampler].Filter);
                
                bool IsComparison = IsComparisonActive(s_NativeSamplerDescriptors[IndexOfSampler].Filter);
                
                int ComparisonMode = ConvertComparisonMode(s_NativeSamplerDescriptors[IndexOfSampler].ComparisonFunction);
                
                glSamplerParameteri(NativeSampler, GL_TEXTURE_WRAP_S, AdressModeU); // GL_REPEAT
                glSamplerParameteri(NativeSampler, GL_TEXTURE_WRAP_T, AdressModeV); // GL_REPEAT
                glSamplerParameteri(NativeSampler, GL_TEXTURE_WRAP_R, AdressModeW); // GL_REPEAT
                
                glSamplerParameteri(NativeSampler, GL_TEXTURE_MAG_FILTER, MagFilter); // GL_LINEAR
                glSamplerParameteri(NativeSampler, GL_TEXTURE_MIN_FILTER, MinFilter); // GL_LINEAR
                
                glSamplerParameterf(NativeSampler, GL_TEXTURE_MIN_LOD, s_NativeSamplerDescriptors[IndexOfSampler].MinLOD);
                glSamplerParameterf(NativeSampler, GL_TEXTURE_MAX_LOD, s_NativeSamplerDescriptors[IndexOfSampler].MaxLOD);
                
#ifndef PLATFORM_ANDROID
                glSamplerParameterf(NativeSampler, GL_TEXTURE_LOD_BIAS, s_NativeSamplerDescriptors[IndexOfSampler].Bias);
#endif // !PLATFORM_ANDROID
                
                glSamplerParameteri(NativeSampler, GL_TEXTURE_COMPARE_MODE, GL_NONE);

                glSamplerParameteri(NativeSampler, GL_TEXTURE_COMPARE_FUNC, GL_NEVER);

                if (IsComparison)
                {
                    glSamplerParameteri(NativeSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

                    glSamplerParameteri(NativeSampler, GL_TEXTURE_COMPARE_FUNC, ComparisonMode);
                }

                glObjectLabel(GL_SAMPLER, NativeSampler, -1, m_SamplerPtrs[IndexOfSampler]->GetName());
            }
        }
        catch (...)
        {
            m_Samplers.Clear();

            BASE_THROWM("Error creating sampler!");
        }

        // -----------------------------------------------------------------------------
        // No exception from now on so setup the samplers.
        // -----------------------------------------------------------------------------
        for (IndexOfSampler = 0; IndexOfSampler < CSampler::NumberOfSamplers; ++ IndexOfSampler)
        {
            CInternSampler& rSampler = *m_SamplerPtrs[IndexOfSampler];

            rSampler.m_Filter              = s_NativeSamplerDescriptors[IndexOfSampler].Filter;
            rSampler.m_AddressMode[0]      = s_NativeSamplerDescriptors[IndexOfSampler].AddressModeU;
            rSampler.m_AddressMode[1]      = s_NativeSamplerDescriptors[IndexOfSampler].AddressModeV;
            rSampler.m_AddressMode[2]      = s_NativeSamplerDescriptors[IndexOfSampler].AddressModeW;
            rSampler.m_ComparisonFunction  = s_NativeSamplerDescriptors[IndexOfSampler].ComparisonFunction;
            rSampler.m_NativeSampler       = m_NativeSampler[IndexOfSampler];
            rSampler.m_Type                = static_cast<CSampler::ESampler>(IndexOfSampler);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSamplerManager::OnExit()
    {
        unsigned int IndexOfSampler;

        // -----------------------------------------------------------------------------
        // Release all smart pointers to the samplers.
        // -----------------------------------------------------------------------------
        for (IndexOfSampler = 0; IndexOfSampler < CSampler::NumberOfSamplers; ++ IndexOfSampler)
        {
            m_SamplerPtrs[IndexOfSampler] = nullptr;
        }

        // -----------------------------------------------------------------------------
        // First release the sets to decrease the reference counters of the samplers.
        // -----------------------------------------------------------------------------
        m_SamplerSets.Clear();
    }

    // -----------------------------------------------------------------------------

    CSamplerPtr CGfxSamplerManager::GetSampler(CSampler::ESampler _Sampler)
    {
        return CSamplerPtr(m_SamplerPtrs[_Sampler]);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CGfxSamplerManager::CreateSamplerSet(CSamplerPtr* _pSamplerPtrs, unsigned int _NumberOfSamplers)
    {
        unsigned int IndexOfSampler;

        assert((_pSamplerPtrs != nullptr) && (_NumberOfSamplers < CSamplerSet::s_MaxNumberOfSamplers));

        // -----------------------------------------------------------------------------
        // We did not find a matching entry, so create a new sampler set.
        // -----------------------------------------------------------------------------
        Base::CManagedPoolItemPtr<CInternSamplerSet> SamplerSetPtr = m_SamplerSets.Allocate();

        CInternSamplerSet& rSamplerSet = *SamplerSetPtr;

        for (IndexOfSampler = 0; IndexOfSampler < _NumberOfSamplers; ++ IndexOfSampler)
        {
            rSamplerSet.m_SamplerPtrs[IndexOfSampler] = _pSamplerPtrs[IndexOfSampler];
        }

        rSamplerSet.m_NumberOfSamplers = _NumberOfSamplers;

        // -----------------------------------------------------------------------------
        // Return a smart pointer to the visible base class.
        // -----------------------------------------------------------------------------
        return CSamplerSetPtr(SamplerSetPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxSamplerManager::SetSamplerLabel(CSamplerPtr _SamplerPtr, const char* _pLabel)
    {
        assert(_pLabel != nullptr);

        CInternSampler* pInternSampler = static_cast<CInternSampler*>(_SamplerPtr.GetPtr());

        glObjectLabel(GL_SAMPLER, pInternSampler->m_NativeSampler, -1, _pLabel);
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxSamplerManager::ConvertAddressMode(CSampler::EAddressMode _AddressMode)
    {
        static int s_NativeAdressMode[] =
        {
            GL_REPEAT,
            GL_MIRRORED_REPEAT,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_BORDER,
            GL_CLAMP_TO_EDGE,
        };
        
        return s_NativeAdressMode[_AddressMode];
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxSamplerManager::ConvertMagFilterMode(CSampler::EFilter _Filter)
    {
        static int s_NativeMagFilterMode[] =
        {
            GL_NEAREST,
            GL_NEAREST,
            GL_LINEAR,
            GL_LINEAR,
            GL_NEAREST,
            GL_NEAREST,
            GL_LINEAR,
            GL_LINEAR,
            GL_NEAREST,
            GL_NEAREST,
            GL_LINEAR,
            GL_LINEAR,
            GL_NEAREST,
            GL_NEAREST,
            GL_LINEAR,
            GL_LINEAR,
        };
        
        return s_NativeMagFilterMode[_Filter];
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxSamplerManager::ConvertMinFilterMode(CSampler::EFilter _Filter)
    {
        static int s_NativeMinFilterMode[] =
        {
            GL_NEAREST_MIPMAP_NEAREST,
            GL_NEAREST_MIPMAP_LINEAR,
            GL_NEAREST_MIPMAP_NEAREST,
            GL_NEAREST_MIPMAP_LINEAR,
            GL_LINEAR_MIPMAP_NEAREST,
            GL_LINEAR_MIPMAP_LINEAR,
            GL_LINEAR_MIPMAP_NEAREST,
            GL_LINEAR_MIPMAP_LINEAR,
            GL_NEAREST_MIPMAP_NEAREST,
            GL_NEAREST_MIPMAP_LINEAR,
            GL_NEAREST_MIPMAP_NEAREST,
            GL_NEAREST_MIPMAP_LINEAR,
            GL_LINEAR_MIPMAP_NEAREST,
            GL_LINEAR_MIPMAP_LINEAR,
            GL_LINEAR_MIPMAP_NEAREST,
            GL_LINEAR_MIPMAP_LINEAR,
        };
        
        return s_NativeMinFilterMode[_Filter];
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxSamplerManager::ConvertComparisonMode(CSampler::EComparisonFunction _ComparisonFunc)
    {
        static int s_NativeComparisonMode[] =
        {
            GL_NEVER,
            GL_LESS,
            GL_EQUAL,
            GL_LEQUAL,
            GL_GREATER,
            GL_NOTEQUAL,
            GL_GEQUAL,
            GL_ALWAYS,
        };
        
        return s_NativeComparisonMode[_ComparisonFunc];
    }
    
    // -----------------------------------------------------------------------------
    
    bool CGfxSamplerManager::IsComparisonActive(CSampler::EFilter _Filter)
    {
        static bool s_IsComparison[] =
        {
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
        };
        
        return s_IsComparison[_Filter];
    }
} // namespace

namespace
{
    CGfxSamplerManager::CInternSampler::CInternSampler()
        : CNativeSampler()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxSamplerManager::CInternSampler::~CInternSampler()
    {
    }
} // namespace

namespace
{
    CGfxSamplerManager::CInternSamplerSet::CInternSamplerSet()
        : CNativeSamplerSet()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxSamplerManager::CInternSamplerSet::~CInternSamplerSet()
    {
    }

    // -----------------------------------------------------------------------------

    bool CGfxSamplerManager::CInternSamplerSet::operator == (const CInternSamplerSet& _rSamplerSet) const
    {
        unsigned int IndexOfSampler;

        if (m_NumberOfSamplers != _rSamplerSet.m_NumberOfSamplers)
        {
            return false;
        }

        for (IndexOfSampler = 0; IndexOfSampler < m_NumberOfSamplers; ++ IndexOfSampler)
        {
        }

        return true;
    }

    // -----------------------------------------------------------------------------

    bool CGfxSamplerManager::CInternSamplerSet::operator != (const CInternSamplerSet& _rSamplerSet) const
    {
        unsigned int IndexOfSampler;

        if (m_NumberOfSamplers != _rSamplerSet.m_NumberOfSamplers)
        {
            return true;
        }

        for (IndexOfSampler = 0; IndexOfSampler < m_NumberOfSamplers; ++ IndexOfSampler)
        {
        }

        return false;
    }
} // namespace

namespace Gfx
{
namespace SamplerManager
{
    void OnStart()
    {
        CGfxSamplerManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxSamplerManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    CSamplerPtr GetSampler(CSampler::ESampler _Sampler)
    {
        return CGfxSamplerManager::GetInstance().GetSampler(_Sampler);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr)
    {
        CSamplerPtr SamplerPtrs[] = { _Sampler1Ptr, };

        return CGfxSamplerManager::GetInstance().CreateSamplerSet(SamplerPtrs, 1);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr)
    {
        CSamplerPtr SamplerPtrs[] = { _Sampler1Ptr, _Sampler2Ptr, };

        return CGfxSamplerManager::GetInstance().CreateSamplerSet(SamplerPtrs, 2);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr, CSamplerPtr _Sampler3Ptr)
    {
        CSamplerPtr SamplerPtrs[] = { _Sampler1Ptr, _Sampler2Ptr, _Sampler3Ptr, };

        return CGfxSamplerManager::GetInstance().CreateSamplerSet(SamplerPtrs, 3);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr, CSamplerPtr _Sampler3Ptr, CSamplerPtr _Sampler4Ptr)
    {
        CSamplerPtr SamplerPtrs[] = { _Sampler1Ptr, _Sampler2Ptr, _Sampler3Ptr, _Sampler4Ptr, };

        return CGfxSamplerManager::GetInstance().CreateSamplerSet(SamplerPtrs, 4);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CreateSamplerSet(CSamplerPtr* _pSamplerPtrs, unsigned int _NumberOfSamplers)
    {
        return CGfxSamplerManager::GetInstance().CreateSamplerSet(_pSamplerPtrs, _NumberOfSamplers);
    }

    // -----------------------------------------------------------------------------

    void SetSamplerLabel(CSamplerPtr _SamplerPtr, const char* _pLabel)
    {
        CGfxSamplerManager::GetInstance().SetSamplerLabel(_SamplerPtr, _pLabel);
    }
} // SamplerManager
} // namespace Gfx
