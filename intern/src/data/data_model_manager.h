//
//  data_model_manager.hpp
//  data
//
//  Created by Tobias Schwandt on 22/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Dt
{
    class CEntity;
    class CModel;
} // namespace Dt

namespace Dt
{
    struct SGeneratorFlag
    {
        enum
        {
            Nothing                  ,
            Default                  ,
            DefaultFlipUVs           ,
            RealtimeFast             ,
            NumberOfGenerationPresets,
        };
    };
} // namespace Dt

namespace Dt
{
    struct SModelDescriptor
    {
        const Base::Char* m_pFileName;
        unsigned int      m_GenFlag;
    };
} // namespace Dt

namespace Dt
{
namespace ModelManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CModel& CreateModel(const SModelDescriptor& _rDescriptor);
    CModel& CreateBox(float _Width, float _Height, float _Depth);
    CModel& CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
    CModel& CreateCone(float _Radius, float _Height, unsigned int _Slices);
    CModel& CreateRectangle(float _X, float _Y, float _Width, float _Height);

    void FreeModel(CModel& _rModel);
} // namespace ModelManager
} // namespace Dt