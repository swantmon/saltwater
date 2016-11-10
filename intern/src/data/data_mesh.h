
#pragma once

#include "base/base_string.h"
#include "base/base_typedef.h"

#include "data/data_lod.h"

namespace Dt
{
    class CMesh
    {
    public:

        static const unsigned int s_NumberOfLODs = 4;

    public:

        CMesh();
        ~CMesh();

        void SetMeshname(const char* _pMeshname);
        const char* GetMeshname() const;

        void SetNumberOfLODs(unsigned int _NumberOfLODs);
        unsigned int GetNumberOfLODs() const;

        void SetLOD(unsigned int _Index, CLOD* _pLOD);
        CLOD* GetLOD(unsigned int _Index);
        const CLOD* GetLOD(unsigned int _Index) const;

    protected:

        Base::CharString m_Meshname;
        unsigned int     m_NumberOfLODs;
        CLOD*            m_LODs[s_NumberOfLODs];
    };
} // namespace Dt
