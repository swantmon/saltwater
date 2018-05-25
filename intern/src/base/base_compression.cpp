
#include "base/base_precompiled.h"

#include "base/base_exception.h"

#include <zlib.h>

namespace
{
    void CheckResult(int _Result)
    {
        if (_Result != Z_OK)
        {
            throw Base::CException(__FILE__, __LINE__, "Failed to decompress");
        }
    }
}

namespace Base
{
    void Decompress(const std::vector<char> _rCompressedData, std::vector<char> _rDecompressedData)
    {
        z_stream infstream;
        infstream.zalloc = Z_NULL;
        infstream.zfree = Z_NULL;
        infstream.opaque = Z_NULL;
        infstream.avail_in = static_cast<uInt>(_rCompressedData.size());
        infstream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(_rCompressedData.data()));
        infstream.avail_out = static_cast<uInt>(_rDecompressedData.size());
        infstream.next_out = reinterpret_cast<Bytef*>(_rDecompressedData.data());
        
        CheckResult(inflateInit2(&infstream, 16 + MAX_WBITS));
        CheckResult(inflate(&infstream, Z_SYNC_FLUSH));
        CheckResult(inflateEnd(&infstream));
    }
} // namespace Base