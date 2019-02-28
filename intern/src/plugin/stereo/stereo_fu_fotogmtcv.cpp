#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_fu_fotogmtcv.h"


namespace Stereo
{
    Fu_FotoGmtCV::Fu_FotoGmtCV()
    {
    }
    
    Fu_FotoGmtCV::Fu_FotoGmtCV(const std::vector<char>& InputImg)
    {
        FuImg = InputImg; // 1D vector stores 2D image in row-oriented
    }
    Fu_FotoGmtCV::~Fu_FotoGmtCV()
    {
    }

    void Fu_FotoGmtCV::setPmtx(glm::mat3x4 P) // P is the transform from Mapping frame to Image frame
    {
        P_mtx = P;
    }

} // Stereo
