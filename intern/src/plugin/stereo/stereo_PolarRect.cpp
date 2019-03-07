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
    inline void PolarRect::determ_CoRegion(const std::vector<cv::Point2f>& epipoles, const cv::Size imgDimensions, const cv::Mat & F)
    {
        std::vector<cv::Point2f> externalPoints1, externalPoints2;
        getExternalPoints(epipoles[0], imgDimensions, externalPoints1);
        getExternalPoints(epipoles[1], imgDimensions, externalPoints2);

        determ_RhoRange(epipoles[0], imgDimensions, externalPoints1, m_minRho1, m_maxRho1);
        determ_RhoRange(epipoles[1], imgDimensions, externalPoints2, m_minRho2, m_maxRho2);


    }

    //---Assist Function---
    void PolarRect::getExternalPoints(const cv::Point2d& epipole, const cv::Size imgDimensions, std::vector<cv::Point2f>& externalPoints)
    {
        if (epipole.y < 0) // Cases 1, 2 and 3
        {
            if (epipole.x < 0) // Case 1
            {
                externalPoints.resize(2);
                externalPoints[0] = cv::Point2f(imgDimensions.width - 1, 0);
                externalPoints[1] = cv::Point2f(0, imgDimensions.height - 1);
            }
            else if (epipole.x <= imgDimensions.width - 1) // Case 2
            { 
                externalPoints.resize(2);
                externalPoints[0] = cv::Point2f(imgDimensions.width - 1, 0);
                externalPoints[1] = cv::Point2f(0, 0);
            }
            else // Case 3
            { 
                externalPoints.resize(2);
                externalPoints[0] = cv::Point2f(imgDimensions.width - 1, imgDimensions.height - 1);
                externalPoints[1] = cv::Point2f(0, 0);
            }
        }
        else if (epipole.y <= imgDimensions.height - 1) // Cases 4, 5 and 6
        { 
            if (epipole.x < 0) // Case 4
            { 
                externalPoints.resize(2);
                externalPoints[0] = cv::Point2f(0, 0);
                externalPoints[1] = cv::Point2f(0, imgDimensions.height - 1);
            }
            else if (epipole.x <= imgDimensions.width - 1) // Case 5
            { 
                externalPoints.resize(4);
                externalPoints[0] = cv::Point2f(0, 0);
                externalPoints[1] = cv::Point2f(imgDimensions.width - 1, 0);
                externalPoints[2] = cv::Point2f(imgDimensions.width - 1, imgDimensions.height - 1);
                externalPoints[3] = cv::Point2f(0, imgDimensions.height - 1);
            }
            else // Case 6
            { 
                externalPoints.resize(2);
                externalPoints[0] = cv::Point2f(imgDimensions.width - 1, imgDimensions.height - 1);
                externalPoints[1] = cv::Point2f(imgDimensions.width - 1, 0);
            }
        }
        else // Cases 7, 8 and 9
        { 
            if (epipole.x < 0) // Case 7
            {
                externalPoints.resize(2);
                externalPoints[0] = cv::Point2f(0, 0);
                externalPoints[1] = cv::Point2f(imgDimensions.width - 1, imgDimensions.height - 1);
            }
            else if (epipole.x <= imgDimensions.width - 1) // Case 8
            { 
                externalPoints.resize(2);
                externalPoints[0] = cv::Point2f(0, imgDimensions.height - 1);
                externalPoints[1] = cv::Point2f(imgDimensions.width - 1, imgDimensions.height - 1);
            }
            else // Case 9
            { 
                externalPoints.resize(2);
                externalPoints[0] = cv::Point2f(0, imgDimensions.height - 1);
                externalPoints[1] = cv::Point2f(imgDimensions.width - 1, 0);
            }
        }
    }

    void PolarRect::determ_RhoRange(const cv::Point2d& epipole, const cv::Size imgDimensions, const std::vector<cv::Point2f>& externalPoints, double& minRho, double& maxRho)
    {
        if (epipole.y < 0) { // Cases 1, 2 and 3
            if (epipole.x < 0) { // Case 1
                minRho = sqrt(epipole.x * epipole.x + epipole.y * epipole.y);         // Point A
                maxRho = sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                    ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y));        // Point D
            }
            else if (epipole.x <= imgDimensions.width - 1) { // Case 2
                minRho = -epipole.y;
                maxRho = max(sqrt(epipole.x * epipole.x +
                    ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y)),        // Point C
                    sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                    ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y))        // Point D
                );
            }
            else { // Case 3
                minRho = sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                    epipole.y * epipole.y);        // Point B
                maxRho = sqrt(epipole.x * epipole.x +
                    ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y));        // Point C
            }
        }
        else if (epipole.y <= imgDimensions.height - 1) { // Cases 4, 5 and 6
            if (epipole.x < 0) { // Case 4
                minRho = -epipole.x;
                maxRho = max(
                    sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                    ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y)),        // Point D
                    sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                        epipole.y * epipole.y)        // Point B
                );
            }
            else if (epipole.x <= imgDimensions.width - 1) { // Case 5
                minRho = 0;
                maxRho = max(
                    max(
                        sqrt(epipole.x * epipole.x + epipole.y * epipole.y),        // Point A
                        sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                            epipole.y * epipole.y)        // Point B
                    ),
                    max(
                        sqrt(epipole.x * epipole.x +
                        ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y)),        // Point C
                        sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                        ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y))        // Point D
                    )
                );
            }
            else { // Case 6
                minRho = epipole.x - (imgDimensions.width - 1);
                maxRho = max(
                    sqrt(epipole.x * epipole.x + epipole.y * epipole.y),        // Point A
                    sqrt(epipole.x * epipole.x +
                    ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y))        // Point C
                );
            }
        }
        else { // Cases 7, 8 and 9
            if (epipole.x < 0) { // Case 7
                minRho = sqrt(epipole.x * epipole.x +
                    ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y));        // Point C
                maxRho = sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                    epipole.y * epipole.y);        // Point B
            }
            else if (epipole.x <= imgDimensions.width - 1) { // Case 8
                minRho = epipole.y - (imgDimensions.height - 1);
                maxRho = max(
                    sqrt(epipole.x * epipole.x + epipole.y * epipole.y),        // Point A
                    sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                        epipole.y * epipole.y)        // Point B

                );
            }
            else { // Case 9
                minRho = sqrt(((imgDimensions.width - 1) - epipole.x) * ((imgDimensions.width - 1) - epipole.x) +
                    ((imgDimensions.height - 1) - epipole.y) * ((imgDimensions.height - 1) - epipole.y));        // Point D
                maxRho = sqrt(epipole.x * epipole.x + epipole.y * epipole.y);        // Point A
            }
        }
    }

}
