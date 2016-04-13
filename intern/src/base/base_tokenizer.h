#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"

#include <sstream>

namespace CON
{
    class CTokenizer
    {
    public:

        static const char         s_CharSeperator    = '\"';
        static const char         s_EscapeSeperator  = '\n';
        static const char         s_OffsetSeperator  = ';';
        static const unsigned int s_DecimalPrecision = 5;

    public:

        CTokenizer();
        CTokenizer(const Base::Char* _pString);
       ~CTokenizer();

    public:

        template<class T>
        void Append(T _Value);

        void Escape();

        void Clear();

        void SaveToFile(const Base::Char* _pFilename);

    private:

        std::stringstream m_Stream;
    };

    // -----------------------------------------------------------------------------

    template<class T>
    void CTokenizer::Append(T _Value)
    {
        m_Stream << _Value << s_OffsetSeperator;
    }
} // namespace CON