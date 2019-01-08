#pragma once

#include "base/base_serialize_text_reader.h"
#include "base/base_serialize_text_writer.h"

#include <fstream>

namespace Core
{
    class CRecorder
    {
    public:
           
        typedef CRecorder CThis;
        
    public:

        CRecorder(const std::string& _rPathToRecord, bool _Record, int _NumberOfCategories = 1);
        ~CRecorder();

    public:

        inline void Step();

        inline void Stop();
        
    public:

        inline void Write(const void* _pBytes, const unsigned int _NumberOfBytes, int _Category = 0);

        inline void Read(void* _pBytes, const unsigned int _NumberOfBytes, int _Category = 0);

    private:

        typedef std::stringstream            CStream;

        typedef Base::CTextWriter            CWriter;
        typedef Base::CTextReader            CReader;
        typedef std::vector<Base::CArchive*> CCategories;
        typedef std::vector<CCategories>     CFrames;

    private:

        CFrames m_Frames;
        std::string m_PathToRecord;
        bool m_IsRecording;

        std::vector<std::ifstream*> m_InputStreams;
        std::vector<std::ofstream*> m_OutputStreams;

    private:

        int m_CurrentNumberOfFrame;
        int m_NumberOfCategories;
    };
} // namespace Core

namespace Core
{
    CRecorder::CRecorder(const std::string& _rPathToRecord, bool _Record, int _NumberOfCategories)
        : m_CurrentNumberOfFrame(0)
        , m_NumberOfCategories  (_NumberOfCategories)
        , m_PathToRecord        (_rPathToRecord)
        , m_IsRecording         (_Record)

    {
        if (m_IsRecording)
        {
            CCategories NewCategories;

            for (int Category = 0; Category < m_NumberOfCategories; ++Category)
            {
                std::ofstream* OutputCategory = new std::ofstream();

                OutputCategory->open(m_PathToRecord + std::to_string(m_CurrentNumberOfFrame) + "_" + std::to_string(Category) + ".txt");

                CWriter* CategoryReader = new CWriter(*OutputCategory, 1);

                NewCategories.push_back(CategoryReader);
                m_OutputStreams.push_back(OutputCategory);
            }

            m_Frames.push_back(NewCategories);
        }
        else
        {
            // -----------------------------------------------------------------------------
            // Header
            // -----------------------------------------------------------------------------
            std::ifstream InputHeader;

            InputHeader.open(m_PathToRecord + ".info");

            CReader Header(InputHeader, 1);

            Header >> m_NumberOfCategories;

            InputHeader.close();

            // -----------------------------------------------------------------------------
            // Frames
            // -----------------------------------------------------------------------------
            CCategories Categories;

            for (int Category = 0; Category < m_NumberOfCategories; ++Category)
            {
                std::ifstream* InputCategory = new std::ifstream();

                InputCategory->open(m_PathToRecord + std::to_string(m_CurrentNumberOfFrame) + "_" + std::to_string(Category) + ".txt");

                CReader* CategoryReader = new CReader(*InputCategory, 1);

                Categories.push_back(CategoryReader);
                m_InputStreams.push_back(InputCategory);
            }

            m_Frames.push_back(Categories);
        }
    }

    // -----------------------------------------------------------------------------

    CRecorder::~CRecorder()
    {
        Stop();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Step()
    {
        ++ m_CurrentNumberOfFrame;

        // -----------------------------------------------------------------------------
        // Close current streams
        // -----------------------------------------------------------------------------
        for (auto& rStream : m_OutputStreams)
        {
            rStream->close();

            delete rStream;
        }

        for (auto& rStream : m_InputStreams)
        {
            rStream->close();

            delete rStream;
        }

        m_OutputStreams.clear();

        m_InputStreams.clear();

        if (m_IsRecording)
        {
            // -----------------------------------------------------------------------------
            // Open new streams
            // -----------------------------------------------------------------------------
            CCategories NewCategories;

            for (int Category = 0; Category < m_NumberOfCategories; ++Category)
            {
                std::ofstream* OutputCategory = new std::ofstream();

                OutputCategory->open(m_PathToRecord + std::to_string(m_CurrentNumberOfFrame) + "_" + std::to_string(Category) + ".txt");

                CWriter* CategoryReader = new CWriter(*OutputCategory, 1);

                NewCategories.push_back(CategoryReader);
                m_OutputStreams.push_back(OutputCategory);
            }

            m_Frames.push_back(NewCategories);
        }
        else
        {
            // -----------------------------------------------------------------------------
            // Open new streams
            // -----------------------------------------------------------------------------
            CCategories NewCategories;

            for (int Category = 0; Category < m_NumberOfCategories; ++Category)
            {
                std::ifstream* OutputCategory = new std::ifstream();

                OutputCategory->open(m_PathToRecord + std::to_string(m_CurrentNumberOfFrame) + "_" + std::to_string(Category) + ".txt");

                CReader* CategoryReader = new CReader(*OutputCategory, 1);

                NewCategories.push_back(CategoryReader);
                m_InputStreams.push_back(OutputCategory);
            }

            m_Frames.push_back(NewCategories);
        }
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Stop()
    {
        // -----------------------------------------------------------------------------
        // Header
        // -----------------------------------------------------------------------------
        std::ofstream OutputHeader;

        OutputHeader.open(m_PathToRecord + ".info");

        CWriter HeaderWriter(OutputHeader, 1);

        HeaderWriter << m_NumberOfCategories;

        OutputHeader.close();

        // -----------------------------------------------------------------------------
        // Close current streams
        // -----------------------------------------------------------------------------
        for (auto& rStream : m_OutputStreams)
        {
            rStream->close();

            delete rStream;
        }

        for (auto& rStream : m_InputStreams)
        {
            rStream->close();

            delete rStream;
        }

        m_OutputStreams.clear();

        m_InputStreams.clear();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Write(const void* _pBytes, const unsigned int _NumberOfBytes, int _Category)
    {
        static_cast<Base::CTextWriter*>(m_Frames[m_CurrentNumberOfFrame][_Category])->WriteBinary(static_cast<const char*>(_pBytes), _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Read(void* _pBytes, const unsigned int _NumberOfBytes, int _Category)
    {
        static_cast<Base::CTextReader*>(m_Frames[m_CurrentNumberOfFrame][_Category])->ReadBinary(static_cast<char*>(_pBytes), _NumberOfBytes);
    }
} // namespace Core