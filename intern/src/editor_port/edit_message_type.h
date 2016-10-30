
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
            NewLightDirectional,
            NewLightPoint,
            NewLightEnvironment,
            NewLightGlobalProbe,

            RequestEntityInfoFacets,
            RequestEntityInfoTransformation,
            RequestEntityInfoPointlight,
            RequestEntityInfoEnvironment,
            RequestEntityInfoGlobalProbe,

            EntityInfoTransformation,
            EntityInfoPointlight,
            EntityInfoEnvironment,
            EntityInfoGlobalProbe,

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
            EntityInfoTransformation,
            EntityInfoPointlight,
            EntityInfoEnvironment,
            EntityInfoGlobalProbe,

            NumberOfMembers,
            Undefined = -1,
        };
    };
} // namespace Edit