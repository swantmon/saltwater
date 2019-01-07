#pragma once

#include "engine/engine_config.h"

#include <sstream>
#include <string>

namespace Core
{
    ENGINE_API class CRecorder
    {
    public:

        CRecorder();
        ~CRecorder();

    public:

        void Step();

    public:

        void Dump(void* _pBytes, size_t _NumberOfBytes);
        void* Read(size_t _NumberOfBytes);

    public:

        void SaveRecordingToFile(const std::string& _rPathToFile);

    private:

        typedef std::stringstream CStream;

    private:

        class CHeader
        {
        public:
            int    NumberOfFrames;
            int    FrameRate;
            time_t CreationTime;
            time_t ModificationTime;
        };

        class CFrameHeader
        {
        public:
            int StepSize;
            void* NextPtr;
        };

    private:

        CHeader m_Header;
        CStream m_Stream;
        
        CFrameHeader m_CurrentFrameHeader;
        CStream m_CurrentFrameData;

        std::string m_PathToArchive;
        bool m_FrameIsFinished;
    };
} // namespace Core

namespace Core
{
    CRecorder::CRecorder()
    {
        
    }

    // -----------------------------------------------------------------------------

    CRecorder::~CRecorder()
    {

    }

    // -----------------------------------------------------------------------------

    void CRecorder::Step()
    {

    }

    // -----------------------------------------------------------------------------

    void CRecorder::Dump(void* _pBytes, size_t _NumberOfBytes)
    {

    }

    // -----------------------------------------------------------------------------

    void* CRecorder::Read(size_t _NumberOfBytes)
    {
        return 0;
    }

    // -----------------------------------------------------------------------------

    void CRecorder::SaveRecordingToFile(const std::string& _rPathToFile)
    {

    }
} // namespace Core