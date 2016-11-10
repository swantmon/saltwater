
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

            NewActorModel,
            NewActorCamera,

            NewLightPointlight,
            NewLightSun,
            NewLightEnvironment,
            NewLightGlobalProbe,

            NewEffectBloom,
            NewEffectDOF,
            NewEffectFXAA,
            NewEffectSSR,
            NewEffectVolumeFog,

            NewPluginARController,

            RemoveEntity,

            RequestEntityInfoFacets,
            RequestEntityInfoEntity,
            RequestEntityInfoTransformation,

            RequestLightInfoPointlight,
            RequestLightInfoSun,
            RequestLightInfoEnvironment,
            RequestLightInfoGlobalProbe,

            RequestEffectInfoBloom,
            RequestEffectInfoDOF,
            RequestEffectInfoFXAA,
            RequestEffectInfoSSR,
            RequestEffectInfoVolumeFog,

            RequestActorInfoMaterial,
            RequestActorInfoCamera,

            RequestPluginInfoARController,
            RequestPluginInfoARControllerMarker,

            RequestGraphicHistogramInfo,

            EntityInfoEntity,
            EntityInfoHierarchie,
            EntityInfoTransformation,

            LightInfoPointlight,
            LightInfoSun,
            LightInfoEnvironment,
            LightInfoGlobalProbe,

            EffectInfoBloom,
            EffectInfoDOF,
            EffectInfoFXAA,
            EffectInfoSSR,
            EffectInfoVolumeFog,

            ActorInfoMaterial,
            ActorInfoCamera,

            PluginInfoARController,
            PluginInfoARControllerMarker,

            GraphicHistogramInfo,

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

            EntityInfoFacets,
            EntityInfoEntity,
            EntityInfoTransformation,
            EntityInfoHierarchy,

            LightInfoPointlight,
            LightInfoSun,
            LightInfoEnvironment,
            LightInfoGlobalProbe,

            EffectInfoBloom,
            EffectInfoDOF,
            EffectInfoFXAA,
            EffectInfoSSR,
            EffectInfoVolumeFog,

            ActorInfoMaterial,
            ActorInfoCamera,

            PluginInfoARController,
            PluginInfoARControllerMarker,

            HistogramInfo,

            NumberOfMembers,
            Undefined = -1,
        };
    };
} // namespace Edit