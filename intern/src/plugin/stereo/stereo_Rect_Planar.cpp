#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_Rect_Planar.h"

namespace Stereo
{
    //---Constructors & Destructor---
    Rect_Planar::Rect_Planar()
    {
    }


    Rect_Planar::~Rect_Planar()
    {
    }

    //---Generation of Rectified Img---
    void Rect_Planar::genrt_RectImg(const cv::Mat& Img_Orig_B, const cv::Mat& Img_Orig_M)
    {
        //---Create Rectified Images---
        Img_Rect_B = cv::Mat(ImgSize_Rect, CV_8UC4);
        Img_Rect_M = cv::Mat(ImgSize_Rect, CV_8UC4);
        
        //---Build Look-Up Table: from Rectified to Originals---
        mapB_x_Rect2Orig = cv::Mat(ImgSize_Rect, CV_32FC1);
        mapB_y_Rect2Orig = cv::Mat(ImgSize_Rect, CV_32FC1);
        mapM_x_Rect2Orig = cv::Mat(ImgSize_Rect, CV_32FC1);
        mapM_y_Rect2Orig = cv::Mat(ImgSize_Rect, CV_32FC1);

        for (int idx_y = 0; idx_y < ImgSize_Rect.height; idx_y++)
        {
            for (int idx_x = 0; idx_x < ImgSize_Rect.width; idx_x++)
            {
                cv::Mat pix_Rect = cv::Mat::ones(3, 1, CV_32F);
                pix_Rect.ptr<float>(0)[0] = idx_x + ImgSize_Rect_x_min;
                pix_Rect.ptr<float>(1)[0] = idx_y + ImgSize_Rect_y_min;

                cv::Mat pixB_Rect2Orig = H_B.inv() * pix_Rect;
                pixB_Rect2Orig /= pixB_Rect2Orig.ptr<float>(2)[0];

                cv::Mat pixM_Rect2Orig = H_M.inv() * pix_Rect;
                pixM_Rect2Orig /= pixM_Rect2Orig.ptr<float>(2)[0];

                mapB_x_Rect2Orig.ptr<float>(idx_y)[idx_x] = pixB_Rect2Orig.ptr<float>(0)[0];
                mapB_y_Rect2Orig.ptr<float>(idx_y)[idx_x] = pixB_Rect2Orig.ptr<float>(1)[0];

                mapM_x_Rect2Orig.ptr<float>(idx_y)[idx_x] = pixM_Rect2Orig.ptr<float>(0)[0];
                mapM_y_Rect2Orig.ptr<float>(idx_y)[idx_x] = pixM_Rect2Orig.ptr<float>(1)[0];
            }
        }

        //---Derive Pixel Value of Rectified Images: Transform Pixels from Rectfied back to Origianls & Interpolation---
        cv::remap(Img_Orig_B, Img_Rect_B, mapB_x_Rect2Orig, mapB_y_Rect2Orig, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
        cv::remap(Img_Orig_M, Img_Rect_M, mapM_x_Rect2Orig, mapM_y_Rect2Orig, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);


        //---Build Look-Up Table: from Originals to Rectified---
        mapB_x_Orig2Rect = cv::Mat(Img_Orig_B.size(), CV_32FC1);
        mapB_y_Orig2Rect = cv::Mat(Img_Orig_B.size(), CV_32FC1);
        mapM_x_Orig2Rect = cv::Mat(Img_Orig_M.size(), CV_32FC1);
        mapM_y_Orig2Rect = cv::Mat(Img_Orig_M.size(), CV_32FC1);

        for (int idx_y = 0; idx_y < Img_Orig_B.size().height; idx_y++)
        {
            for (int idx_x = 0; idx_x < Img_Orig_B.size().width; idx_x++)
            {
                cv::Mat pixB_Orig = cv::Mat::ones(3, 1, CV_32F);
                pixB_Orig.ptr<float>(0)[0] = idx_x;
                pixB_Orig.ptr<float>(1)[0] = idx_y;

                cv::Mat pixM_Orig = cv::Mat::ones(3, 1, CV_32F);
                pixM_Orig.ptr<float>(0)[0] = idx_x;
                pixM_Orig.ptr<float>(1)[0] = idx_y;

                cv::Mat pixB_Orig2Rect = H_B * pixB_Orig;
                pixB_Orig2Rect /= pixB_Orig2Rect.ptr<float>(2)[0];
                pixB_Orig2Rect.ptr<float>(0)[0] = pixB_Orig2Rect.ptr<float>(0)[0] - ImgSize_Rect_x_min;
                pixB_Orig2Rect.ptr<float>(1)[0] = pixB_Orig2Rect.ptr<float>(1)[0] - ImgSize_Rect_y_min;                

                cv::Mat pixM_Orig2Rect = H_M * pixM_Orig;
                pixM_Orig2Rect /= pixM_Orig2Rect.ptr<float>(2)[0];
                pixM_Orig2Rect.ptr<float>(0)[0] = pixM_Orig2Rect.ptr<float>(0)[0] - ImgSize_Rect_x_min;
                pixM_Orig2Rect.ptr<float>(1)[0] = pixM_Orig2Rect.ptr<float>(1)[0] - ImgSize_Rect_y_min;

                mapB_x_Orig2Rect.ptr<float>(idx_y)[idx_x] = pixB_Orig2Rect.ptr<float>(0)[0];
                mapB_y_Orig2Rect.ptr<float>(idx_y)[idx_x] = pixB_Orig2Rect.ptr<float>(1)[0];

                mapM_x_Orig2Rect.ptr<float>(idx_y)[idx_x] = pixM_Orig2Rect.ptr<float>(0)[0];
                mapM_y_Orig2Rect.ptr<float>(idx_y)[idx_x] = pixM_Orig2Rect.ptr<float>(1)[0];
            }
        }
    }

    void Rect_Planar::determ_RectImgSize(const cv::Size& ImgSize_OrigB, const cv::Size& ImgSize_OrigM)
    {
        //---Select Img Corner in Originals---
        cv::Mat ImgCnrUL_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrUL_OrigB.ptr<float>(0)[0] = 0;
        ImgCnrUL_OrigB.ptr<float>(1)[0] = 0;
        cv::Mat ImgCnrUR_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrUR_OrigB.ptr<float>(0)[0] = ImgSize_OrigB.width;
        ImgCnrUR_OrigB.ptr<float>(1)[0] = 0;
        cv::Mat ImgCnrDL_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrDL_OrigB.ptr<float>(0)[0] = 0;
        ImgCnrDL_OrigB.ptr<float>(1)[0] = ImgSize_OrigB.height;
        cv::Mat ImgCnrDR_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrDR_OrigB.ptr<float>(0)[0] = ImgSize_OrigB.width;
        ImgCnrDR_OrigB.ptr<float>(1)[0] = ImgSize_OrigB.height;

        cv::Mat ImgCnrUL_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrUL_OrigM.ptr<float>(0)[0] = 0;
        ImgCnrUL_OrigM.ptr<float>(1)[0] = 0;
        cv::Mat ImgCnrUR_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrUR_OrigM.ptr<float>(0)[0] = ImgSize_OrigM.width;
        ImgCnrUR_OrigM.ptr<float>(1)[0] = 0;
        cv::Mat ImgCnrDL_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrDL_OrigM.ptr<float>(0)[0] = 0;
        ImgCnrDL_OrigM.ptr<float>(1)[0] = ImgSize_OrigM.height;
        cv::Mat ImgCnrDR_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrDR_OrigM.ptr<float>(0)[0] = ImgSize_OrigM.width;
        ImgCnrDR_OrigM.ptr<float>(1)[0] = ImgSize_OrigM.height;

        
        //---Transform Img Corner from Originals to Rectified---
        cv::Mat ImgCnrUL_Orig2Rect_B = H_B * ImgCnrUL_OrigB;
        ImgCnrUL_Orig2Rect_B = ImgCnrUL_Orig2Rect_B / ImgCnrUL_Orig2Rect_B.ptr<float>(2)[0];
        cv::Mat ImgCnrUR_Orig2Rect_B = H_B * ImgCnrUR_OrigB;
        ImgCnrUR_Orig2Rect_B = ImgCnrUR_Orig2Rect_B / ImgCnrUR_Orig2Rect_B.ptr<float>(2)[0];
        cv::Mat ImgCnrDL_Orig2Rect_B = H_B * ImgCnrDL_OrigB;
        ImgCnrDL_Orig2Rect_B = ImgCnrDL_Orig2Rect_B / ImgCnrDL_Orig2Rect_B.ptr<float>(2)[0];
        cv::Mat ImgCnrDR_Orig2Rect_B = H_B * ImgCnrDR_OrigB;
        ImgCnrDR_Orig2Rect_B = ImgCnrDR_Orig2Rect_B / ImgCnrDR_Orig2Rect_B.ptr<float>(2)[0];

        cv::Mat ImgCnrUL_Orig2Rect_M = H_M * ImgCnrUL_OrigM;
        ImgCnrUL_Orig2Rect_M = ImgCnrUL_Orig2Rect_M / ImgCnrUL_Orig2Rect_M.ptr<float>(2)[0];
        cv::Mat ImgCnrUR_Orig2Rect_M = H_M * ImgCnrUR_OrigM;
        ImgCnrUR_Orig2Rect_M = ImgCnrUR_Orig2Rect_M / ImgCnrUR_Orig2Rect_M.ptr<float>(2)[0];
        cv::Mat ImgCnrDL_Orig2Rect_M = H_M * ImgCnrDL_OrigM;
        ImgCnrDL_Orig2Rect_M = ImgCnrDL_Orig2Rect_M / ImgCnrDL_Orig2Rect_M.ptr<float>(2)[0];
        cv::Mat ImgCnrDR_Orig2Rect_M = H_M * ImgCnrDR_OrigM;
        ImgCnrDR_Orig2Rect_M = ImgCnrDR_Orig2Rect_M / ImgCnrDR_Orig2Rect_M.ptr<float>(2)[0];

        //---Determine the Boundary of Epipolar Image---
        std::vector<float> ImgCnr_Orig2Rect_B_x = { ImgCnrUL_Orig2Rect_B.ptr<float>(0)[0], ImgCnrUR_Orig2Rect_B.ptr<float>(0)[0], ImgCnrDL_Orig2Rect_B.ptr<float>(0)[0], ImgCnrDR_Orig2Rect_B.ptr<float>(0)[0] };

        std::vector<float> ImgCnr_Orig2Rect_B_y = { ImgCnrUL_Orig2Rect_B.ptr<float>(1)[0], ImgCnrUR_Orig2Rect_B.ptr<float>(1)[0], ImgCnrDL_Orig2Rect_B.ptr<float>(1)[0], ImgCnrDR_Orig2Rect_B.ptr<float>(1)[0] };

        std::vector<float> ImgCnr_Orig2Rect_M_x = { ImgCnrUL_Orig2Rect_M.ptr<float>(0)[0], ImgCnrUR_Orig2Rect_M.ptr<float>(0)[0], ImgCnrDL_Orig2Rect_M.ptr<float>(0)[0], ImgCnrDR_Orig2Rect_M.ptr<float>(0)[0] };

        std::vector<float> ImgCnr_Orig2Rect_M_y = { ImgCnrUL_Orig2Rect_M.ptr<float>(1)[0], ImgCnrUR_Orig2Rect_M.ptr<float>(1)[0], ImgCnrDL_Orig2Rect_M.ptr<float>(1)[0], ImgCnrDR_Orig2Rect_M.ptr<float>(1)[0] };

        float ImgBound_RectB_x_min = *std::min_element(ImgCnr_Orig2Rect_B_x.begin(), ImgCnr_Orig2Rect_B_x.end());
        float ImgBound_RectB_y_min = *std::min_element(ImgCnr_Orig2Rect_B_y.begin(), ImgCnr_Orig2Rect_B_y.end());
        float ImgBound_RectB_x_max = *std::max_element(ImgCnr_Orig2Rect_B_x.begin(), ImgCnr_Orig2Rect_B_x.end());
        float ImgBound_RectB_y_max = *std::max_element(ImgCnr_Orig2Rect_B_y.begin(), ImgCnr_Orig2Rect_B_y.end());

        float ImgBound_RectM_x_min = *std::min_element(ImgCnr_Orig2Rect_M_x.begin(), ImgCnr_Orig2Rect_M_x.end());
        float ImgBound_RectM_y_min = *std::min_element(ImgCnr_Orig2Rect_M_y.begin(), ImgCnr_Orig2Rect_M_y.end());
        float ImgBound_RectM_x_max = *std::max_element(ImgCnr_Orig2Rect_M_x.begin(), ImgCnr_Orig2Rect_M_x.end());
        float ImgBound_RectM_y_max = *std::max_element(ImgCnr_Orig2Rect_M_y.begin(), ImgCnr_Orig2Rect_M_y.end());

        //---Determine the Size of Epipolar Imgs---
        ImgSize_Rect_x_min = std::floor(std::min(ImgBound_RectB_x_min, ImgBound_RectM_x_min));
        ImgSize_Rect_x_max = std::ceil(std::max(ImgBound_RectB_x_max, ImgBound_RectM_x_max));
        ImgSize_Rect_y_min = std::floor(std::min(ImgBound_RectB_y_min, ImgBound_RectM_y_min));
        ImgSize_Rect_y_max = std::ceil(std::max(ImgBound_RectB_y_max, ImgBound_RectM_y_max));

        ImgSize_Rect = cv::Size(ImgSize_Rect_x_max - ImgSize_Rect_x_min, ImgSize_Rect_y_max - ImgSize_Rect_y_min);
    }

    void Rect_Planar::imp_CenterRectImg(const cv::Size& ImgSize_OrigB, const cv::Size& ImgSize_OrigM)
    {
        cv::Mat CenterB_Orig = cv::Mat::ones(3, 1, CV_32F);
        CenterB_Orig.ptr<float>(0)[0] = ImgSize_OrigB.width / 2;
        CenterB_Orig.ptr<float>(1)[0] = ImgSize_OrigB.height / 2;
        cv::Mat CenterM_Orig = cv::Mat::ones(3, 1, CV_32F);
        CenterM_Orig.ptr<float>(0)[0] = ImgSize_OrigM.width / 2;
        CenterM_Orig.ptr<float>(1)[0] = ImgSize_OrigM.height / 2;

        cv::Mat CenterB_Orig2Rect = H_B * CenterB_Orig;
        CenterB_Orig2Rect /= CenterB_Orig2Rect.ptr<float>(2)[0];
        cv::Mat CenterM_Orig2Rect = H_M * CenterM_Orig;
        CenterM_Orig2Rect /= CenterM_Orig2Rect.ptr<float>(2)[0];

        cv::Mat Drift_B = CenterB_Orig - CenterB_Orig2Rect;
        cv::Mat Drift_M = CenterM_Orig - CenterM_Orig2Rect;
        float Drift_y = (Drift_B.ptr<float>(1)[0] + Drift_M.ptr<float>(1)[0]) / 2;
        Drift_B.ptr<float>(1)[0] = Drift_y;
        Drift_M.ptr<float>(1)[0] = Drift_y;

        K_Rect_B.ptr<float>(0)[2] += Drift_B.ptr<float>(0)[0];
        K_Rect_B.ptr<float>(1)[2] += Drift_B.ptr<float>(1)[0];
        K_Rect_M.ptr<float>(0)[2] += Drift_M.ptr<float>(0)[0];
        K_Rect_M.ptr<float>(1)[2] += Drift_M.ptr<float>(1)[0];
    }

    //---Compute Orientations---
    void Rect_Planar::cal_K_Rect(const cv::Mat& K_Orig_B, const cv::Mat& K_Orig_M)
    {
        K_Rect_B = 0.5 * (K_Orig_B + K_Orig_M);
        K_Rect_M = 0.5 * (K_Orig_B + K_Orig_M);
    }

    void Rect_Planar::cal_R_Rect(const cv::Mat& R_Orig_B, const cv::Mat& PC_Orig_B, const cv::Mat& PC_Orig_M)
    {
        R_Rect = cv::Mat::eye(3, 3, CV_32F);

        cv::Mat R_Rect_row0 = PC_Orig_M - PC_Orig_B;
        if (cv::sum(R_Rect_row0)[0] < 0)
        {
            R_Rect_row0 *= -1; // Make the rotation always left2right and up2down.
        }

        R_Rect_row0 /= cv::norm(R_Rect_row0, cv::NORM_L2);
        cv::transpose(R_Rect_row0, R_Rect_row0);

        cv::Mat R_Rect_row1 = R_Orig_B.row(2).cross(R_Rect_row0);
        R_Rect_row1 /= cv::norm(R_Rect_row1, cv::NORM_L2);

        cv::Mat R_Rect_row2 = R_Rect_row0.cross(R_Rect_row1);
        R_Rect_row2 /= cv::norm(R_Rect_row2, cv::NORM_L2);

        R_Rect_row0.copyTo(R_Rect.row(0));
        R_Rect_row1.copyTo(R_Rect.row(1));
        R_Rect_row2.copyTo(R_Rect.row(2));
    }

    void Rect_Planar::cal_P_Rect(const cv::Mat& PC_Orig_B, const cv::Mat& PC_Orig_M)
    {
        cv::Mat Trans_Rect_B(3, 4, CV_32F);
        R_Rect.colRange(0, 3).copyTo(Trans_Rect_B.colRange(0, 3)); // StartCol is inclusive while EndCol is exclusive
        cv::Mat t_Orig_B = -R_Rect * PC_Orig_B;
        t_Orig_B.col(0).copyTo(Trans_Rect_B.col(3));

        cv::Mat Trans_Rect_M(3, 4, CV_32F);
        R_Rect.colRange(0, 3).copyTo(Trans_Rect_M.colRange(0, 3)); // StartCol is inclusive while EndCol is exclusive
        cv::Mat t_Orig_M = -R_Rect * PC_Orig_M;
        t_Orig_M.col(0).copyTo(Trans_Rect_M.col(3));

        P_Rect_B = K_Rect_B * Trans_Rect_B;
        P_Rect_M = K_Rect_M * Trans_Rect_M;
    }

    void Rect_Planar::cal_H(const cv::Mat& P_Orig_B, cv::Mat& P_Orig_M)
    {
        //---Calculate the Homography---
        H_B = P_Rect_B * P_Orig_B.inv(cv::DECOMP_SVD); 
        H_M = P_Rect_M * P_Orig_M.inv(cv::DECOMP_SVD);
        //---Another Homography Transformation used in Fusiello's Code. -> I do not like it because it is not reasonable
        /*
        //H_B = P_Rect_B.colRange(0, 3) * P_Orig_B.colRange(0, 3).inv();
        //H_M = P_Rect_M.colRange(0, 3) * P_Orig_M.colRange(0, 3).inv();
        */
        //---

    }

    //---Get Function---
	void Rect_Planar::get_K_Rect(cv::Mat &CamB_Rect, cv::Mat &CamM_Rect)
	{
		CamB_Rect = K_Rect_B;
		CamM_Rect = K_Rect_M;
	}
	void Rect_Planar::get_R_Rect(cv::Mat &Rot_Rect)
	{
		Rot_Rect = R_Rect;
	}

    void Rect_Planar::get_RectImg(cv::Mat& Output_RectImgB, cv::Mat& Output_RectImgM)
    {
        Output_RectImgB = Img_Rect_B;
        Output_RectImgM = Img_Rect_M;
    }

    void Rect_Planar::get_Transform_Orig2Rect(cv::Mat& LookUpTx_B_Orig2Rect, cv::Mat& LookUpTy_B_Orig2Rect, cv::Mat& LookUpTx_M_Orig2Rect, cv::Mat& LookUpTy_M_Orig2Rect)
    {
        LookUpTx_B_Orig2Rect = mapB_x_Orig2Rect;
        LookUpTy_B_Orig2Rect = mapB_y_Orig2Rect;
        LookUpTx_M_Orig2Rect = mapM_x_Orig2Rect;
        LookUpTy_M_Orig2Rect = mapM_y_Orig2Rect;
    }

} // Stereo
