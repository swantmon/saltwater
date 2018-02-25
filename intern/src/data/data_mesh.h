
#pragma once

#include "base/base_typedef.h"

#include <string>

namespace Dt
{
    class CMesh
    {
    public:

        struct SGeneratorFlag
        {
            enum
            {
                Nothing      = 0x00,
                Default      = 0x01,
                FlipUVs      = 0x02,
                RealtimeFast = 0x04
            };
        };

        enum EPredefinedMesh
        {
            Nothing,
            Box,
            Sphere,
            IsometricSphere,
            Cone,
            Rectangle,
        };

    public:

        CMesh();
        ~CMesh();

        const std::string& GetFilename() const;

        int GetGeneratorFlag() const;

        EPredefinedMesh GetPredefinedMesh() const;
        
    protected:
        
        std::string     m_Filename;
        int             m_GeneratorFlag;
        EPredefinedMesh m_PredefinedMesh;
    };
} // namespace Dt
