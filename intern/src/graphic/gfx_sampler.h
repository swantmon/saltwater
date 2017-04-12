
#pragma once

#include "base/base_managed_pool.h"

namespace Gfx
{
    class CSampler : public Base::CManagedPoolItemBase
    {
        public:

            enum EFilter
            {
                MinMagMipPoint                       =  0,
                MinMagPointMipLinear                 =  1,
                MinPointMagLinearMipPoint            =  2,
                MinPointMagMipLinear                 =  3,
                MinLinearMagMipPoint                 =  4,
                MinLinearMagPointMipLinear           =  5,
                MinMagLinearMipPoint                 =  6,
                MinMagMipLinear                      =  7,
                MinMagMipPointCompare                =  8,
                MinMagPointMipLinearCompare          =  9,
                MinPointMagLinearMipPointCompare     = 10,
                MinPointMagMipLinearCompare          = 11,
                MinLinearMagMipPointCompare          = 12,
                MinLinearMagPointMipLinearCompare    = 13,
                MinMagLinearMipPointCompare          = 14,
                MinMagMipLinearCompare               = 15,
                NumberOfFilters                      = 16,
                UndefinedFilter                      = -1,
            };

            enum EAddressMode
            {
                Wrap                                 =  0,
                Mirror                               =  1,
                Clamp                                =  2,
                Border                               =  3,
                NumberOfAddressModi                  =  4,
                UndefinedAddressMode                 = -1,
            };

            enum EComparisonFunction
            {
                Never                                =  0,
                Less                                 =  1,
                Equal                                =  2,
                LesserEqual                          =  3,
                Greater                              =  4,
                Unequal                              =  5,
                GreaterEqual                         =  6,
                Always                               =  7,
                NumberOfComparisonFunctions          =  8,
                UndefinedComparisonFunction          = -1,
            };

            enum ESampler
            {
                MinMagMipPointClamp,
                MinMagMipPointWrap,
                MinMagMipPointBorder,
                MinMagMipPointMirror,
                MinMagLinearMipPointClamp,
                MinMagLinearMipPointWrap,
                MinMagLinearMipPointBorder,
                MinMagLinearMipPointMirror,
                MinMagMipLinearClamp,
                MinMagMipLinearWrap,
                MinMagMipLinearBorder,
                MinMagMipLinearMirror,
                PCF,
                NumberOfSamplers,
                UndefinedSampler = -1,
            };

        public:

            EFilter GetFilter() const;

            EAddressMode GetAddressModeU() const;
            EAddressMode GetAddressModeV() const;
            EAddressMode GetAddressModeW() const;

            EComparisonFunction GetComparisonFunction() const;

            ESampler GetType() const;

        protected:

            EFilter             m_Filter;
            EAddressMode        m_AddressMode[3];
            EComparisonFunction m_ComparisonFunction;
            ESampler            m_Type;

        protected:

            CSampler();
           ~CSampler();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CSampler> CSamplerPtr;
} // namespace Gfx
