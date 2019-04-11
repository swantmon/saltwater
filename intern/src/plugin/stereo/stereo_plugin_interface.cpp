
#include "plugin/stereo/stereo_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/engine.h"
#include "engine/graphic/gfx_texture.h"

#include "plugin/stereo/stereo_plugin_interface.h"

#include "base/base_include_glm.h" // Some warnings appears when directly #include glm 
#include "opencv2/opencv.hpp" 


CORE_PLUGIN_INFO(Stereo::CPluginInterface, "Stereo Matching", "1.0", "This plugin takes RGB and transformation data and provides 2.5D depth maps")

namespace Stereo
{
    void CPluginInterface::SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
    {
        //---For variables which has not been used yet. Use this statement to avoid warning.---
        BASE_UNUSED(_rFocalLength); 
        BASE_UNUSED(_rFocalPoint); 
        //------
        m_ImageSize = _rImageSize;
    }

    // -----------------------------------------------------------------------------

    std::vector<char> CPluginInterface::GetLatestDepthImageCPU() const
    {
        return std::vector<char>();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics, const std::vector<uint16_t>& _rDepthImage)
    {

        //---Transform Image & Orientations to OpenCV format---
		glm::mat3 K_glm = glm::mat3(_Intrinsics) / 2; // Image is half resolution but Intrinsic is full resolution
		K_glm[2].z = 1;
        cv::Mat K_cv(3, 3, CV_32F);
        glm2cv(&K_cv, glm::transpose(K_glm));

        glm::mat3 R_glm = glm::mat3(_Transform);
        cv::Mat R_cv(3, 3, CV_32F);
        glm2cv(&R_cv, R_glm); // No transpose -> Rotation given by ARKit is Camera2World, but Rotation in Photogrammetry is World2Camera.

        glm::vec3 PC_glm = glm::vec3(_Transform[3]); // The last column of _Transform given by ARKit is the Position of Camera in World frame.
        cv::Mat PC_cv(3, 1, CV_32F);
        glm2cv(&PC_cv, PC_glm);

        cv::Mat Img_dist_cv(cv::Size(m_ImageSize.x, m_ImageSize.y), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
        memcpy(Img_dist_cv.data, _rRGBImage.data(), _rRGBImage.size());
        cv::cvtColor(Img_dist_cv, Img_dist_cv, cv::COLOR_BGRA2RGBA); // Transform image from BGRA (default color mode in OpenCV) to RGBA

        cv::Mat Img_Undist_cv, DistCoeff_cv(5, 1, CV_32F); // DistCoeff = K1, K2, P1, P2, K3
		DistCoeff_cv = cv::Mat::zeros(5, 1, CV_32F);
        cv::undistort(Img_dist_cv, Img_Undist_cv, K_cv, DistCoeff_cv);

		FutoGmtCV frame = FutoGmtCV(Img_Undist_cv, K_cv, R_cv, PC_cv);

        //---Select Keyframe for Computation---
		bool T = true;
		if (Keyframe_Curt == NULL)
		{
			Keyframe_Curt = &frame;
		}
		else if (T)
		{
			//---Keyframe Condition: Baseline Length---
			cv::Mat BaseLine = frame.get_PC() - Keyframe_Curt->get_PC();
			float BaseLineLength = cv::norm(BaseLine, cv::NORM_L2);

			//---Selecting Keyframe---
			if (BaseLineLength >= Cnd_Keyf_BaseLineL)
			{
				Keyframe_Last = Keyframe_Curt;
				Keyframe_Curt = &frame;
			}
			
			//---show Original Img for check---

			cv::imwrite("E:\\Project_ARCHITECT\\OrigImg_Curt.png", Keyframe_Curt->get_Img());
			cv::imwrite("E:\\Project_ARCHITECT\\OrigImg_Last.png", Keyframe_Last->get_Img());

			//---

			//---Epipolarization---
			FutoGmtCV RectImg_Curt, RectImg_Last;
			cv::Mat Orig2Rect_Curt_x, Orig2Rect_Curt_y, Orig2Rect_Last_x, Orig2Rect_Last_y; // Look-Up Table of Orig2Rect (for getting value by interpolation)

			Keyframe_Curt->imp_PlanarRect(RectImg_Curt, RectImg_Last, Orig2Rect_Curt_x, Orig2Rect_Curt_y, Orig2Rect_Last_x, Orig2Rect_Last_y, *Keyframe_Last);

			//---Verify by Test Data---
				   /*
				   cv::Mat TestInputL = cv::imread("E:\\Project_ARCHITECT\\01 Epipolarization\\Fusiello\\Testing Data\\01-002570.jpg");
				   cv::Mat TestInputR = cv::imread("E:\\Project_ARCHITECT\\01 Epipolarization\\Fusiello\\Testing Data\\02-002570.jpg");

				   cv::Mat K_L = cv::Mat::zeros(3, 3, CV_32F);
				   K_L.at<float>(0, 0) = 1280.465;
				   K_L.at<float>(1, 1) = 1280.465;
				   K_L.at<float>(0, 2) = 712.961;
				   K_L.at<float>(1, 2) = 515.829;
				   K_L.at<float>(2, 2) = 1;
				   cv::Mat K_R = cv::Mat::zeros(3, 3, CV_32F);
				   K_R.at<float>(0, 0) = 1281.566;
				   K_R.at<float>(1, 1) = 1281.566;
				   K_R.at<float>(0, 2) = 698.496;
				   K_R.at<float>(1, 2) = 511.008;
				   K_R.at<float>(2, 2) = 1;
				   cv::Mat PC_L = cv::Mat::zeros(3, 1, CV_32F);
				   cv::Mat PC_R = cv::Mat::zeros(3, 1, CV_32F);
				   PC_R.at<float>(0, 0) = 1.630;
				   PC_R.at<float>(1, 0) = 0.016;
				   PC_R.at<float>(2, 0) = -0.192;
				   cv::Mat R_L = cv::Mat::eye(3, 3, CV_32F);
				   cv::Mat R_R = cv::Mat::eye(3, 3, CV_32F);
				   R_R.at<float>(0, 0) = 0.9999;
				   R_R.at<float>(0, 1) = 0.0085;
				   R_R.at<float>(0, 2) = -0.0132;
				   R_R.at<float>(1, 0) = -0.0084;
				   R_R.at<float>(1, 1) = 0.9999;
				   R_R.at<float>(1, 2) = 0.0123;
				   R_R.at<float>(2, 0) = 0.0132;
				   R_R.at<float>(2, 1) = -0.0122;
				   R_R.at<float>(2, 2) = 0.9999;

				   FutoGmtCV TestImgL = FutoGmtCV(TestInputL, K_L, R_L, PC_L);
				   FutoGmtCV TestImgR = FutoGmtCV(TestInputR, K_R, R_R, PC_R);

				   TestImgL.imp_PlanarRect(RectImg_Curt, RectImg_Next, TableB_x_Orig2Rect, TableB_y_Orig2Rect, TableM_x_Orig2Rect, TableM_y_Orig2Rect, TestImgR);
				   */
			//---

			cv::Mat RectImg_Curt_Gray, RectImg_Last_Gray;
			cv::cvtColor(RectImg_Curt.get_Img(), RectImg_Curt_Gray, cv::COLOR_RGBA2GRAY);
			cv::cvtColor(RectImg_Last.get_Img(), RectImg_Last_Gray, cv::COLOR_RGBA2GRAY);
			
			//---Show Rectified Img for check---

			cv::imwrite("E:\\Project_ARCHITECT\\RectImg_Curt.png", RectImg_Curt_Gray);
			cv::imwrite("E:\\Project_ARCHITECT\\RectImg_Last.png", RectImg_Last_Gray);

			//---

			//---Stereo Matching---
			cv::Mat DispImg_Rect, DispImg_Orig;
			Keyframe_Curt->imp_cvSGBM(DispImg_Rect, RectImg_Curt_Gray, RectImg_Last_Gray);
			
			//---test SGBM in OpenCV---
					/*
					cv::Mat TestImgL = cv::imread("C:\\saltwater\\intern\\src\\plugin\\stereo\\Test\\im2.png");
					cv::Mat TestImgR = cv::imread("C:\\saltwater\\intern\\src\\plugin\\stereo\\Test\\im6.png");
					cv::imshow("TestImgL", TestImgL);
					cv::imshow("TestImgR", TestImgR);
					iter->imp_cvSGBM(DispImg_Curt, TestImgL, TestImgR);
					*/

			DispImg_Rect.convertTo(DispImg_Rect, CV_32F, 1.0 / 16); // Disparity Image is in 16-bit -> Divide by 16 to get real Disparity.
			DispImg_Orig = cv::Mat(Keyframe_Curt->get_Img().size(), CV_32F);
			cv::remap(DispImg_Rect, DispImg_Orig, Orig2Rect_Curt_x, Orig2Rect_Curt_y, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT); // !!! Warning: Using interpolation may cause additional errors !!!

			//---Show Disparity Image generated from Stereo Matching---
			cv::Mat DispImg_Rect_8U(DispImg_Rect.size(), CV_8UC1);
			cv::normalize(DispImg_Rect, DispImg_Rect_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
			cv::imwrite("E:\\Project_ARCHITECT\\Disp_Rect.png", DispImg_Rect_8U);

			cv::Mat DispImg_Orig_8U(DispImg_Orig.size(), CV_8UC1);
			cv::normalize(DispImg_Orig, DispImg_Orig_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
			cv::imwrite("E:\\Project_ARCHITECT\\Disp_Orig.png", DispImg_Orig_8U);

			//---Transform Disparity to Depth: Using Parallax Equation---
			cv::Mat DepthImg_Rect = cv::Mat::zeros(DispImg_Rect.size(), CV_32F);
			for (int x_idx = 0; x_idx < DispImg_Rect.size().width; x_idx++)
			{
				for (int y_idx = 0; y_idx < DispImg_Rect.size().height; y_idx++)
				{
					DepthImg_Rect.ptr<float>(y_idx)[x_idx] = RectImg_Curt.get_Cam().ptr<float>(0)[0] * BaseLineLength / DispImg_Rect.ptr<float>(y_idx)[x_idx];
				}
			}

			cv::Mat DepthImg_Orig = cv::Mat::zeros(DispImg_Orig.size(), CV_32F);
			cv::remap(DepthImg_Rect, DepthImg_Orig, Orig2Rect_Curt_x, Orig2Rect_Curt_y, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT); // !!! Warning: Using interpolation may cause additional errors !!!

			//---Show Depth Image---
			cv::Mat DepthImg_Rect_8U(DepthImg_Rect.size(), CV_8UC1);
			cv::normalize(DepthImg_Rect, DepthImg_Rect_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
			cv::imwrite("E:\\Project_ARCHITECT\\Depth_Rect.png", DepthImg_Rect_8U);
			cv::Mat DepthImg_Orig_8U(DepthImg_Orig.size(), CV_8UC1);
			cv::normalize(DepthImg_Orig, DepthImg_Orig_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
			cv::imwrite("E:\\Project_ARCHITECT\\Depth_Orig.png", DepthImg_Orig_8U);

			cv::Mat DepthImg_Orig_Sensor(cv::Size(m_ImageSize.x, m_ImageSize.y), CV_16UC1); // 2D Matrix(x*y) with (8-bit unsigned character & 1 Channel)
			memcpy(DepthImg_Orig_Sensor.data, _rDepthImage.data(), _rDepthImage.size() * sizeof(_rDepthImage[0]));
			cv::Mat DepthImg_Orig_Sensor_8U = cv::Mat(DepthImg_Orig_Sensor.size(), CV_8UC1);
			cv::normalize(DepthImg_Orig_Sensor, DepthImg_Orig_Sensor_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
			cv::imwrite("E:\\Project_ARCHITECT\\Depth_Orig_Sensor.png", DepthImg_Orig_Sensor_8U);

			T = false;
		}
        
        
        // Optional for internal check
        CPluginInterface::ShowImg(_rRGBImage); // Showing image for visual checking -> Modify to control in editor.config
        
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CPluginInterface::GetLatestDepthImageGPU() const
    {
        return nullptr;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4& _Transform)
    {
        BASE_UNUSED(_RGBImage);
        BASE_UNUSED(_Transform);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 9 * sizeof(float));
    }

    void CPluginInterface::glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 3 * sizeof(float));
    }

    void CPluginInterface::glm2cv(cv::Mat* cvmat, const glm::vec4& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 4 * sizeof(float));
    }

    void CPluginInterface::glm2cv(cv::Mat* cvmat, const glm::mat3x4& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 12 * sizeof(float));
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::ShowImg(const std::vector<char>& Img_RGBA) const
    {
        cv::Mat CV_Img(cv::Size(m_ImageSize.x, m_ImageSize.y), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
            // cv::Mat is built in BGR/BGRA in default.
            // cv::Mat is ImgH(Num of Row), ImgW(Num of Col)
        
        memcpy(CV_Img.data, Img_RGBA.data(), Img_RGBA.size());
        
        cv::cvtColor(CV_Img, CV_Img, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite
        
        cv::imshow("Hello", CV_Img);
	}

	// -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin exited!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin paused!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin resumed!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::EventHook()
    {
        ENGINE_CONSOLE_INFOV("Oh hello. An event has been raised!");
    }

} // namespace HW

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics, const std::vector<uint16_t>& _rDepthImage)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).OnFrameCPU(_rRGBImage, _Transform, _Intrinsics, _rDepthImage);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void GetLatestDepthImageCPU(std::vector<char>& _rDepthImage)
{
    _rDepthImage = static_cast<Stereo::CPluginInterface&>(GetInstance()).GetLatestDepthImageCPU();
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4& _Transform)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).OnFrameGPU(_RGBImage, _Transform);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void GetLatestDepthImageGPU(Gfx::CTexturePtr& _rDepthImage)
{
    _rDepthImage = static_cast<Stereo::CPluginInterface&>(GetInstance()).GetLatestDepthImageGPU();
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).SetIntrinsics(_rFocalLength, _rFocalPoint, _rImageSize);
}