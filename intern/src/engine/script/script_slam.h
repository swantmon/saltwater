
#pragma once

#include "base/base_compression.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"

#include "engine/core/core_program_parameters.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"

#include "engine/script/script_script.h"

#include "engine/network/core_network_manager.h"

namespace Scpt
{
    class CSLAMScript : public CScript<CSLAMScript>
    {
    private:

        enum EMessageType
        {
            COMMAND,
            TRANSFORM,
            DEPTHFRAME,
            COLORFRAME
        };

        typedef void(*InitializeCallback)(void);
        typedef void(*TerminateCallback)(void);
        typedef void(*ResetCallback)(void);
        typedef void(*DepthFrameCallback)(Gfx::CTexturePtr, Gfx::CTexturePtr, const glm::mat4*);
        typedef void(*SizeAndIntrinsicsCallback)(glm::vec4, glm::vec4);

        ResetCallback OnResetReconstruction;
        InitializeCallback OnInitializeReconstructor;
        TerminateCallback OnTerminateReconstructor;
        DepthFrameCallback OnNewFrame;
        SizeAndIntrinsicsCallback OnSetImageSizesAndIntrinsics;

    private:

        enum EDATASOURCE
        {
            NETWORK,
            KINECT
        };

        EDATASOURCE m_DataSource;
        
        Gfx::CTexturePtr m_DepthTexture;
        Gfx::CTexturePtr m_RGBTexture;
        Gfx::CTexturePtr m_YTexture;
        Gfx::CTexturePtr m_UVTexture;
        uint16_t* m_DepthBuffer;
        glm::mat4 m_PoseMatrix;

        glm::ivec2 m_DepthSize;
        glm::ivec2 m_ColorSize;

        bool IsReconstructorInitialized = false;

        const int m_TileSize2D = 16;

        // -----------------------------------------------------------------------------
        // Stuff for network data source
        // -----------------------------------------------------------------------------
        std::shared_ptr<Net::CMessageDelegate> m_NetworkDelegate;

        Gfx::CShaderPtr m_YUVtoRGBCSPtr;

        // -----------------------------------------------------------------------------
        // Stuff for Kinect data source
        // -----------------------------------------------------------------------------
        typedef bool(*GetDepthBufferFunc)(uint16_t*);
        GetDepthBufferFunc GetDepthBuffer;

    public:

        void Start() override
        {
            // -----------------------------------------------------------------------------
            // Load SLAM plugin
            // -----------------------------------------------------------------------------
            if (!Core::PluginManager::HasPlugin("SLAM"))
            {
                throw Base::CException(__FILE__, __LINE__, "SLAM plugin was not loaded");
            }

            OnNewFrame = (DepthFrameCallback)(Core::PluginManager::GetPluginFunction("SLAM", "OnNewDepthFrame"));
            OnInitializeReconstructor = (InitializeCallback)(Core::PluginManager::GetPluginFunction("SLAM", "InitializeReconstructor"));
            OnTerminateReconstructor = (TerminateCallback)(Core::PluginManager::GetPluginFunction("SLAM", "TerminateReconstructor"));
            OnSetImageSizesAndIntrinsics = (SizeAndIntrinsicsCallback)(Core::PluginManager::GetPluginFunction("SLAM", "SetImageSizesAndIntrinsicData"));
            OnResetReconstruction = (ResetCallback)(Core::PluginManager::GetPluginFunction("SLAM", "ResetReconstruction"));

            // -----------------------------------------------------------------------------
            // Determine where we get our data from
            // -----------------------------------------------------------------------------
            std::string DataSource = Core::CProgramParameters::GetInstance().Get("mr:slam:data_source", "network");

            if (DataSource == "network")
            {
                // -----------------------------------------------------------------------------
                // Create network connection
                // -----------------------------------------------------------------------------
                m_NetworkDelegate = std::shared_ptr<Net::CMessageDelegate>(new Net::CMessageDelegate(std::bind(&CSLAMScript::OnNewMessage, this, std::placeholders::_1, std::placeholders::_2)));

                Net::CNetworkManager::GetInstance().RegisterMessageHandler(0, m_NetworkDelegate);

                m_DataSource = NETWORK;
            }
            else if (DataSource == "kinect")
            {
                // -----------------------------------------------------------------------------
                // Load Kinect plugin
                // -----------------------------------------------------------------------------
                if (!Core::PluginManager::HasPlugin("Kinect"))
                {
                    throw Base::CException(__FILE__, __LINE__, "Kinect plugin was not loaded");
                }

                m_DataSource = KINECT;

                typedef void(*GetIntrinsicsFunc)(glm::vec2&, glm::vec2&, glm::ivec2&);

                GetIntrinsicsFunc GetIntrinsics = (GetIntrinsicsFunc)(Core::PluginManager::GetPluginFunction("Kinect", "GetIntrinsics"));

                glm::vec2 FocalLength;
                glm::vec2 FocalPoint;
                
                GetIntrinsics(FocalLength, FocalPoint, m_DepthSize);
                m_ColorSize = m_DepthSize;

                OnSetImageSizesAndIntrinsics(glm::vec4(m_DepthSize, m_ColorSize), glm::vec4(FocalLength, FocalPoint));

                OnInitializeReconstructor();

                IsReconstructorInitialized = true;

                m_DepthBuffer = new uint16_t[m_DepthSize.x * m_DepthSize.y];

                Gfx::STextureDescriptor TextureDescriptor = {};

                TextureDescriptor.m_NumberOfPixelsU = m_DepthSize.x;
                TextureDescriptor.m_NumberOfPixelsV = m_DepthSize.y;
                TextureDescriptor.m_NumberOfPixelsW = 1;
                TextureDescriptor.m_NumberOfMipMaps = 1;
                TextureDescriptor.m_NumberOfTextures = 1;
                TextureDescriptor.m_Binding = Gfx::CTexture::ShaderResource;
                TextureDescriptor.m_Access = Gfx::CTexture::CPUWrite;
                TextureDescriptor.m_Usage = Gfx::CTexture::GPUReadWrite;
                TextureDescriptor.m_Semantic = Gfx::CTexture::UndefinedSemantic;
                TextureDescriptor.m_pFileName = nullptr;
                TextureDescriptor.m_pPixels = 0;
                TextureDescriptor.m_Format = Gfx::CTexture::R16_UINT;

                m_DepthTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                GetDepthBuffer = (GetDepthBufferFunc)(Core::PluginManager::GetPluginFunction("Kinect", "GetDepthBuffer"));
            }
            else
            {
                throw Base::CException(__FILE__, __LINE__, "Unknown data source for SLAM plugin");
            }
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            if (IsReconstructorInitialized)
            {
                OnTerminateReconstructor();
            }
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (m_DataSource == KINECT && GetDepthBuffer(m_DepthBuffer))
            {
                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_DepthSize.x, m_DepthSize.y));
                Gfx::TextureManager::CopyToTexture2D(m_DepthTexture, TargetRect, m_DepthSize.x, const_cast<uint16_t*>(m_DepthBuffer));

                OnNewFrame(m_DepthTexture, nullptr, nullptr);
            }
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }

    private:

        void OnNewMessage(const Net::CMessage& _rMessage, int _Port)
        {
            BASE_UNUSED(_Port);

            std::vector<char> Decompressed(_rMessage.m_DecompressedSize);

            if (_rMessage.m_CompressedSize != _rMessage.m_DecompressedSize)
            {   
                Base::Decompress(_rMessage.m_Payload, Decompressed);
            }
            else
            {
                std::memcpy(Decompressed.data(), _rMessage.m_Payload.data(), Decompressed.size());
            }

            int32_t MessageType = *reinterpret_cast<int32_t*>(Decompressed.data());
            
            if (MessageType == COMMAND)
            {
                const int MessageID = *reinterpret_cast<int32_t*>(Decompressed.data() + sizeof(int32_t));

                if (MessageID == 0)
                {
                    OnResetReconstruction();
                }
                else if(MessageID == 1)
                {
                    ENGINE_CONSOLE_INFO("Initializing reconstructor");

                    glm::vec2 FocalLength = *reinterpret_cast<glm::vec2* >(Decompressed.data() + sizeof(int32_t) * 2);
                    glm::vec2 FocalPoint  = *reinterpret_cast<glm::vec2* >(Decompressed.data() + sizeof(int32_t) * 2 + sizeof(glm::vec2));
                    m_DepthSize  = *reinterpret_cast<glm::ivec2*>(Decompressed.data() + sizeof(int32_t) * 2 + sizeof(glm::vec2) * 2);
                    //m_ColorSize  = *reinterpret_cast<glm::ivec2*>(Decompressed.data() + sizeof(int32_t) * 2 + sizeof(glm::vec2) * 2 + sizeof(glm::ivec2));
                    m_ColorSize = glm::ivec2(640, 360);

                    OnSetImageSizesAndIntrinsics(glm::vec4(m_DepthSize, m_ColorSize), glm::vec4(FocalLength, FocalPoint));

                    OnInitializeReconstructor();

                    IsReconstructorInitialized = true;

                    m_DepthBuffer = new uint16_t[m_DepthSize.x * m_DepthSize.y];

                    Gfx::STextureDescriptor TextureDescriptor = {};

                    TextureDescriptor.m_NumberOfPixelsU = m_DepthSize.x;
                    TextureDescriptor.m_NumberOfPixelsV = m_DepthSize.y;
                    TextureDescriptor.m_NumberOfPixelsW = 1;
                    TextureDescriptor.m_NumberOfMipMaps = 1;
                    TextureDescriptor.m_NumberOfTextures = 1;
                    TextureDescriptor.m_Binding = Gfx::CTexture::ShaderResource;
                    TextureDescriptor.m_Access = Gfx::CTexture::CPUWrite;
                    TextureDescriptor.m_Usage = Gfx::CTexture::GPUReadWrite;
                    TextureDescriptor.m_Semantic = Gfx::CTexture::UndefinedSemantic;
                    TextureDescriptor.m_pFileName = nullptr;
                    TextureDescriptor.m_pPixels = 0;
                    TextureDescriptor.m_Format = Gfx::CTexture::R16_UINT;
                    m_DepthTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                    TextureDescriptor.m_NumberOfPixelsU = m_ColorSize.x;
                    TextureDescriptor.m_NumberOfPixelsV = m_ColorSize.y;
                    TextureDescriptor.m_Format = Gfx::CTexture::R8G8B8A8_UBYTE;
                    m_RGBTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                    TextureDescriptor.m_Format = Gfx::CTexture::R8_UBYTE;
                    m_YTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                    TextureDescriptor.m_NumberOfPixelsU = m_ColorSize.x / 2;
                    TextureDescriptor.m_NumberOfPixelsV = m_ColorSize.y / 2;
                    TextureDescriptor.m_Format = Gfx::CTexture::R8G8_UBYTE;
                    m_UVTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                    std::stringstream DefineStream;
                    DefineStream
                        << "#define TILE_SIZE_2D " << m_TileSize2D << " \n";
                    std::string DefineString = DefineStream.str();
                    m_YUVtoRGBCSPtr = Gfx::ShaderManager::CompileCS("slam\\cs_yuv_to_rgb.glsl", "main", DefineString.c_str());

                    ENGINE_CONSOLE_INFO("Initialization complete");
                }
            }
            else if (MessageType == TRANSFORM)
            {
                m_PoseMatrix = *reinterpret_cast<glm::mat4*>(Decompressed.data() + sizeof(int32_t)) * glm::eulerAngleX(glm::pi<float>());
            }
            else if (MessageType == DEPTHFRAME)
            {
                int32_t Width = *reinterpret_cast<int32_t*>(Decompressed.data() + sizeof(int32_t));
                int32_t Height = *reinterpret_cast<int32_t*>(Decompressed.data() + 2 * sizeof(int32_t));
                
                const uint16_t* RawBuffer = reinterpret_cast<uint16_t*>(Decompressed.data() + 3 * sizeof(int32_t));

                std::vector<char> Message(640 * 480);

                for (int i = 0; i < Width; ++ i)
                {
                    for (int j = 0; j < Height; ++ j)
                    {
                        m_DepthBuffer[j * Width + i] = shift2depth(RawBuffer[j * Width + (Width - 1 - i)]);
                        Message[(Height - j - 1) * Width + (Width - i - 1)] = static_cast<char>((m_DepthBuffer[j * Width + i] / 3000.0f) * 255.0f);
                    }
                }

                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(Width, Height));
                Gfx::TextureManager::CopyToTexture2D(m_DepthTexture, TargetRect, Width, const_cast<uint16_t*>(m_DepthBuffer));

                OnNewFrame(m_DepthTexture, nullptr, &m_PoseMatrix);
                
                /*std::vector<char> Compressed;
                Base::Compress(Message, Compressed, 1);
                Net::CNetworkManager::GetInstance().SendMessage(0, Compressed);*/
            }
            else if (MessageType == COLORFRAME)
            {
                const int32_t Width = *reinterpret_cast<int32_t*>(Decompressed.data() + sizeof(int32_t));
                const int32_t Height = *reinterpret_cast<int32_t*>(Decompressed.data() + 2 * sizeof(int32_t));
                
                const char* YData = Decompressed.data() + 3 * sizeof(int32_t);
                const char* UVData = YData + Width * Height;

                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_ColorSize.x, m_ColorSize.y));
                Gfx::TextureManager::CopyToTexture2D(m_YTexture, TargetRect, m_ColorSize.x, const_cast<char*>(YData));

                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_ColorSize.x / 2, m_ColorSize.y / 2));
                Gfx::TextureManager::CopyToTexture2D(m_UVTexture, TargetRect, m_ColorSize.x / 2, const_cast<char*>(UVData));

                Gfx::ContextManager::SetShaderCS(m_YUVtoRGBCSPtr);
                Gfx::ContextManager::SetImageTexture(0, m_YTexture);
                Gfx::ContextManager::SetImageTexture(1, m_UVTexture);
                Gfx::ContextManager::SetImageTexture(2, m_RGBTexture);

                Gfx::ContextManager::Dispatch(DivUp(m_ColorSize.x, m_TileSize2D), DivUp(m_ColorSize.y, m_TileSize2D), 1);

                //OnNewFrame(m_DepthTexture, m_ColorTexture, &m_PoseMatrix);
            }
        }

        uint16_t shift2depth(uint16_t shift)
        {
            static const uint16_t table[] = { 0,
                264, 264, 265, 265, 265, 265, 265, 266, 266, 266, 266, 267, 267, 267, 267, 268, 268, 268,
                268, 269, 269, 269, 269, 270, 270, 270, 270, 271, 271, 271, 271, 272, 272, 272, 272, 273,
                273, 273, 273, 274, 274, 274, 274, 275, 275, 275, 275, 276, 276, 276, 276, 277, 277, 277,
                277, 278, 278, 278, 278, 279, 279, 279, 279, 280, 280, 280, 280, 281, 281, 281, 281, 282,
                282, 282, 283, 283, 283, 283, 284, 284, 284, 284, 285, 285, 285, 286, 286, 286, 286, 287,
                287, 287, 287, 288, 288, 288, 289, 289, 289, 289, 290, 290, 290, 291, 291, 291, 291, 292,
                292, 292, 293, 293, 293, 293, 294, 294, 294, 295, 295, 295, 295, 296, 296, 296, 297, 297,
                297, 297, 298, 298, 298, 299, 299, 299, 300, 300, 300, 300, 301, 301, 301, 302, 302, 302,
                303, 303, 303, 304, 304, 304, 304, 305, 305, 305, 306, 306, 306, 307, 307, 307, 308, 308,
                308, 309, 309, 309, 309, 310, 310, 310, 311, 311, 311, 312, 312, 312, 313, 313, 313, 314,
                314, 314, 315, 315, 315, 316, 316, 316, 317, 317, 317, 318, 318, 318, 319, 319, 319, 320,
                320, 320, 321, 321, 321, 322, 322, 322, 323, 323, 324, 324, 324, 325, 325, 325, 326, 326,
                326, 327, 327, 327, 328, 328, 329, 329, 329, 330, 330, 330, 331, 331, 331, 332, 332, 333,
                333, 333, 334, 334, 334, 335, 335, 336, 336, 336, 337, 337, 337, 338, 338, 339, 339, 339,
                340, 340, 340, 341, 341, 342, 342, 342, 343, 343, 344, 344, 344, 345, 345, 346, 346, 346,
                347, 347, 348, 348, 348, 349, 349, 350, 350, 350, 351, 351, 352, 352, 353, 353, 353, 354,
                354, 355, 355, 355, 356, 356, 357, 357, 358, 358, 358, 359, 359, 360, 360, 361, 361, 361,
                362, 362, 363, 363, 364, 364, 365, 365, 365, 366, 366, 367, 367, 368, 368, 369, 369, 369,
                370, 370, 371, 371, 372, 372, 373, 373, 374, 374, 375, 375, 376, 376, 376, 377, 377, 378,
                378, 379, 379, 380, 380, 381, 381, 382, 382, 383, 383, 384, 384, 385, 385, 386, 386, 387,
                387, 388, 388, 389, 389, 390, 390, 391, 391, 392, 392, 393, 393, 394, 394, 395, 395, 396,
                396, 397, 397, 398, 399, 399, 400, 400, 401, 401, 402, 402, 403, 403, 404, 404, 405, 406,
                406, 407, 407, 408, 408, 409, 409, 410, 411, 411, 412, 412, 413, 413, 414, 415, 415, 416,
                416, 417, 417, 418, 419, 419, 420, 420, 421, 422, 422, 423, 423, 424, 425, 425, 426, 426,
                427, 428, 428, 429, 429, 430, 431, 431, 432, 433, 433, 434, 434, 435, 436, 436, 437, 438,
                438, 439, 439, 440, 441, 441, 442, 443, 443, 444, 445, 445, 446, 447, 447, 448, 449, 449,
                450, 451, 451, 452, 453, 453, 454, 455, 456, 456, 457, 458, 458, 459, 460, 460, 461, 462,
                463, 463, 464, 465, 465, 466, 467, 468, 468, 469, 470, 471, 471, 472, 473, 474, 474, 475,
                476, 477, 477, 478, 479, 480, 480, 481, 482, 483, 484, 484, 485, 486, 487, 487, 488, 489,
                490, 491, 491, 492, 493, 494, 495, 496, 496, 497, 498, 499, 500, 500, 501, 502, 503, 504,
                505, 506, 506, 507, 508, 509, 510, 511, 512, 512, 513, 514, 515, 516, 517, 518, 519, 520,
                520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 533, 534, 535, 536,
                537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554,
                555, 556, 557, 558, 559, 560, 561, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573,
                574, 575, 577, 578, 579, 580, 581, 582, 583, 584, 586, 587, 588, 589, 590, 591, 593, 594,
                595, 596, 597, 599, 600, 601, 602, 603, 605, 606, 607, 608, 609, 611, 612, 613, 614, 616,
                617, 618, 620, 621, 622, 623, 625, 626, 627, 629, 630, 631, 633, 634, 635, 637, 638, 639,
                641, 642, 644, 645, 646, 648, 649, 650, 652, 653, 655, 656, 658, 659, 661, 662, 663, 665,
                666, 668, 669, 671, 672, 674, 675, 677, 678, 680, 682, 683, 685, 686, 688, 689, 691, 693,
                694, 696, 697, 699, 701, 702, 704, 706, 707, 709, 711, 712, 714, 716, 717, 719, 721, 723,
                724, 726, 728, 730, 732, 733, 735, 737, 739, 741, 742, 744, 746, 748, 750, 752, 754, 755,
                757, 759, 761, 763, 765, 767, 769, 771, 773, 775, 777, 779, 781, 783, 785, 787, 789, 791,
                794, 796, 798, 800, 802, 804, 806, 808, 811, 813, 815, 817, 820, 822, 824, 826, 829, 831,
                833, 836, 838, 840, 843, 845, 847, 850, 852, 855, 857, 860, 862, 864, 867, 869, 872, 875,
                877, 880, 882, 885, 888, 890, 893, 895, 898, 901, 904, 906, 909, 912, 915, 917, 920, 923,
                926, 929, 932, 935, 937, 940, 943, 946, 949, 952, 955, 958, 962, 965, 968, 971, 974, 977,
                980, 984, 987, 990, 993, 997, 1000, 1003, 1007, 1010, 1014, 1017, 1020, 1024, 1027, 1031,
                1035, 1038, 1042, 1045, 1049, 1053, 1056, 1060, 1064, 1068, 1072, 1075, 1079, 1083, 1087,
                1091, 1095, 1099, 1103, 1107, 1111, 1115, 1120, 1124, 1128, 1132, 1137, 1141, 1145, 1150,
                1154, 1159, 1163, 1168, 1172, 1177, 1181, 1186, 1191, 1196, 1200, 1205, 1210, 1215, 1220,
                1225, 1230, 1235, 1240, 1245, 1250, 1256, 1261, 1266, 1272, 1277, 1282, 1288, 1294, 1299,
                1305, 1310, 1316, 1322, 1328, 1334, 1340, 1346, 1352, 1358, 1364, 1370, 1377, 1383, 1389,
                1396, 1402, 1409, 1416, 1422, 1429, 1436, 1443, 1450, 1457, 1464, 1471, 1479, 1486, 1493,
                1501, 1508, 1516, 1524, 1531, 1539, 1547, 1555, 1563, 1572, 1580, 1588, 1597, 1605, 1614,
                1623, 1631, 1640, 1649, 1658, 1668, 1677, 1686, 1696, 1706, 1715, 1725, 1735, 1745, 1756,
                1766, 1776, 1787, 1798, 1809, 1820, 1831, 1842, 1853, 1865, 1876, 1888, 1900, 1912, 1925,
                1937, 1950, 1962, 1975, 1988, 2002, 2015, 2029, 2043, 2057, 2071, 2085, 2100, 2115, 2130,
                2145, 2160, 2176, 2192, 2208, 2224, 2241, 2258, 2275, 2292, 2310, 2328, 2346, 2365, 2384,
                2403, 2422, 2442, 2462, 2482, 2503, 2524, 2545, 2567, 2589, 2612, 2635, 2658, 2682, 2706,
                2731, 2756, 2782, 2808, 2834, 2861, 2889, 2917, 2945, 2975, 3005, 3035, 3066, 3098, 3130,
                3163, 3197, 3231, 3266, 3302, 3339, 3377, 3415, 3454, 3495, 3536, 3578, 3621, 3665, 3711,
                3757, 3805, 3854, 3904, 3956, 4008, 4063, 4118, 4176, 4235, 4295, 4358, 4422, 4488, 4556,
                4627, 4699, 4774, 4851, 4931, 5013, 5099, 5187, 5278, 5373, 5471, 5572, 5678, 5787, 5901,
                6020, 6143, 6271, 6405, 6545, 6691, 6844, 7003, 7171, 7346, 7531, 7725, 7929, 8144, 8372,
                8612, 8866, 9137, 9424, 9729
            };

            return shift < (sizeof(table) / sizeof(table[0])) ? table[shift] : 0;
        }

        int DivUp(int TotalShaderCount, int WorkGroupSize)
        {
            return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
        }
    };
} // namespace Scpt