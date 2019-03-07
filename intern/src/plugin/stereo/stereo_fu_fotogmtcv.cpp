#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_fu_fotogmtcv.h"


namespace Stereo
{
    Fu_FotoGmtCV::Fu_FotoGmtCV()
    {
    }

    Fu_FotoGmtCV::Fu_FotoGmtCV(const std::vector<char>& Img_Input, int ImgW, int ImgH)
    {
        // Latter apply switch for different types of images
        cv::Mat Img_Input_cv(cv::Size(ImgW, ImgH), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
        memcpy(Img_Input_cv.data, Img_Input.data(), Img_Input.size());
        cv::cvtColor(Img_Input_cv, Img_Input_cv, cv::COLOR_BGRA2RGBA); // Transform image from BGRA (default of OpenCV) to RGB
        Img = Img_Input_cv; // 1D vector stores 2D image in row-oriented
    }
    Fu_FotoGmtCV::~Fu_FotoGmtCV()
    {
    }

    //---Polar Rectification---
    cv::Mat Fu_FotoGmtCV::PolarRect(Fu_FotoGmtCV Img_Match) // Apply Polar Rectification proposed by Pollefeys et al. (1999)
    {
        //---Derive F-mtx by known P-mtx---
            // E = B * R_1to2 = R_2to1 * B
            // F = inv(K2)' * E * inv(K)
            // F = [epipole2]_skewSym * P2 * inv(P1) = [P2*PC1]_skewSym * P2 * inv(P1)

        cv::Mat P_ImgB_PsudoInv;
        cv::invert(P_mtx, P_ImgB_PsudoInv, cv::DECOMP_SVD);
        cv::Mat PC = cv::Mat(4, 1, CV_32F);
        PC.at<float>(0, 0) = Trans_vec.at<float>(0, 0);
        PC.at<float>(1, 0) = Trans_vec.at<float>(1, 0);
        PC.at<float>(2, 0) = Trans_vec.at<float>(2, 0);
        PC.at<float>(3, 0) = 1;
        cv::Mat EpiPole_ImgM = Img_Match.P_mtx * PC; // Epipole of Image_Match
        cv::Mat Epipole_ImgM_SkewSym = cv::Mat::zeros(cv::Size(3, 3), CV_16F);
        Epipole_ImgM_SkewSym.at<float>(0, 1) = -EpiPole_ImgM.at<float>(2, 0);
        Epipole_ImgM_SkewSym.at<float>(0, 2) = EpiPole_ImgM.at<float>(1, 0);
        Epipole_ImgM_SkewSym.at<float>(1, 0) = EpiPole_ImgM.at<float>(2, 0);
        Epipole_ImgM_SkewSym.at<float>(1, 2) = -EpiPole_ImgM.at<float>(0, 0);
        Epipole_ImgM_SkewSym.at<float>(2, 0) = -EpiPole_ImgM.at<float>(1, 0);
        Epipole_ImgM_SkewSym.at<float>(2, 1) = EpiPole_ImgM.at<float>(0, 0);

        cv::Mat F_mtx = Epipole_ImgM_SkewSym * Img_Match.P_mtx * P_ImgB_PsudoInv;

        cv::SVD F_svd(F_mtx);
        cv::Mat EpiPole_ImgB = F_svd.vt.row(2);
        std::vector<cv::Point2f> EpiPoles(2);
        EpiPoles[0] = cv::Point2d(EpiPole_ImgB.at<float>(0, 0) / EpiPole_ImgB.at<float>(0, 2), EpiPole_ImgB.at<float>(0, 1) / EpiPole_ImgB.at<float>(0, 2));
        //cv::Mat e2 = F_svd.u.col(2);
        //cv::Point2f EpiPole_M = cv::Point2d(e2.at<float>(0, 0) / e2.at<float>(2, 0), e2.at<float>(1, 0) / e2.at<float>(2, 0));
        EpiPoles[1] = cv::Point2d(EpiPole_ImgM.at<float>(0, 0) / EpiPole_ImgM.at<float>(2, 0), EpiPole_ImgM.at<float>(1, 0) / EpiPole_ImgM.at<float>(2, 0));
        if ((EpiPoles[0].x * EpiPoles[1].x < 0) && (EpiPoles[0].y * EpiPoles[1].y < 0))
        {
            EpiPoles[1] *= -1;
        }
        
        determ_CoRegion(EpiPoles, Img.size(), F_mtx);

        return Img_Rect;
    }

    void Fu_FotoGmtCV::determ_CoRegion(const std::vector<cv::Point2f>& EpiPoles, const cv::Size ImgSize, const cv::Mat& F)
    {
        std::vector<cv::Point2f> ExternPt_ImgB, ExternPt_ImgM;
        get_ExternPt(EpiPoles[0], ImgSize, ExternPt_ImgB);
        get_ExternPt(EpiPoles[1], ImgSize, ExternPt_ImgM);

        float Rho_B_min, Rho_B_max, Rho_M_min, Rho_M_max;
        determ_RhoRange(EpiPoles[0], ImgSize, ExternPt_ImgB, Rho_B_min, Rho_B_max);
        determ_RhoRange(EpiPoles[1], ImgSize, ExternPt_ImgM, Rho_M_min, Rho_M_max);

        if (!is_InsideImg(EpiPoles[0], ImgSize) && !is_InsideImg(EpiPoles[1], ImgSize))
        {
            cv::Vec3f ImgLn11;
            get_ImgLn_from_ImgPt(EpiPoles[0], ExternPt_ImgB[0], ImgLn11);
            cv::Vec3f ImgLn12;
            get_ImgLn_from_ImgPt(EpiPoles[0], ExternPt_ImgB[1], ImgLn12);

            cv::Vec3f ImgLn23;
            get_ImgLn_from_ImgPt(EpiPoles[1], ExternPt_ImgM[0], ImgLn23);
            cv::Vec3f ImgLn24;
            get_ImgLn_from_ImgPt(EpiPoles[1], ExternPt_ImgM[1], ImgLn24);

            std::vector <cv::Vec3f> ImgLn_Input(2), ImgLn_Output(2);
            ImgLn_Input[0] = ImgLn23;
            ImgLn_Input[1] = ImgLn24;
            cal_EpiLn(ExternPt_ImgM, 2, F, ImgLn_Input, ImgLn_Output);
            const cv::Vec3f ImgLn13 = ImgLn_Output[0];
            const cv::Vec3f ImgLn14 = ImgLn_Output[1];

            ImgLn_Input[0] = ImgLn11;
            ImgLn_Input[1] = ImgLn12;
            cal_EpiLn(ExternPt_ImgB, 1, F, ImgLn_Input, ImgLn_Output);
            const cv::Vec3f ImgLn21 = ImgLn_Output[0];
            const cv::Vec3f ImgLn22 = ImgLn_Output[1];

            // Beginning & Ending Image Line
            cv::Vec3f ImgLn_B_Bgn = LnIntersectRect(ImgLn13, ImgSize) ? ImgLn13 : ImgLn11;
            cv::Vec3f ImgLn_B_End = LnIntersectRect(ImgLn14, ImgSize) ? ImgLn14 : ImgLn12;
            cv::Vec3f ImgLn_M_Bgn = LnIntersectRect(ImgLn13, ImgSize) ? ImgLn21 : ImgLn23;
            cv::Vec3f ImgLn_M_End = LnIntersectRect(ImgLn13, ImgSize) ? ImgLn22 : ImgLn24;

            cv::Point2d m_b1, m_b2;

            // Beginning & Ending Image Line Intersection with the Border
            {
                std::vector<cv::Point2f> intersections;
                getBorderIntersections(epipoles[0], m_line1B, ImgSize, intersections);
                double maxDist = std::numeric_limits<double>::min();
                for (uint32_t i = 0; i < intersections.size(); i++) 
                {
                    const cv::Point2f intersect = intersections[i];
                    const double dist = cv::norm(EpiPoles[0] - intersect);
                    if (dist > maxDist) {
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
                        m_e1 = intersections[i];
                    }
                }
            }
        } 
        else if (is_InsideImg(EpiPoles[0], ImgSize) && is_InsideImg(EpiPoles[1], ImgSize))
        {
            get_ImgLn_from_ImgPt(EpiPoles[0], ExternPt_ImgB[0], m_line1B);
            m_line1E = m_line1B;

            std::vector <cv::Vec3f> ImgLn_Input(1), ImgLn_Output(1);
            ImgLn_Input[0] = m_line1B;
            cal_EpiLn(ExternPt_ImgB, 1, F, ImgLn_Input, ImgLn_Output);

            m_line2B = ImgLn_Output[0];
            m_line2E = ImgLn_Output[0];

            m_b1 = getBorderIntersection(EpiPoles[0], m_line1B, ImgSize);
            m_e1 = getBorderIntersection(EpiPoles[0], m_line1E, ImgSize);

            m_b2 = m_e2 = getNearestIntersection(epipoles[0], epipoles[1], m_line2B, m_b1, imgDimensions);
        } 
        else
        {
            if (is_InsideImg(EpiPoles[0], ImgSize)) 
            {
                // CASE 3.1: Only the first epipole is inside

                cv::Vec3f ImgLn23;
                get_ImgLn_from_ImgPt(EpiPoles[1], ExternPt_ImgM[0]);
                cv::Vec3f ImgLn24;
                get_ImgLn_from_ImgPt(EpiPoles[1], ExternPt_ImgM[1]);

                std::vector <cv::Vec3f> ImgLn_Input(2), ImgLn_Output(2);
                ImgLn_Input[0] = ImgLn23;
                ImgLn_Input[1] = ImgLn24;
                cal_EpiLn(ExternPt_ImgM, 2, F, ImgLn_Input, ImgLn_Output);
                const cv::Vec3f & line13 = ImgLn_Output[0];
                const cv::Vec3f & line14 = ImgLn_Output[1];

                m_line1B = line13;
                m_line1E = line14;
                m_line2B = ImgLn23;
                m_line2E = ImgLn24;

                m_b2 = getBorderIntersection(Epipoles[1], m_line2B, ImgSize);
                m_e2 = getBorderIntersection(EpiPoles[1], m_line2E, ImgSize);

                m_b1 = getNearestIntersection(EpiPoles[1], EpiPoles[0], m_line1B, m_b2, ImgSize);
                m_e1 = getNearestIntersection(EpiPoles[1], EpiPoles[0], m_line1E, m_e2, ImgSize);
            }
            else 
            {
                cv::Vec3f ImgLn11;
                get_ImgLn_from_ImgPt(EpiPoles[0], ExternPt_ImgB[0], ImgLn11);
                cv::Vec3f ImgLn12;
                get_ImgLn_from_ImgPt(EpiPoles[0], ExternPt_ImgB[1], ImgLn12);

                std::vector <cv::Vec3f> ImgLn_Input(2), ImgLn_Output(2);
                ImgLn_Input[0] = ImgLn11;
                ImgLn_Input[1] = ImgLn12;
                cal_EpiLn(ExternPt_ImgB, 1, F, ImgLn_Input, ImgLn_Output);
                cv::Vec3f & ImgLn21 = ImgLn_Output[0];
                cv::Vec3f & ImgLn22 = ImgLn_Output[1];

                m_line1B = ImgLn11;
                m_line1E = ImgLn12;
                m_line2B = ImgLn21;
                m_line2E = ImgLn22;

                m_b1 = getBorderIntersection(EpiPoles[0], m_line1B, ImgSize);
                m_e1 = getBorderIntersection(EpiPoles[0], m_line1E, ImgSize);

                m_b2 = getNearestIntersection(EpiPoles[0], EpiPoles[1], m_line2B, m_b1, ImgSize);
                m_e2 = getNearestIntersection(EpiPoles[0], EpiPoles[1], m_line2E, m_e1, ImgSize);
            }
        }
    }

    void Fu_FotoGmtCV::get_ExternPt(const cv::Point2f& EpiPole, const cv::Size ImgSize, std::vector<cv::Point2f>& ExternPt)
    {
        if (EpiPole.y < 0)
        {
            if (EpiPole.x < 0)
            {
                ExternPt.resize(2);
                ExternPt[0] = cv::Point2f(ImgSize.width - 1, 0);
                ExternPt[1] = cv::Point2f(0, ImgSize.height - 1);
            }
            else if (EpiPole.x < ImgSize.width - 1)
            {
                ExternPt.resize(2);
                ExternPt[0] = cv::Point2f(ImgSize.width - 1, 0);
                ExternPt[1] = cv::Point2f(0, 0);
            }
            else
            {
                ExternPt.resize(2);
                ExternPt[0] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
                ExternPt[1] = cv::Point2f(0, 0);
            }
        }
        else if (EpiPole.y <= ImgSize.height - 1)
        {
            if (EpiPole.x < 0)
            {
                ExternPt.resize(2);
                ExternPt[0] = cv::Point2f(0, 0);
                ExternPt[1] = cv::Point2f(0, ImgSize.height - 1);
            }
            else if (EpiPole.x < ImgSize.width - 1)
            {
                ExternPt.resize(4);
                ExternPt[0] = cv::Point2f(0, 0);
                ExternPt[1] = cv::Point2f(ImgSize.width - 1, 0);
                ExternPt[2] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
                ExternPt[3] = cv::Point2f(0, ImgSize.height - 1);
            }
            else
            {
                ExternPt.resize(2);
                ExternPt[0] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
                ExternPt[1] = cv::Point2f(ImgSize.width - 1, 0);
            }
        }
        else
        {
            if (EpiPole.x < 0)
            {
                ExternPt.resize(2);
                ExternPt[0] = cv::Point2f(0, 0);
                ExternPt[1] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
            }
            else if (EpiPole.x < ImgSize.width - 1)
            {
                ExternPt.resize(2);
                ExternPt[0] = cv::Point2f(0, ImgSize.height - 1);
                ExternPt[1] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
            }
            else
            {
                ExternPt.resize(2);
                ExternPt[0] = cv::Point2f(0, ImgSize.height - 1);
                ExternPt[1] = cv::Point2f(ImgSize.width - 1, 0);
            }
        }
    }

    void Fu_FotoGmtCV::determ_RhoRange(const cv::Point2f & EpiPole, const cv::Size ImgSize, std::vector<cv::Point2f> & ImgCorner, float & minRho, float & maxRho)
    {
        if (EpiPole.y < 0)
        { 
            if (EpiPole.x < 0) 
            {
                minRho = sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y); // Point A
                maxRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)); // Point D
            }
            else if (EpiPole.x <= ImgSize.width - 1) 
            {
                minRho = -EpiPole.y;
                maxRho = std::max(
                                  sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)), // Point C
                                  sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)) // Point D
                                 );
            }
            else { // Case 3
                minRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y); // Point B
                maxRho = sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)); // Point C
            }
        }
        else if (EpiPole.y <= ImgSize.height - 1) 
        { 
            if (EpiPole.x < 0) 
            { 
                minRho = -EpiPole.x;
                maxRho = std::max(
                                  sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)), // Point D
                                  sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y) // Point B
                                 );
            }
            else if (EpiPole.x <= ImgSize.width - 1) 
            { 
                minRho = 0;
                maxRho = std::max(
                                  std::max(
                                           sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y), // Point A
                                           sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y) // Point B
                                          ),
                                  std::max(
                                           sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)), // Point C
                                           sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)) // Point D
                                          )
                                 );
            }
            else 
            { 
                minRho = EpiPole.x - (ImgSize.width - 1);
                maxRho = std::max(
                                  sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y), // Point A
                                  sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)) // Point C
                                 );
            }
        }
        else 
        {
            if (EpiPole.x < 0) 
            {
                minRho = sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)); // Point C
                maxRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y); // Point B
            }
            else if (EpiPole.x <= ImgSize.width - 1) 
            { 
                minRho = EpiPole.y - (ImgSize.height - 1);
                maxRho = std::max(
                                  sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y), // Point A
                                  sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y) // Point B
                                 );
            }
            else 
            {
                minRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)); // Point D
                maxRho = sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y); // Point A
            }
        }
    }

    void Fu_FotoGmtCV::get_ImgLn_from_ImgPt(const cv::Point2f & ImgPt1, const cv::Point2f & ImgPt2, cv::Vec3f ImgLn)
    {
        ImgLn = cv::Vec3f(ImgPt1.y - ImgPt2.y, ImgPt2.x - ImgPt1.x, ImgPt1.x * ImgPt2.y - ImgPt2.x * ImgPt1.y);
    }

    void Fu_FotoGmtCV::cal_EpiLn(const std::vector<cv::Point2f>& ImgPts, const uint whichImg, const cv::Mat F_mtx, const std::vector<cv::Vec3f>& ImgLn_old, std::vector<cv::Vec3f>& ImgLn_new)
    {
        cv::computeCorrespondEpilines(ImgPts, whichImg, F_mtx, ImgLn_new);

        for (int i = 0; i < ImgLn_old.size(); i++)
        {
            if ((ImgLn_old[i][0] * ImgLn_new[i][0] < 0) && (ImgLn_old[i][1] * ImgLn_new[i][1] < 0) )
            {
                ImgLn_new[i] * 0 - 1;
            }
        }
    }

    bool Fu_FotoGmtCV::LnIntersectRect(const cv::Vec3d & ImgLn, const cv::Size & ImgSize, cv::Point2d * intersection)
    {
        return LnIntersectsSegment(ImgLn, cv::Point2d(0, 0), cv::Point2d(ImgSize.width - 1, 0), intersection) ||
            LnIntersectsSegment(ImgLn, cv::Point2d(ImgSize.width - 1, 0), cv::Point2d(ImgSize.width - 1, ImgSize.height - 1), intersection) ||
            LnIntersectsSegment(ImgLn, cv::Point2d(ImgSize.width - 1, ImgSize.height - 1), cv::Point2d(0, ImgSize.height - 1), intersection) ||
            LnIntersectsSegment(ImgLn, cv::Point2d(0, ImgSize.height - 1), cv::Point2d(0, 0), intersection);
    }

    bool Fu_FotoGmtCV::LnIntersectsSegment(const cv::Vec3d & ImgLn, const cv::Point2d & ImgPtB, const cv::Point2d & ImgPtM, cv::Point2d * intersection)
    {
        cv::Vec3d segment;
        get_ImgLn_from_ImgPt(ImgPtB, ImgPtM, segment);

        if (intersection != NULL)
            *intersection = cv::Point2d(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

        // Lines are represented as ax + by + c = 0, so
        // y = -(ax+c)/b. If y1=y2, then we have to obtain x, which is
        // x = (b1 * c2 - b2 * c1) / (b2 * a1 - b1 * a2)
        if ((segment[1] * ImgLn[0] - ImgLn[1] * segment[0]) == 0)
            return false;
        double x = (ImgLn[1] * segment[2] - segment[1] * ImgLn[2]) / (segment[1] * ImgLn[0] - ImgLn[1] * segment[0]);
        double y = -(ImgLn[0] * x + ImgLn[2]) / ImgLn[1];

        if (((int32_t)round(x) >= (int32_t)min(ImgPtB.x, ImgPtM.x)) && ((int32_t)round(x) <= (int32_t)max(ImgPtB.x, ImgPtM.x))) 
        {
            if (((int32_t)round(y) >= (int32_t)min(ImgPtB.y, ImgPtM.y)) && ((int32_t)round(y) <= (int32_t)max(ImgPtB.y, ImgPtM.y))) 
            {
                if (intersection != NULL)
                    *intersection = cv::Point2d(x, y);

                return true;
            }
        }

        return false;
    }

    cv::Point2d Fu_FotoGmtCV::get_BorderIntersect(const cv::Point2f& Epipole, const cv::Vec3f& ImgLn, const cv::Size& ImgSize, const cv::Point2f* ImgPt_Last)
    {
        cv::Point2d intersection(-1, -1);

        if (is_InsideImg(Epipole, ImgSize)) 
        {
            if (LnIntersectsSegment(ImgLn, cv::Point2d(0, 0), cv::Point2d(ImgSize.width - 1, 0), &intersection)) 
            {
                if (isTheRightPoint(epipole, intersection, line, lastPoint)) 
                {

                    return intersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, 0), cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), &intersection)) {
                if (isTheRightPoint(epipole, intersection, line, lastPoint)) {

                    return intersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), cv::Point2d(0, imgDimensions.height - 1), &intersection)) {
                if (isTheRightPoint(epipole, intersection, line, lastPoint)) {

                    return intersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(0, imgDimensions.height - 1), cv::Point2d(0, 0), &intersection)) {
                if (isTheRightPoint(epipole, intersection, line, lastPoint)) {

                    return intersection;
                }
            }
        }
        else {
            double maxDist = std::numeric_limits<double>::min();
            cv::Point2d tmpIntersection(-1, -1);
            if (lineIntersectsSegment(line, cv::Point2d(0, 0), cv::Point2d(imgDimensions.width - 1, 0), &tmpIntersection)) {
                const double dist2 = (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x) +
                    (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x);

                if (dist2 > maxDist) {
                    maxDist = dist2;
                    intersection = tmpIntersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, 0), cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), &tmpIntersection)) {
                const double dist2 = (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x) +
                    (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x);

                if (dist2 > maxDist) {
                    maxDist = dist2;
                    intersection = tmpIntersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(imgDimensions.width - 1, imgDimensions.height - 1), cv::Point2d(0, imgDimensions.height - 1), &tmpIntersection)) {
                const double dist2 = (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x) +
                    (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x);

                if (dist2 > maxDist) {
                    maxDist = dist2;
                    intersection = tmpIntersection;
                }
            }
            if (lineIntersectsSegment(line, cv::Point2d(0, imgDimensions.height - 1), cv::Point2d(0, 0), &tmpIntersection)) {
                const double dist2 = (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x) +
                    (tmpIntersection.x - epipole.x) * (tmpIntersection.x - epipole.x);

                if (dist2 > maxDist) {
                    maxDist = dist2;
                    intersection = tmpIntersection;
                }
            }
            return intersection;
        }
    }

    //---Orientation & Transformation---

    //---Is Function---
    bool Fu_FotoGmtCV::is_InsideImg(cv::Point2f ImgPt, cv::Size ImgSize)
    {
        return ( (ImgPt.x >= 0) && (ImgPt.y >= 0) && (ImgPt.x < ImgSize.width) && (ImgPt.y < ImgSize.height) );
    }

    //---Set Functions---
    void Fu_FotoGmtCV::set_Cam(glm::mat3& glmK)
    {
        cv::Mat cvK = cv::Mat(3, 3, CV_32F);
        glm2cv(&cvK, glm::transpose(glmK));
        
        K_mtx = cvK;
    }

    void Fu_FotoGmtCV::set_Rot(glm::mat3 glmR)
    {
        cv::Mat cvR = cv::Mat(3, 3, CV_32F);
        glm2cv(&cvR, glm::transpose(glmR));
        
        Rot_mtx = cvR;
    }

    void Fu_FotoGmtCV::set_Trans(glm::vec3 glmT)
    {
        cv::Mat cvT = cv::Mat(3, 1, CV_32F);
        glm2cv(&cvT, glmT);
        
        Trans_vec = cvT;
    }

    void Fu_FotoGmtCV::set_P(glm::mat3x4 glmP)
    {
        cv::Mat cvP = cv::Mat(3, 4, CV_32F);
        glm2cv(&cvP, glmP);

        P_mtx = cvP;
    }

    //---Show Function---
    void Fu_FotoGmtCV::show_Img()
    {
        cv::imshow("Image", Img);
    }

    //---Type Transform---
    void Fu_FotoGmtCV::glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 9 * sizeof(float));
    }

    void Fu_FotoGmtCV::glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 3 * sizeof(float));
    }

    void Fu_FotoGmtCV::glm2cv(cv::Mat* cvmat, const glm::mat3x4& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 12 * sizeof(float));
    }

} // Stereo
