
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
            MouseMove,
            MouseLeftPressed,
            MouseLeftReleased,
            MouseMiddlePressed,
            MouseMiddleReleased,
            MouseRightPressed,
            MouseRightReleased,
            MouseWheel,
            KeyPressed,
            KeyReleased,
            ResizeMapEditWindow,
            FocusMapEditWindow,
            Exit,
            NewMap,
            LoadMap,
            SaveMap,
            SaveMapAs,
            TakeScreenshot,
            Play,
            Edit,

            NewEntityActor,
            NewEntityPointlight,
            NewEntitySun,
            NewEntityEnvironment,
            NewEntityGlobalProbe,
            NewEntityBloom,
            NewEntityDOF,
            NewEntityFXAA,
            NewEntitySSR,
            NewEntityVolumeFog,

            RemoveEntity,

            RequestEntityInfoFacets,
            RequestEntityInfoEntity,
            RequestEntityInfoTransformation,
            RequestEntityInfoPointlight,
            RequestEntityInfoSun,
            RequestEntityInfoEnvironment,
            RequestEntityInfoGlobalProbe,
            RequestEntityInfoBloom,
            RequestEntityInfoDOF,
            RequestEntityInfoFXAA,
            RequestEntityInfoSSR,
            RequestEntityInfoVolumeFog,

            RequestHistogramInfo,

            EntityInfoEntity,
            EntityInfoHierarchie,
            EntityInfoTransformation,
            EntityInfoPointlight,
            EntityInfoSun,
            EntityInfoEnvironment,
            EntityInfoGlobalProbe,
            EntityInfoBloom,
            EntityInfoDOF,
            EntityInfoFXAA,
            EntityInfoSSR,
            EntityInfoVolumeFog,

            HistogramInfo,

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
            FramesPerSecond,

            SceneGraphChanged,

            EntityInfoFacets,
            EntityInfoEntity,
            EntityInfoTransformation,
            EntityInfoPointlight,
            EntityInfoSun,
            EntityInfoEnvironment,
            EntityInfoGlobalProbe,
            EntityInfoBloom,
            EntityInfoDOF,
            EntityInfoFXAA,
            EntityInfoSSR,
            EntityInfoVolumeFog,

            HistogramInfo,

            NumberOfMembers,
            Undefined = -1,
        };
    };
} // namespace Edit