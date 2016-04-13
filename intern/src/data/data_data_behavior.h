
#pragma once

namespace Dt
{
    struct SDataBehavior
    {
        enum Enum
        {
            Listen            =  0,     ///< Manager listen on source data and do not delete memory when the corresponding resource is destructed.
            TakeOwnerShip     =  1,     ///< Manager takes ownership of the source data and deletes it when the corresponding resource is destructed.
            Copy              =  2,     ///< Manager makes a copy of the source data, which is deleted when the corresponding resource is destructed.
            CopyAndDelete     =  3,     ///< Manager makes a copy of the source data, deletes the source data after the upload and deletes the copy when the corresponding resource is destructed.
            NumberOfMembers   =  4,
            Undefined         = -1,
        };
    };
} // namespace Dt
