
#pragma once

#include <vector>

namespace Base
{
    void Decompress(const std::vector<char>& _rCompressedData, std::vector<char>& _rDecompressedData);
    void Compress(const std::vector<char>& _rDecompressedData, std::vector<char>& _rCompressedData, int _Level = 1);
    void Compress(const char* _pDecompressedData, int _Size, std::vector<char>& _rCompressedData, int _Level = 1);
} // namespace Base
