#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_PolarRect.h"

namespace Stereo
{
    //---Constructor & Destructor---
    PolarRect::PolarRect()
    {
    }

    PolarRect::PolarRect(const cv::Mat& Img_B, const cv::Mat& Img_M)
    {
        Img_B_Orig = Img_B;
        Img_M_Orig = Img_M;
    }

    PolarRect::~PolarRect()
    {
    }

    //---Main Function---
    void PolarRect::compute(const cv::Mat& F)
    {
        std::vector<cv::Point2f> EpiPoles(2);
        getEpipoles(F, EpiPoles[0], EpiPoles[1]);

        determ_CoRegion(EpiPoles, cv::Size(Img_B_Orig.cols, Img_B_Orig.rows), F);

        getTransformationPoints(Img_B_Orig.size(), EpiPoles[0], EpiPoles[1], F);

        doTransformation(Img_B_Orig, Img_M_Orig, EpiPoles[0], EpiPoles[1], F);

    }

    void PolarRect::gen_RectImg(int interpolation)
    {
        cv::remap(Img_B_Orig, Img_B_Rect, m_mapX1, m_mapY1, interpolation, cv::BORDER_TRANSPARENT);
        cv::remap(Img_M_Orig, Img_M_Rect, m_mapX2, m_mapY2, interpolation, cv::BORDER_TRANSPARENT);
    }

    void PolarRect::get_RectImg(cv::Mat& RectImg_B, cv::Mat& RectImg_M)
    {
        RectImg_B = Img_B_Rect;
        RectImg_M = Img_M_Rect;
    }

    inline void PolarRect::determ_CoRegion(const std::vector<cv::Point2f>& EpiPoles, const cv::Size ImgSize, const cv::Mat& F)
    {
        std::vector<cv::Point2f> externalPoints1, externalPoints2;
        getExternalPoints(EpiPoles[0], ImgSize, externalPoints1);
        getExternalPoints(EpiPoles[1], ImgSize, externalPoints2);

        cal_RhoRange(EpiPoles[0], ImgSize, externalPoints1, m_minRho1, m_maxRho1);
        cal_RhoRange(EpiPoles[1], ImgSize, externalPoints2, m_minRho2, m_maxRho2);

        if (!Is_InsideImg(EpiPoles[0], ImgSize) && !Is_InsideImg(EpiPoles[1], ImgSize))
        {
            // CASE 1: Both outside
            const cv::Vec3f line11 = get_ImgLn_from_ImgPt(EpiPoles[0], externalPoints1[0]);
            const cv::Vec3f line12 = get_ImgLn_from_ImgPt(EpiPoles[0], externalPoints1[1]);

            const cv::Vec3f line23 = get_ImgLn_from_ImgPt(EpiPoles[1], externalPoints2[0]);
            const cv::Vec3f line24 = get_ImgLn_from_ImgPt(EpiPoles[1], externalPoints2[1]);

            std::vector <cv::Vec3f> inputLines(2), outputLines(2);
            inputLines[0] = line23;
            inputLines[1] = line24;
            computeEpilines(externalPoints2, 2, F, inputLines, outputLines);
            const cv::Vec3f line13 = outputLines[0];
            const cv::Vec3f line14 = outputLines[1];

            inputLines[0] = line11;
            inputLines[1] = line12;
            computeEpilines(externalPoints1, 1, F, inputLines, outputLines);
            const cv::Vec3f line21 = outputLines[0];
            const cv::Vec3f line22 = outputLines[1];

            // Beginning and ending lines
            m_line1B = lineIntersectsRect(line13, ImgSize) ? line13 : line11;
            m_line1E = lineIntersectsRect(line14, ImgSize) ? line14 : line12;
            m_line2B = lineIntersectsRect(line21, ImgSize) ? line21 : line23;
            m_line2E = lineIntersectsRect(line22, ImgSize) ? line22 : line24;

            // Beginning and ending lines intersection with the borders
            {
                std::vector<cv::Point2d> intersections;
                getBorderIntersections(EpiPoles[0], m_line1B, ImgSize, intersections);
                double maxDist = std::numeric_limits<double>::min();
                for (uint32_t i = 0; i < intersections.size(); i++)
                {
                    const cv::Point2f intersect = intersections[i];
                    const double dist = cv::norm(EpiPoles[0] - intersect);
                    if (dist > maxDist)
                    {
                        maxDist = dist;
                        m_b1 = intersections[i];
                    }
                }
            }
            {
                std::vector<cv::Point2d> intersections;
                getBorderIntersections(EpiPoles[1], m_line2B, ImgSize, intersections);
                double maxDist = std::numeric_limits<double>::min();
                for (uint32_t i = 0; i < intersections.size(); i++)
                {
                    const cv::Point2f intersect = intersections[i];
                    const double dist = cv::norm(EpiPoles[1] - intersect);
                    if (dist > maxDist)
                    {
                        maxDist = dist;
                        m_b2 = intersections[i];
                    }
                }
            }
            {
                std::vector<cv::Point2d> intersections;
                getBorderIntersections(EpiPoles[0], m_line1E, ImgSize, intersections);
                double maxDist = std::numeric_limits<double>::min();
                for (uint32_t i = 0; i < intersections.size(); i++)
                {
                    const cv::Point2f intersect = intersections[i];
                    const double dist = cv::norm(EpiPoles[0] - intersect);
                    if (dist > maxDist)
                    {
                        maxDist = dist;
                        m_e1 = intersections[i];
                    }
                }
            }
            {
                std::vector<cv::Point2d> intersections;
                getBorderIntersections(EpiPoles[1], m_line2E, ImgSize, intersections);
                double maxDist = std::numeric_limits<double>::min();
                for (uint32_t i = 0; i < intersections.size(); i++)
                {
                    const cv::Point2f intersect = intersections[i];
                    const double dist = cv::norm(EpiPoles[1] - intersect);
                    if (dist > maxDist)
                    {
                        maxDist = dist;
                        m_e2 = intersections[i];
                    }
                }
            }
        }
        else if (Is_InsideImg(EpiPoles[0], ImgSize) && Is_InsideImg(EpiPoles[1], ImgSize)) 
        {
            // CASE 2: Both inside
            m_line1B = get_ImgLn_from_ImgPt(EpiPoles[0], externalPoints1[0]);
            m_line1E = m_line1B;

            std::vector <cv::Vec3f> inputLines(1), outputLines(1);
            inputLines[0] = m_line1B;
            computeEpilines(externalPoints1, 1, F, inputLines, outputLines);

            m_line2B = outputLines[0];
            m_line2E = outputLines[0];

            m_b1 = getBorderIntersection(EpiPoles[0], m_line1B, ImgSize);
            m_e1 = getBorderIntersection(EpiPoles[0], m_line1E, ImgSize);

            m_b2 = m_e2 = getNearestIntersection(EpiPoles[0], EpiPoles[1], m_line2B, m_b1, ImgSize);

        }
        else 
        {
            // CASE 3: One inside and one outside
            if (Is_InsideImg(EpiPoles[0], ImgSize)) 
            {
                // CASE 3.1: Only the first epipole is inside

                const cv::Vec3f line23 = get_ImgLn_from_ImgPt(EpiPoles[1], externalPoints2[0]);
                const cv::Vec3f line24 = get_ImgLn_from_ImgPt(EpiPoles[1], externalPoints2[1]);

                std::vector <cv::Vec3f> inputLines(2), outputLines(2);
                inputLines[0] = line23;
                inputLines[1] = line24;
                computeEpilines(externalPoints2, 2, F, inputLines, outputLines);
                const cv::Vec3f & line13 = outputLines[0];
                const cv::Vec3f & line14 = outputLines[1];

                m_line1B = line13;
                m_line1E = line14;
                m_line2B = line23;
                m_line2E = line24;

                m_b2 = getBorderIntersection(EpiPoles[1], m_line2B, ImgSize);
                m_e2 = getBorderIntersection(EpiPoles[1], m_line2E, ImgSize);

                m_b1 = getNearestIntersection(EpiPoles[1], EpiPoles[0], m_line1B, m_b2, ImgSize);
                m_e1 = getNearestIntersection(EpiPoles[1], EpiPoles[0], m_line1E, m_e2, ImgSize);
            }
            else 
            {
                // CASE 3.2: Only the second epipole is inside
                const cv::Vec3f line11 = get_ImgLn_from_ImgPt(EpiPoles[0], externalPoints1[0]);
                const cv::Vec3f line12 = get_ImgLn_from_ImgPt(EpiPoles[0], externalPoints1[1]);

                std::vector <cv::Vec3f> inputLines(2), outputLines(2);
                inputLines[0] = line11;
                inputLines[1] = line12;
                computeEpilines(externalPoints1, 1, F, inputLines, outputLines);
                const cv::Vec3f& line21 = outputLines[0];
                const cv::Vec3f& line22 = outputLines[1];

                m_line1B = line11;
                m_line1E = line12;
                m_line2B = line21;
                m_line2E = line22;

                m_b1 = getBorderIntersection(EpiPoles[0], m_line1B, ImgSize);
                m_e1 = getBorderIntersection(EpiPoles[0], m_line1E, ImgSize);

                m_b2 = getNearestIntersection(EpiPoles[0], EpiPoles[1], m_line2B, m_b1, ImgSize);
                m_e2 = getNearestIntersection(EpiPoles[0], EpiPoles[1], m_line2E, m_e1, ImgSize);

            }
        }

    }

    void PolarRect::getTransformationPoints(const cv::Size& ImgSize, const cv::Point2d EpiPole1, const cv::Point2d EpiPole2, const cv::Mat& F)
    {
        cv::Point2d p1 = m_b1, p2 = m_b2;
        cv::Vec3f line1 = m_line1B, line2 = m_line2B;

        m_thetaPoints1.clear();
        m_thetaPoints2.clear();
        m_thetaPoints1.reserve(2 * (ImgSize.width + ImgSize.height));
        m_thetaPoints2.reserve(2 * (ImgSize.width + ImgSize.height));

        int32_t crossesLeft = 0;
        if (Is_InsideImg(EpiPole1, ImgSize) && Is_InsideImg(EpiPole2, ImgSize))
            crossesLeft++;

        uint32_t thetaIdx = 0;
        double lastCrossProd = 0;

        while (true) 
        {
            m_thetaPoints1.push_back(p1);
            m_thetaPoints2.push_back(p2);
            //         transformLine(epipole1, p1, img1, thetaIdx, m_minRho1, m_maxRho1, m_mapX1, m_mapY1, m_inverseMapX1, m_inverseMapY1);
            //         transformLine(epipole2, p2, img2, thetaIdx, m_minRho2, m_maxRho2, m_mapX2, m_mapY2, m_inverseMapX2, m_inverseMapY2);

            cv::Vec3f v0(p1.x - EpiPole1.x, p1.y - EpiPole1.y, 1.0);
            v0 /= cv::norm(v0);
            cv::Point2d oldP1 = p1;

            getNewEpiline(EpiPole1, EpiPole2, ImgSize, F, p1, p2, line1, line2, p1, p2, line1, line2);

            // Check if we reached the end
            cv::Vec3f v1(p1.x - EpiPole1.x, p1.y - EpiPole1.y, 0.0);
            v1 /= cv::norm(v1);
            cv::Vec3f v2(m_e1.x - EpiPole1.x, m_e1.y - EpiPole1.y, 0.0);
            v2 /= cv::norm(v2);
            cv::Vec3f v3(oldP1.x - EpiPole1.x, oldP1.y - EpiPole1.y, 0.0);
            v3 /= cv::norm(v3);

            double crossProd = v1.cross(v2)[2];

            if (thetaIdx != 0) 
            {
                if ( ((lastCrossProd * crossProd) < 0) || (fabs(acos(v1.dot(-v3))) < 0.01) || (p1 == cv::Point2d(-1, -1)) )
                    crossesLeft--;

                if ((crossesLeft < 0)) 
                {
                    break;
                }
            }
            lastCrossProd = crossProd;
            thetaIdx++;
            
        }
        m_thetaPoints1.pop_back();
        m_thetaPoints2.pop_back();
    }

    void PolarRect::doTransformation(const cv::Mat& img1, const cv::Mat& img2, const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Mat& F)
    {
        const double rhoRange1 = m_maxRho1 - m_minRho1 + 1;
        const double rhoRange2 = m_maxRho2 - m_minRho2 + 1;

        const double rhoRange = std::max(rhoRange1, rhoRange2);

        m_mapX1 = cv::Mat::ones(m_thetaPoints1.size(), rhoRange, CV_32FC1) * -1;
        m_mapY1 = cv::Mat::ones(m_thetaPoints1.size(), rhoRange, CV_32FC1) * -1;
        m_mapX2 = cv::Mat::ones(m_thetaPoints2.size(), rhoRange, CV_32FC1) * -1;
        m_mapY2 = cv::Mat::ones(m_thetaPoints2.size(), rhoRange, CV_32FC1) * -1;

        m_inverseMapX1 = cv::Mat::ones(img1.rows, img1.cols, CV_32FC1) * -1;
        m_inverseMapY1 = cv::Mat::ones(img1.rows, img1.cols, CV_32FC1) * -1;
        m_inverseMapX2 = cv::Mat::ones(img1.rows, img1.cols, CV_32FC1) * -1;
        m_inverseMapY2 = cv::Mat::ones(img1.rows, img1.cols, CV_32FC1) * -1;

        for (uint32_t thetaIdx = 0; thetaIdx < m_thetaPoints1.size(); thetaIdx++) 
        {
            transformLine(epipole1, m_thetaPoints1[thetaIdx], img1, thetaIdx, m_minRho1, m_maxRho1, m_mapX1, m_mapY1, m_inverseMapX1, m_inverseMapY1);
            transformLine(epipole2, m_thetaPoints2[thetaIdx], img2, thetaIdx, m_minRho2, m_maxRho2, m_mapX2, m_mapY2, m_inverseMapX2, m_inverseMapY2);
        }
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

    inline void PolarRect::cal_RhoRange(const cv::Point2d& EpiPole, const cv::Size ImgSize, const std::vector<cv::Point2f>& ImgPt_Extern, double& minRho, double& maxRho)
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

    inline void PolarRect::getEpipoles(const cv::Mat& F, cv::Point2f& EpiPole1, cv::Point2f& EpiPole2)
    {
        cv::SVD svd(F);

        cv::Mat e1 = svd.vt.row(2);
        cv::Mat e2 = svd.u.col(2);

        EpiPole1 = cv::Point2f(e1.at<double>(0, 0) / e1.at<double>(0, 2), e1.at<double>(0, 1) / e1.at<double>(0, 2));
        EpiPole2 = cv::Point2f(e2.at<double>(0, 0) / e2.at<double>(2, 0), e2.at<double>(1, 0) / e2.at<double>(2, 0));
    }

    cv::Vec3f PolarRect::get_ImgLn_from_ImgPt(const cv::Point2d& ImgPt1, const cv::Point2d& ImgPt2)
    {
        // Image Line: a*x + b*y + c = 0
        // Using Vec3f to store a, b, c
        cv::Vec3f ImgLn = (ImgPt1.y - ImgPt2.y, ImgPt2.x - ImgPt1.x, ImgPt1.x * ImgPt2.y - ImgPt2.x * ImgPt1.y);
        return ImgLn;
    }

    inline void PolarRect::computeEpilines(const std::vector<cv::Point2f>& points, const uint32_t &whichImage, const cv::Mat & F, const std::vector<cv::Vec3f>& oldlines, std::vector<cv::Vec3f>& newLines)
    {
        cv::computeCorrespondEpilines(points, whichImage, F, newLines);

        for (uint32_t i = 0; i < oldlines.size(); i++) 
        {
            if (((oldlines[i][0] * newLines[i][0]) < 0) && ((oldlines[i][1] * newLines[i][1]) < 0)) 
            {
                newLines[i] *= -1;
            }
        }
    }

    inline void PolarRect::getNewPointAndLineSingleImage(const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Size & imgDimensions, const cv::Mat & F, const uint32_t & whichImage, const cv::Point2d & pOld1, const cv::Point2d & pOld2,
        cv::Vec3f & prevLine, cv::Point2d & pNew1, cv::Vec3f & newLine1, cv::Point2d & pNew2, cv::Vec3f & newLine2)
    {
        // We obtain vector v
        cv::Vec2f v;

        cv::Vec3f vBegin(m_b1.x - epipole1.x, m_b1.y - epipole1.y, 0.0);
        cv::Vec3f vCurr(pOld1.x - epipole1.x, pOld1.y - epipole1.y, 0.0);
        cv::Vec3f vEnd(m_e1.x - epipole1.x, m_e1.y - epipole1.y, 0.0);

        vBegin /= cv::norm(vBegin);
        vCurr /= cv::norm(vCurr);
        vEnd /= cv::norm(vEnd);

        if (Is_InsideImg(epipole1, imgDimensions)) 
        {
            if (Is_InsideImg(epipole2, imgDimensions)) 
            {
                v = cv::Vec2f(vCurr[1], -vCurr[0]);
            }
            else 
            {
                vBegin = cv::Vec3f(m_b2.x - epipole2.x, m_b2.y - epipole2.y, 0.0);
                vCurr = cv::Vec3f(pOld2.x - epipole2.x, pOld2.y - epipole2.y, 0.0);
                vEnd = cv::Vec3f(m_e2.x - epipole2.x, m_e2.y - epipole2.y, 0.0);

                vBegin /= cv::norm(vBegin);
                vCurr /= cv::norm(vCurr);
                vEnd /= cv::norm(vEnd);

                const cv::Vec3f vCross = vBegin.cross(vEnd);

                v = cv::Vec2f(vCurr[1], -vCurr[0]);
                if (vCross[2] > 0.0) {
                    v = -v;
                }
            }
        }
        else 
        {
            const cv::Vec3f vCross = vBegin.cross(vEnd);

            v = cv::Vec2f(vCurr[1], -vCurr[0]);
            if (vCross[2] > 0.0) 
            {
                v = -v;
            }
        }

        pNew1 = cv::Point2d(pOld1.x + v[0] * m_stepSize, pOld1.y + v[1] * m_stepSize);
        newLine1 = get_ImgLn_from_ImgPt(epipole1, pNew1);

        if (!Is_InsideImg(epipole1, imgDimensions)) 
        {
            pNew1 = getBorderIntersection(epipole1, newLine1, imgDimensions, &pOld1);
        }
        else 
        {
            pNew1 = getNearestIntersection(epipole1, epipole1, newLine1, pOld1, imgDimensions);
        }

        std::vector<cv::Point2f> points(1);
        points[0] = pNew1;
        std::vector<cv::Vec3f> inLines(1);
        inLines[0] = newLine1;
        std::vector<cv::Vec3f> outLines(1);
        computeEpilines(points, whichImage, F, inLines, outLines);
        newLine2 = outLines[0];

        if (!Is_InsideImg(epipole2, imgDimensions)) 
        {
            cv::Point2d tmpPoint = getBorderIntersection(epipole2, newLine2, imgDimensions, &pOld2);
            pNew2 = tmpPoint;
        }
        else {
            std::vector <cv::Point2d> intersections;
            getBorderIntersections(epipole2, newLine2, imgDimensions, intersections);
            pNew2 = intersections[0];

            double minDist = std::numeric_limits<double>::max();
            for (uint32_t i = 0; i < intersections.size(); i++) {
                const double dist = (pOld2.x - intersections[i].x) * (pOld2.x - intersections[i].x) +
                    (pOld2.y - intersections[i].y) * (pOld2.y - intersections[i].y);
                if (minDist > dist) {
                    minDist = dist;
                    pNew2 = intersections[i];
                }
            }
        }
    }

    inline void PolarRect::getNewEpiline(const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Size & imgDimensions, const cv::Mat & F, const cv::Point2d pOld1, const cv::Point2d pOld2,
                                         cv::Vec3f prevLine1, cv::Vec3f prevLine2, cv::Point2d & pNew1, cv::Point2d & pNew2, cv::Vec3f & newLine1, cv::Vec3f & newLine2)
    {
        getNewPointAndLineSingleImage(epipole1, epipole2, imgDimensions, F, 1, pOld1, pOld2, prevLine1, pNew1, newLine1, pNew2, newLine2);

        //TODO If the distance is too big in image 2, we do it in the opposite sense
    //     double distImg2 = (pOld2.x - pNew2.x) * (pOld2.x - pNew2.x) + (pOld2.y - pNew2.y) * (pOld2.y - pNew2.y);
    //     if (distImg2 > m_stepSize * m_stepSize)
    //         getNewPointAndLineSingleImage(epipole2, epipole1, imgDimensions, F, 2, pOld2, pOld1, prevLine2, pNew2, newLine2, pNew1, newLine1);
    }

    void PolarRect::transformLine(const cv::Point2d& epipole, const cv::Point2d& p2, const cv::Mat& inputImage, const uint32_t & thetaIdx, const double &minRho, const double & maxRho, cv::Mat& mapX, cv::Mat& mapY, cv::Mat& inverseMapX, cv::Mat& inverseMapY)
    {
        cv::Vec2f v(p2.x - epipole.x, p2.y - epipole.y);
        double maxDist = cv::norm(v);
        v /= maxDist;
            
        {
            uint32_t rhoIdx = 0;
            for (double rho = minRho; rho <= std::min(maxDist, maxRho); rho += 1.0, rhoIdx++) 
            {
                cv::Point2d target(v[0] * rho + epipole.x, v[1] * rho + epipole.y);
                if ((target.x >= 0) && (target.x < inputImage.cols) &&
                    (target.y >= 0) && (target.y < inputImage.rows)) 
                {
                    mapX.at<float>(thetaIdx, rhoIdx) = target.x;
                    mapY.at<float>(thetaIdx, rhoIdx) = target.y;

                    inverseMapX.at<float>(target.y, target.x) = rhoIdx;
                    inverseMapY.at<float>(target.y, target.x) = thetaIdx;
                }
            }
        }
    }

    inline bool PolarRect::lineIntersectsRect(const cv::Vec3d& line, const cv::Size& imgDimensions, cv::Point2d* intersection)
    {
        return 
            lineIntersectsSegment(line, cv::Point2d(0, 0), cv::Point2d(imgDimensions.width - 1, 0), intersection) ||
            lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, 0), cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), intersection) ||
            lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), cv::Point2d(0, imgDimensions.height - 1), intersection) ||
            lineIntersectsSegment(line, cv::Point2d(0, imgDimensions.height - 1), cv::Point2d(0, 0), intersection);
    }

    inline bool PolarRect::lineIntersectsSegment(const cv::Vec3d & line, const cv::Point2d & p1, const cv::Point2d & p2, cv::Point2d * intersection)
    {
        const cv::Vec3d segment = get_ImgLn_from_ImgPt(p1, p2);

        if (intersection != NULL)
            *intersection = cv::Point2d(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

        // Lines are represented as ax + by + c = 0, so
        // y = -(ax+c)/b. If y1=y2, then we have to obtain x, which is
        // x = (b1 * c2 - b2 * c1) / (b2 * a1 - b1 * a2)
        if ((segment[1] * line[0] - line[1] * segment[0]) == 0)
            { return false; }
        double x = (line[1] * segment[2] - segment[1] * line[2]) / (segment[1] * line[0] - line[1] * segment[0]);
        double y = -(line[0] * x + line[2]) / line[1];

        if (((int32_t)round(x) >= (int32_t)std::min(p1.x, p2.x)) && ((int32_t)round(x) <= (int32_t)std::max(p1.x, p2.x))) 
        {
            if (((int32_t)round(y) >= (int32_t)std::min(p1.y, p2.y)) && ((int32_t)round(y) <= (int32_t)std::max(p1.y, p2.y))) 
            {
                if (intersection != NULL)
                    *intersection = cv::Point2d(x, y);

                return true;
            }
        }

        return false;
    }

    inline cv::Point2d PolarRect::getBorderIntersection(const cv::Point2d& epipole, const cv::Vec3d& line, const cv::Size& imgDimensions, const cv::Point2d* lastPoint)
    {
        cv::Point2d intersection(-1, -1);

        if (Is_InsideImg(epipole, imgDimensions)) 
        {
            if (lineIntersectsSegment(line, cv::Point2d(0, 0), cv::Point2d(imgDimensions.width - 1, 0), &intersection)) 
            {
                if (Is_TheRightPoint(epipole, intersection, line, lastPoint)) {

                    return intersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, 0), cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), &intersection)) 
            {
                if (Is_TheRightPoint(epipole, intersection, line, lastPoint)) 
                {
                    return intersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), cv::Point2d(0, imgDimensions.height - 1), &intersection)) 
            {
                if (Is_TheRightPoint(epipole, intersection, line, lastPoint)) 
                {
                    return intersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(0, imgDimensions.height - 1), cv::Point2d(0, 0), &intersection)) 
            {
                if (Is_TheRightPoint(epipole, intersection, line, lastPoint)) 
                {
                    return intersection;
                }
            }
        }
        else {
            double maxDist = std::numeric_limits<double>::min();
            cv::Point2d tmpIntersection(-1, -1);
            if (lineIntersectsSegment(line, cv::Point2d(0, 0), cv::Point2d(imgDimensions.width - 1, 0), &tmpIntersection)) 
            {
                const double dist2 = (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x) + (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x);

                if (dist2 > maxDist) {
                    maxDist = dist2;
                    intersection = tmpIntersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, 0), cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), &tmpIntersection)) 
            {
                const double dist2 = (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x) + (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x);

                if (dist2 > maxDist) 
                {
                    maxDist = dist2;
                    intersection = tmpIntersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), cv::Point2d(0, imgDimensions.height - 1), &tmpIntersection)) 
            {
                const double dist2 = (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x) + (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x);

                if (dist2 > maxDist) 
                {
                    maxDist = dist2;
                    intersection = tmpIntersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(0, imgDimensions.height - 1), cv::Point2d(0, 0), &tmpIntersection)) 
            {
                const double dist2 = (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x) + (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x);

                if (dist2 > maxDist) 
                {
                    maxDist = dist2;
                    intersection = tmpIntersection;
                }
            }
            return intersection;
        }
    }

    inline void PolarRect::getBorderIntersections(const cv::Point2d& epipole, const cv::Vec3d& line, const cv::Size& imgDimensions, std::vector<cv::Point2d>& intersections)
    {
        cv::Point2d intersection(-1, -1);
        intersections.reserve(2);

        if (lineIntersectsSegment(line, cv::Point2d(0, 0), cv::Point2d(imgDimensions.width - 1, 0), &intersection)) 
        {
            intersections.push_back(intersection);
        }
        if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, 0), cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), &intersection)) 
        {
            intersections.push_back(intersection);
        }
        if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), cv::Point2d(0, imgDimensions.height - 1), &intersection)) 
        {
            intersections.push_back(intersection);
        }
        if (lineIntersectsSegment(line, cv::Point2d(0, imgDimensions.height - 1), cv::Point2d(0, 0), &intersection)) 
        {
            intersections.push_back(intersection);
        }
    }

    inline cv::Point2d PolarRect::getNearestIntersection(const cv::Point2d& oldEpipole, const cv::Point2d& newEpipole, const cv::Vec3d& line, const cv::Point2d& oldPoint, const cv::Size& imgDimensions)
    {
        std::vector<cv::Point2d> intersections;
        getBorderIntersections(newEpipole, line, imgDimensions, intersections);

        double minAngle = std::numeric_limits<double>::max();
        cv::Point2d point(-1, -1);

        cv::Vec3d v1(oldPoint.x - oldEpipole.x, oldPoint.y - oldEpipole.y, 0.0);
        v1 /= cv::norm(v1);

        for (uint32_t i = 0; i < intersections.size(); i++) 
        {
            cv::Vec3d v(intersections[i].x - newEpipole.x, intersections[i].y - newEpipole.y, 0.0);
            v /= cv::norm(v);

            const double & angle = fabs(acos(v.dot(v1)));

            if (angle < minAngle) 
            {
                minAngle = angle;
                point = intersections[i];
            }
        }

        return point;
    }

    inline bool PolarRect::Is_InsideImg(cv::Point2d ImgPt, cv::Size ImgSize)
    {
        return ((ImgPt.x >= 0) && (ImgPt.y >= 0) && (ImgPt.x < (ImgSize.width - 1.0)) && (ImgPt.y <= (ImgSize.height - 1.0)));
    }

    inline bool PolarRect::Is_TheRightPoint(const cv::Point2d & epipole, const cv::Point2d & intersection, const cv::Vec3d & line, const cv::Point2d * lastPoint)
    {
        if (lastPoint != NULL) 
        {
            cv::Vec3f v1(lastPoint->x - epipole.x, lastPoint->y - epipole.y, 0.0);
            v1 /= cv::norm(v1);
            cv::Vec3f v2(intersection.x - epipole.x, intersection.y - epipole.y, 0.0);
            v2 /= cv::norm(v2);

            if (fabs(acos(v1.dot(v2))) > CV_PI / 2.0)
                return false;
            else
                return true;
        }
        else 
        {
            if ((line[0] > 0) && (epipole.y < intersection.y)) return false;
            if ((line[0] < 0) && (epipole.y > intersection.y)) return false;
            if ((line[1] > 0) && (epipole.x > intersection.x)) return false;
            if ((line[1] < 0) && (epipole.x < intersection.x)) return false;

            return true;
        }
        return false;
    }

} // Stereo
