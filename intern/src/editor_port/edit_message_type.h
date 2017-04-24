
#pragma once

namespace Edit
{
    // ----------------------------------------------------------------------------------
    // Messages from GUI to application
    // ----------------------------------------------------------------------------------
    struct SGUIMessageType 
    {
        enum Enum
        {
            Input_MouseMove,
            Input_MouseLeftPressed,
            Input_MouseLeftReleased,
            Input_MouseMiddlePressed,
            Input_MouseMiddleReleased,
            Input_MouseRightPressed,
            Input_MouseRightReleased,
            Input_MouseWheel,
            Input_KeyPressed,
            Input_KeyReleased,

            Window_Resize,

            App_Exit,
            App_NewMap,
            App_LoadMap,
            App_SaveMap,
            App_SaveMapAs,
            App_Play,
            App_Edit,

            Entity_New,
            Entity_Load,
            Entity_Create,
            Entity_Add,
            Entity_Remove, 
            Entity_Destroy,
            Entity_Info,
            Entity_Info_Update,
            Entity_Facets_Info,
            Entity_Transformation_Info,
            Entity_Transformation_Update,
            Entity_Hierarchy_Info,
            Entity_Hierarchy_Update,
            
            Actor_Camera_New,
            Actor_Camera_Info,
            Actor_Camera_Update,

            Actor_Mesh_New,
            Actor_Material_Info,
            Actor_Material_Update,

            Light_Pointlight_New,
            Light_Pointlight_Info,
            Light_Pointlight_Update,

            Light_Arealight_New,
            Light_Arealight_Info,
            Light_Arealight_Update,

            Light_Sun_New,
            Light_Sun_Info,
            Light_Sun_Update,

            Light_Environment_New,
            Light_Environment_Info,
            Light_Environment_Update,

            Light_Probe_New,
            Light_Probe_Info,
            Light_Probe_Update,

            Effect_Bloom_New,
            Effect_Bloom_Info,
            Effect_Bloom_Update,

            Effect_DOF_New,
            Effect_DOF_Info,
            Effect_DOF_Update,

            Effect_PostAA_New,
            Effect_PostAA_Info,
            Effect_PostAA_Update,

            Effect_SSAO_New,
            Effect_SSAO_Info,
            Effect_SSAO_Update,

            Effect_SSR_New,
            Effect_SSR_Info,
            Effect_SSR_Update,

            Effect_VolumeFog_New,
            Effect_VolumeFog_Info,
            Effect_VolumeFog_Update,

            Plugin_ARConroller_New,
            Plugin_ARConroller_Info,
            Plugin_ARConroller_Update,
            Plugin_ARConroller_Marker_Info,
            Plugin_ARConroller_Marker_Update,
            
            Texture_New,
            Texture_Load,
            Texture_Remove,
            Texture_Info,
            Texture_Update,

            Material_New,
            Material_Load,
            Material_Remove,
            Material_Info,
            Material_Update,
            
            Graphic_Histogram_Info,
            Graphic_Histogram_Update,

            Graphic_ToneMapping_Info,
            Graphic_ToneMapping_Update,

            Graphic_HighlightEntity,

            Graphic_ReloadRenderer,
            Graphic_ReloadAllShader,

            MR_SLAM_NewReconstruction,
            MR_SLAM_Reconstruction_Update,
            MR_SLAM_Reconstruction_Pause_Integration,
            MR_SLAM_Reconstruction_Pause_Tracking,

            NumberOfMembers,
            Undefined = -1,
        };
    };

    // ----------------------------------------------------------------------------------
    // Messages from application to GUI
    // ----------------------------------------------------------------------------------
    struct SApplicationMessageType 
    {
        enum Enum
        {
            App_State_Change,

            Entity_Info,
            Entity_Facets_Info,
            Entity_Transformation_Info,
            Entity_Hierarchy_Info,
            Entity_Selected,

            Actor_Camera_Info,

            Actor_Material_Info,

            Light_Pointlight_Info,
            Light_Sun_Info,
            Light_Environment_Info,
            Light_Probe_Info,
            Light_Arealight_Info,

            Effect_Bloom_Info,
            Effect_DOF_Info,
            Effect_PostAA_Info,
            Effect_SSAO_Info,
            Effect_SSR_Info,
            Effect_VolumeFog_Info,

            Texture_Info,

            Material_Info,

            Plugin_ARController_Info,
            Plugin_ARController_Marker_Info,

            Graphic_FPS_Info,
            Graphic_Histogram_Info,
            Graphic_ToneMapping_Info,

            NumberOfMembers,
            Undefined = -1,
        };
    };
} // namespace Edit