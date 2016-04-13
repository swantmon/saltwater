
#pragma once

namespace Gfx
{
    struct SDataBehavior
    {
        enum Enum
        {
            LeftAlone         =  0,     ///< Manager does nothing with the source data. 
            DeleteAfterUpload =  1,     ///< Manager deletes the source data after it has been uploaded to the GPU driver.
            TakeOwnerShip     =  2,     ///< Manager takes ownership of the source data and deletes it when the corresponding resource is destructed.
            Copy              =  3,     ///< Manager makes a copy of the source data, which is deleted when the corresponding resource is destructed.
            CopyAndDelete     =  4,     ///< Manager makes a copy of the source data, deletes the source data after the upload and deletes the copy when the corresponding resource is destructed.
            NumberOfMembers   =  5,
            Undefined         = -1,
        };
    };
} // namespace Gfx
