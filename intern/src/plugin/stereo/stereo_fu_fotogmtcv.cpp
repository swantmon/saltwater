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
} // Stereo
