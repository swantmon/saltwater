
#include "base/base_precompiled.h"

#include "base/base_exception.h"

#include <zlib.h>

#include <cassert>

namespace
{
    void CheckResult(int _Result)
    {
        if (_Result != Z_OK && _Result != Z_STREAM_END)
        {
            BASE_THROWM("Failed to decompress")
        }
    }

    const int CompressionLevels[] =
    {
        Z_NO_COMPRESSION,
        Z_BEST_SPEED,
        Z_BEST_COMPRESSION,
        Z_DEFAULT_COMPRESSION
    };
}

namespace Base
{
    void Decompress(const std::vector<char>& _rCompressedData, std::vector<char>& _rDecompressedData)
    {
        assert(_rDecompressedData.size() >= _rCompressedData.size()); // TODO: allow empty result vector and figure out output length with zlib

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

    // -----------------------------------------------------------------------------

    void Compress(const std::vector<char>& _rDecompressedData, std::vector<char>& _rCompressedData, int _Level = 1)
    {
        _rCompressedData.resize(_rDecompressedData.size());

        z_stream defstream;
        defstream.zalloc = Z_NULL;
        defstream.zfree = Z_NULL;
        defstream.opaque = Z_NULL;
        defstream.avail_in = static_cast<uInt>(_rDecompressedData.size());
        defstream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(_rDecompressedData.data()));
        defstream.avail_out = static_cast<uInt>(_rCompressedData.size());
        defstream.next_out = reinterpret_cast<Bytef*>(_rCompressedData.data());

        CheckResult(deflateInit2(&defstream, CompressionLevels[_Level], Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY));
        CheckResult(deflate(&defstream, Z_FINISH));
        CheckResult(deflateEnd(&defstream));

        _rCompressedData.resize(defstream.total_out);
    }

    // -----------------------------------------------------------------------------

    void Compress(const char* _pDecompressedData, int _Size, std::vector<char>& _rCompressedData, int _Level = 1)
    {
        _rCompressedData.resize(_Size);

        z_stream defstream;
        defstream.zalloc = Z_NULL;
        defstream.zfree = Z_NULL;
        defstream.opaque = Z_NULL;
        defstream.avail_in = static_cast<uInt>(_Size);
        defstream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(_pDecompressedData));
        defstream.avail_out = static_cast<uInt>(_rCompressedData.size());
        defstream.next_out = reinterpret_cast<Bytef*>(_rCompressedData.data());

        CheckResult(deflateInit2(&defstream, CompressionLevels[_Level], Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY));
        CheckResult(deflate(&defstream, Z_FINISH));
        CheckResult(deflateEnd(&defstream));

        _rCompressedData.resize(defstream.total_out);
    }

} // namespace Base