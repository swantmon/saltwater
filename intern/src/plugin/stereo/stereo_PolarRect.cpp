#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_PolarRect.h"

namespace Stereo
{
    //---Constructor & Destructor---
    PolarRect::PolarRect()
    {
    }


    PolarRect::~PolarRect()
    {
    }

    //---Main Function---
    inline void PolarRect::determ_CoRegion(const std::vector<cv::Point2f>& epipoles, const cv::Size imgDimensions, const cv::Mat& F)
    {
        std::vector<cv::Point2f> externalPoints1, externalPoints2;
        getExternalPoints(epipoles[0], imgDimensions, externalPoints1);
        getExternalPoints(epipoles[1], imgDimensions, externalPoints2);

        determ_RhoRange(epipoles[0], imgDimensions, externalPoints1, m_minRho1, m_maxRho1);
        determ_RhoRange(epipoles[1], imgDimensions, externalPoints2, m_minRho2, m_maxRho2);


    }

    //---Assist Function---
    void PolarRect::getExternalPoints(const cv::Point2d& EpiPole, const cv::Size ImgSize, std::vector<cv::Point2f>& ImgPt_Extern)
    {
        if (EpiPole.y < 0) 
        {
            if (EpiPole.x < 0) // Region 1
            {
                ImgPt_Extern.resize(2);
                ImgPt_Extern[0] = cv::Point2f(ImgSize.width - 1, 0);
                ImgPt_Extern[1] = cv::Point2f(0, ImgSize.height - 1);
            }
            else if (EpiPole.x <= ImgSize.width - 1) // Region 2
            { 
                ImgPt_Extern.resize(2);
                ImgPt_Extern[0] = cv::Point2f(ImgSize.width - 1, 0);
                ImgPt_Extern[1] = cv::Point2f(0, 0);
            }
            else // Region 3
            { 
                ImgPt_Extern.resize(2);
                ImgPt_Extern[0] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
                ImgPt_Extern[1] = cv::Point2f(0, 0);
            }
        }
        else if (EpiPole.y <= ImgSize.height - 1)
        { 
            if (EpiPole.x < 0) // Region 4
            { 
                ImgPt_Extern.resize(2);
                ImgPt_Extern[0] = cv::Point2f(0, 0);
                ImgPt_Extern[1] = cv::Point2f(0, ImgSize.height - 1);
            }
            else if (EpiPole.x <= ImgSize.width - 1) // Region 5
            { 
                ImgPt_Extern.resize(4);
                ImgPt_Extern[0] = cv::Point2f(0, 0);
                ImgPt_Extern[1] = cv::Point2f(ImgSize.width - 1, 0);
                ImgPt_Extern[2] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
                ImgPt_Extern[3] = cv::Point2f(0, ImgSize.height - 1);
            }
            else // Region 6
            { 
                ImgPt_Extern.resize(2);
                ImgPt_Extern[0] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
                ImgPt_Extern[1] = cv::Point2f(ImgSize.width - 1, 0);
            }
        }
        else
        { 
            if (EpiPole.x < 0) // Region 7
            {
                ImgPt_Extern.resize(2);
                ImgPt_Extern[0] = cv::Point2f(0, 0);
                ImgPt_Extern[1] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
            }
            else if (EpiPole.x <= ImgSize.width - 1) // Region 8
            { 
                ImgPt_Extern.resize(2);
                ImgPt_Extern[0] = cv::Point2f(0, ImgSize.height - 1);
                ImgPt_Extern[1] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
            }
            else // Region 9
            { 
                ImgPt_Extern.resize(2);
                ImgPt_Extern[0] = cv::Point2f(0, ImgSize.height - 1);
                ImgPt_Extern[1] = cv::Point2f(ImgSize.width - 1, 0);
            }
        }
    }

    inline void PolarRect::determ_RhoRange(const cv::Point2d& EpiPole, const cv::Size ImgSize, const std::vector<cv::Point2f>& ImgPt_Extern, double& minRho, double& maxRho)
    {
        if (EpiPole.y < 0) 
        { 
            if (EpiPole.x < 0) // Region 
            { 
                minRho = sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y);         // Point A
                maxRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y));        // Point D
            }
            else if (EpiPole.x <= ImgSize.width - 1) // Region 2
            { 
                minRho = -EpiPole.y;
                maxRho = std::max(
                                sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)),        // Point C
                                sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y))        // Point D
                                 );
            }
            else // Region 3
            { 
                minRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y);        // Point B
                maxRho = sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y));        // Point C
            }
        }
        else if (EpiPole.y <= ImgSize.height - 1) 
        { 
            if (EpiPole.x < 0) // Region 4
            { 
                minRho = -EpiPole.x;
                maxRho = std::max(
                                sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)),        // Point D
                                sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y)        // Point B
                                 );
            }
            else if (EpiPole.x <= ImgSize.width - 1) // Region 5
            { 
                minRho = 0;
                maxRho = std::max(
                            std::max(
                                    sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y),        // Point A
                                    sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y)        // Point B
                                    ),
                            std::max(
                                    sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)),        // Point C
                                    sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y))        // Point D
                                    )
                                 );
            }
            else // Region 6
            { 
                minRho = EpiPole.x - (ImgSize.width - 1);
                maxRho = std::max(
                                sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y),        // Point A
                                sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y))        // Point C
                                 );
            }
        }
        else 
        { 
            if (EpiPole.x < 0) // Region 7
            { 
                minRho = sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y));        // Point C
                maxRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y);        // Point B
            }
            else if (EpiPole.x <= ImgSize.width - 1) // Region 8
            { 
                minRho = EpiPole.y - (ImgSize.height - 1);
                maxRho = std::max(
                                sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y),        // Point A
                                sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y)        // Point B
                                 );
            }
            else // Region 9
            { 
                minRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y));        // Point D
                maxRho = sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y);        // Point A
            }
        }
    }

}
