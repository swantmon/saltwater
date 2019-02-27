#pragma once

#ifndef Photogrammmetry_H
#define Photogrammmetry_H

#include <vector>

#include "base/base_include_glm.h"

namespace Stereo
{
    class Photogrammmetry
    {
    public:
        Photogrammmetry();
        ~Photogrammmetry();
        
    public:
        
    private:
        std::vector<char> Img;
        glm::mat3 K_mtx;
        glm::mat3x4 P_mtx;
    };
} // Stereo

#endif
