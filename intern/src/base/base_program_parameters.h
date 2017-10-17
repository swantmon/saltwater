
#pragma once

#include "base/base_crc.h"

#include <map>

namespace IO
{
    class CProgramParameters
    {
    public:

        static CProgramParameters& GetInstance();

    public:

        template<typename T>
        void AddParameter(const std::string& _rOption, const T _rParameter);
        void AddParameter(const std::string& _rOption, bool _Bool);
        void AddParameter(const std::string& _rOption, const char* _pText);
        void AddParameter(const std::string& _rOption, const std::string& _rText);

    public:

        short GetShort(const std::string& _rOption);
        unsigned short GetUShort(const std::string& _rOption);

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

        CProgramParameters();
        ~CProgramParameters();

    private:

        COptionParameter m_Container;
    };
} // namespace IO

namespace IO
{
    CProgramParameters& CProgramParameters::GetInstance()
    {
        static CProgramParameters s_Sinstance;

        return s_Sinstance;
    }
} // namespace IO

namespace IO
{
    CProgramParameters::CProgramParameters()
    {

    }

    // -----------------------------------------------------------------------------

    CProgramParameters::~CProgramParameters()
    {
        m_Container.clear();
    }

    // -----------------------------------------------------------------------------

    template<typename T>
    void CProgramParameters::AddParameter(const std::string& _rOption, const T _Parameter)
    {
        m_Container.insert(COptionParameterPair(_rOption, std::to_string(_Parameter)));
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::AddParameter(const std::string& _rOption, bool _Bool)
    {
        m_Container.insert(COptionParameterPair(_rOption, _Bool ? "1" : "0"));
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::AddParameter(const std::string& _rOption, const char* _pText)
    {
        m_Container.insert(COptionParameterPair(_rOption, _pText));
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::AddParameter(const std::string& _rOption, const std::string& _rText)
    {
        m_Container.insert(COptionParameterPair(_rOption, _rText));
    }

    // -----------------------------------------------------------------------------

    int CProgramParameters::GetInt(const std::string& _rOption)
    {
        return std::stoi(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    unsigned int CProgramParameters::GetUInt(const std::string& _rOption)
    {
        return std::stoul(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    long CProgramParameters::GetLong(const std::string& _rOption)
    {
        return std::stol(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    unsigned long CProgramParameters::GetULong(const std::string& _rOption)
    {
        return std::stoul(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    long long CProgramParameters::GetLongLong(const std::string& _rOption)
    {
        return std::stoll(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    unsigned long long CProgramParameters::GetULongLong(const std::string& _rOption)
    {
        return std::stoull(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    bool CProgramParameters::GetBoolean(const std::string& _rOption)
    {
        return m_Container[_rOption] == "1" ? true : false;
    }

    // -----------------------------------------------------------------------------

    float CProgramParameters::GetFloat(const std::string& _rOption)
    {
        return std::stof(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    double CProgramParameters::GetDouble(const std::string& _rOption)
    {
        return std::stod(m_Container[_rOption]);
    }

    // -----------------------------------------------------------------------------

    const char* CProgramParameters::GetString(const std::string& _rOption)
    {
        return m_Container[_rOption].c_str();
    }

    // -----------------------------------------------------------------------------

    const std::string& CProgramParameters::GetStdString(const std::string& _rOption)
    {
        return m_Container[_rOption];
    }

    // -----------------------------------------------------------------------------

    bool CProgramParameters::HasParameter(const std::string& _rOption)
    {
        return m_Container.find(_rOption) != m_Container.end();
    }
} // namespace IO