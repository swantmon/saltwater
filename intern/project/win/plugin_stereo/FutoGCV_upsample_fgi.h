#pragma once

#include "base/base_include_glm.h" // Some warnings appears when directly #include "glm" in Engine

namespace FutoGCV
{
    struct SFGIParam 
    {
        glm::vec2 m_Lamda = glm::vec2(900.0f, 100.0f);
        glm::vec2 m_Sigma = glm::vec2(0.005f, 0.005f);
    };

    class CFGI
    {
    //---Constructors & Destructor---
    public:
        CFGI();
        ~CFGI();

    //---Execute Functions---
    public:

    //---Assist Functions---
    private:
        void FGS();
    
    //---Members---
    private:

    };
} // namespace FutoGCV


