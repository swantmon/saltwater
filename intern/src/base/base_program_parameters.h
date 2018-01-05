
#pragma once

#include "base/base_console.h"

#include <map>
#include <string>

namespace IO
{
    class CProgramParameters
    {
    public:

        static CProgramParameters& GetInstance();

    public:

        void ParseArguments(const std::string& _rArguments);
        void ParseFile(const std::string& _rFile);

    public:

        void WriteFile(const std::string& _rFile);

    public:

        template<typename T>
        void AddParameter(const std::string& _rOption, const T _rParameter);
        void AddParameter(const std::string& _rOption, bool _Bool);
        void AddParameter(const std::string& _rOption, const char* _pText);
        void AddParameter(const std::string& _rOption, const std::string& _rText);

    public:

        int GetInt(const std::string& _rOption);
        unsigned int GetUInt(const std::string& _rOption);

        long GetLong(const std::string& _rOption);
        unsigned long GetULong(const std::string& _rOption);

        long long GetLongLong(const std::string& _rOption);
        unsigned long long GetULongLong(const std::string& _rOption);

        bool GetBoolean(const std::string& _rOption);

        float GetFloat(const std::string& _rOption);

        double GetDouble(const std::string& _rOption);

        const char* GetString(const std::string& _rOption);

        const std::string& GetStdString(const std::string& _rOption);

    public:

        bool HasParameter(const std::string& _rOption);

    private:

        typedef std::map<std::string, std::string>  COptionParameter;
        typedef COptionParameter::iterator          COptionParameterIterator;
        typedef std::pair<std::string, std::string> COptionParameterPair;

    private:

        COptionParameter m_Container;

    private:

        CProgramParameters();
        ~CProgramParameters();

    private:

        void InternParseString(const std::string& _rString, const char _Delimiter);
    
    };
} // namespace IO

namespace IO
{
    template<typename T>
    void CProgramParameters::AddParameter(const std::string& _rOption, const T _Parameter)
    {
#ifdef __ANDROID__
        std::ostringstream Stream;

        Stream << _Parameter;

        m_Container.insert(COptionParameterPair(_rOption, Stream.str()));
#else
        m_Container.insert(COptionParameterPair(_rOption, std::to_string(_Parameter)));
#endif // __ANDROID__
    }
} // namespace IO