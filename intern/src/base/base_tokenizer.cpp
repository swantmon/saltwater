
#include "base/base_tokenizer.h"

#include <fstream>
#include <iostream>

namespace CON
{
    CTokenizer::CTokenizer()
        : m_Stream()
    {
        m_Stream.clear();

        m_Stream.precision(s_DecimalPrecision);
    }

    // -----------------------------------------------------------------------------

    CTokenizer::CTokenizer(const Base::Char* _pString)
    {
        m_Stream << _pString;
    }

    // -----------------------------------------------------------------------------
    
    CTokenizer::~CTokenizer()
    {
    }

    // -----------------------------------------------------------------------------

    void CTokenizer::Escape()
    {
        m_Stream << s_EscapeSeperator;
    }

    // -----------------------------------------------------------------------------

    void CTokenizer::Clear()
    {
        m_Stream.clear();
    }

    // -----------------------------------------------------------------------------

    void CTokenizer::SaveToFile(const Base::Char* _pFilename)
    {
        std::ofstream OutputFile;

        OutputFile.open(_pFilename);

        OutputFile << m_Stream.rdbuf();

        OutputFile.close();
    }
} // namespace CON