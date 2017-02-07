
#pragma once

#include "base/base_typedef.h"

#include "data/data_lod.h"

#include <string>

namespace Dt
{
    class CMesh
    {
    public:

        static const unsigned int s_NumberOfLODs = 4;

    public:

        CMesh();
        ~CMesh();

        const std::string& GetMeshname() const;

        void SetNumberOfLODs(unsigned int _NumberOfLODs);
        unsigned int GetNumberOfLODs() const;

        void SetLOD(unsigned int _Index, CLOD* _pLOD);
        CLOD* GetLOD(unsigned int _Index);
        const CLOD* GetLOD(unsigned int _Index) const;

    protected:

        std::string  m_Meshname;
        unsigned int m_NumberOfLODs;
        CLOD*        m_LODs[s_NumberOfLODs];
    };
} // namespace Dt
