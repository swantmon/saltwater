
#pragma once

namespace Gfx
{
    struct STopology
    {
        enum Enum
        {
            PointList,
            LineList,
            LineStrip,
            LineLoop,
            TriangleList,
            TriangleStrip,
            TriangleFan,
            Quad,
            QuadStrip,
            Polygon,
            Patches,
            Undefined = -1,
        };
    };
} // namespace Gfx