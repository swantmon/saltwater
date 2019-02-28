#pragma once

#ifndef Fu_FotoGmtCV_H
#define Fu_FotoGmtCV_H

#include <vector>

#include "base/base_include_glm.h" // Some warnings appears when directly #include glm 

namespace Stereo
{
    class Fu_FotoGmtCV
    {
    public:
        Fu_FotoGmtCV();
        Fu_FotoGmtCV(const std::vector<char>&);
        ~Fu_FotoGmtCV();
        
    public:
        void setPmtx(glm::mat4x3 P);

    private:
        std::vector<char> FuImg;
        glm::mat3 K_mtx;
        glm::mat4x3 P_mtx;
    };
} // Stereo

#endif
